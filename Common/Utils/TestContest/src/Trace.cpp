/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#include "Trace.h"
#include <iostream>


CTrace::CTrace(unsigned maxSize, unsigned timeLimit, short startAltitudeLoss, unsigned algorithm):
  _maxSize(maxSize), _timeLimit(timeLimit), _startAltitudeLoss(startAltitudeLoss), _algorithm(algorithm),
  _size(0), _analyzedPointCount(0), _front(0), _back(0),
  _startDetected(false), _startMaxAltitude(0)
{
}


CTrace::~CTrace()
{
  Clear();
}


void CTrace::Clear()
{
  CPoint *point = _front;
  while(point) {
    CPoint *next = point->_next;
    delete point;
    point = next;
  }
  
  _size = 0;
  _analyzedPointCount = 0;
  _compressionCostSet.clear();
  _front = 0;
  _back = 0;
  _startDetected = false;
  _startMaxAltitude = 0;
}


void CTrace::Push(CPoint *point)
{
  _analyzedPointCount++;
  
  // add new point to a list
  _back = point;
  _size++;
  if(!_front)
    _front = _back;
  
  if(_timeLimit) {
    // limit the trace to required time period
    while(_back && _front && (unsigned)_back->_gps->TimeDelta(*_front->_gps) > _timeLimit) {
      CPoint *next = _front->_next;
      delete _front;
      _size--;
      
      CPointCostSet::iterator nextIt = _compressionCostSet.find(next);
      if(nextIt == _compressionCostSet.end()) {
        std::cerr << "ERROR: next not found!!" << std::endl;
        return;
      }
      _compressionCostSet.erase(nextIt);
      
      _front = next;
      _front->_prevDistance = 0;
      _front->_distanceCost = 0;
      _front->_timeCost = 0;
      _front->_prev = 0;
    }
  }
  
  // first and last point are never a subject of optimization
  if(_size < 3)
    return;
  
  // add previous point to compression pool
  _compressionCostSet.insert(_back->_prev);
}


void CTrace::Push(const CPointGPSSmart &gps)
{
  // filter first points until a standalone flight is detected
  if(!_startDetected) {
    if(_startAltitudeLoss > 0) {
      _startMaxAltitude = std::max(_startMaxAltitude, gps->Altitude());
      if(gps->Altitude() + _startAltitudeLoss > _startMaxAltitude)
        return;
    }
    _startDetected = true;
  }
  
  // add new point
  Push(new CPoint(*this, gps, _back));
}


void CTrace::Compress()
{
  while(_size > _maxSize) {
    // get the worst point
    CPointCostSet::iterator worstIt = _compressionCostSet.begin();
    CPoint *worst = *worstIt;
    
    // remove the worst point from optimization pool
    _compressionCostSet.erase(worstIt);
    
    // find time neighbors
    CPoint *preWorst = worst->_prev;
    CPoint *postWorst = worst->_next;
    
    // find previous time neighbor
    CPoint *prepreWorst = preWorst->_prev;
    if(prepreWorst) { 
      // remove previous neighbor
      CPointCostSet::iterator preWorstIt = _compressionCostSet.find(preWorst);
      if(preWorstIt == _compressionCostSet.end()) {
        std::cerr << "ERROR: preWorst not found!!" << std::endl;
        return;
      }
      _compressionCostSet.erase(preWorstIt);
    }
    
    // find next time neighbor
    CPoint *postpostWorst = postWorst->_next;
    if(postpostWorst) { 
      // remove next neighbor
      CPointCostSet::iterator postWorstIt = _compressionCostSet.find(postWorst);
      if(postWorstIt == _compressionCostSet.end()) {
        std::cerr << "ERROR: postWorst not found!!" << std::endl;
        return;
      }
      _compressionCostSet.erase(postWorstIt);
    }
    
    // reduce and delete current point
    worst->Reduce();
    delete worst;
    _size--;
    
    if(prepreWorst) { 
      // insert next neighbor
      preWorst->AssesCost();
      _compressionCostSet.insert(preWorst);
    }
    if(postpostWorst) { 
      // insert previous neighbor
      postWorst->AssesCost();
      _compressionCostSet.insert(postWorst);
    }
  }
}


std::ostream &operator<<(std::ostream &stream, const CTrace &trace)
{
  stream << "****************************************" << std::endl;
  stream << "**************** TRACE *****************" << std::endl;
  stream << "****************************************" << std::endl;
  stream << "-------------" << std::endl;
  stream << "Time ordered:" << std::endl;
  stream << "-------------" << std::endl;
  const CTrace::CPoint *point=trace.Front();
  do {
    stream << *point << std::endl;
    point = point->Next();
  } while(point);
  
  stream << "-------------" << std::endl;
  stream << "Cost ordered:" << std::endl;
  stream << "-------------" << std::endl;
  for(CTrace::CPointCostSet::const_iterator it=trace._compressionCostSet.begin(); it!=trace._compressionCostSet.end(); ++it) {
    stream << **it << std::endl;
  }
  
  return stream;
}




CTrace::CPoint::CPoint(const CTrace &trace, const CPointGPSSmart &gps, CPoint *prev):
  _trace(trace), 
  _gps(gps),
  _prevDistance(prev ? prev->_gps->Distance(*this->_gps) : 0),
  _inheritedCost(0), _distanceCost(0), _timeCost(0),
  _prev(prev), _next(0)
{
  if(_prev) {
    _prev->_next = this;
    if(_prev->_prev)
      _prev->AssesCost();
  }
}


CTrace::CPoint::CPoint(const CTrace &trace, const CPoint &ref, CPoint *prev):
  _trace(trace), 
  _gps(ref._gps),
  _prevDistance(ref._prevDistance),
  _inheritedCost(ref._inheritedCost), _distanceCost(ref._distanceCost), _timeCost(ref._timeCost),
  _prev(prev), _next(0)
{
  if(_prev) {
    _prev->_next = this;
    if(_prev->_prev)
      _prev->AssesCost();
  }
}


CTrace::CPoint::~CPoint()
{
  if(_prev)
    _prev->_next = _next;
  if(_next)
    _next->_prev = _prev;
}


void CTrace::CPoint::Reduce()
{
  if(!_prev)
    throw std::runtime_error("Reduce(), _prev");
  if(!_next)
    throw std::runtime_error("Reduce(), _next");
  
  // asses new costs & set new prevDistance for next point
  float distanceCost;
  if(_trace._algorithm & ALGORITHM_TRIANGLES) {
    unsigned newDistance = _next->_gps->Distance(*_prev->_gps);
    distanceCost = _prevDistance + _next->_prevDistance - newDistance;
    _next->_prevDistance = newDistance;
  }
  else {
    _next->_prevDistance = _prevDistance + _next->_prevDistance - _distanceCost;
    distanceCost = _distanceCost; 
  }
  
  float cost = (distanceCost + _inheritedCost) / 2.0;
  _prev->_inheritedCost += cost;
  _next->_inheritedCost += cost;
}


void CTrace::CPoint::AssesCost()
{
  if(!_prev)
    throw std::runtime_error("AssesCost(), _prev");
  if(!_next)
    throw std::runtime_error("AssesCost(), _next");
  
  if(_trace._algorithm & ALGORITHM_TRIANGLES) {
    double ax = _gps->Longitude();           double ay = _gps->Latitude();
    double bx = _prev->_gps->Longitude();    double by = _prev->_gps->Latitude();
    double cx = _next->_gps->Longitude();    double cy = _next->_gps->Latitude();
    _distanceCost = fabs(ax*(by-cy) + bx*(cy-ay) + cx*(ay-by));
  }
  else {
    _distanceCost = _prevDistance + _next->_prevDistance - _next->_gps->Distance(*_prev->_gps);
  }
  _timeCost = _gps->TimeDelta(*_prev->_gps);
}



std::ostream &operator<<(std::ostream &stream, const CTrace::CPoint &point)
{
  stream << "Time:      " << TimeToString(point._gps->Time()) << std::endl;
  stream << "Latitude:  " << CoordToString(point._gps->Latitude(), true) << std::endl;
  stream << "Longitude: " << CoordToString(point._gps->Longitude(), false) << std::endl;
  stream << "Altitude:  " << point._gps->Altitude() << "m" << std::endl;
  stream << "Prev distance:  " << point._prevDistance << std::endl;
  stream << "Inherited cost: " << point._inheritedCost << std::endl;
  stream << "Distance Cost:  " << point._distanceCost << std::endl;
  stream << "Time Cost:      " << point._timeCost << std::endl;
  return stream;
}
