/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#include "Trace.h"
#include <iostream>


CTrace::CTrace(unsigned maxSize):
  _maxSize(maxSize), _size(0), _pointCount(0), _front(0), _back(0), _length(0)
{
}


CTrace::~CTrace()
{
  CPoint *point = _front;
  while(point) {
    CPoint *next = point->_next;
    delete point;
    point = next;
  }
}


void CTrace::Compress()
{
  while(_size > _maxSize) {
    // get the worst point
    CPointCostSet::iterator worstIt = _pointCostSet.begin();
    CPoint *worst = *worstIt;
    
    // remove the worst point from optimization pool
    std::cout << "*** Reducing: " << TimeToString(worst->_time) << std::endl;
    _pointCostSet.erase(worstIt);
    
    // find time neighbors
    CPoint *preWorst = worst->_prev;
    CPoint *postWorst = worst->_next;
    
    // find previous time neighbor
    CPoint *prepreWorst = preWorst->_prev;
    if(prepreWorst) { 
      // remove previous neighbor
      CPointCostSet::iterator preWorstIt = _pointCostSet.find(preWorst);
      if(preWorstIt == _pointCostSet.end()) {
        std::cerr << "ERROR: preWorst not found!!" << std::endl;
        return;
      }
      _pointCostSet.erase(preWorstIt);
    }
    
    // find next time neighbor
    CPoint *postpostWorst = postWorst->_next;
    if(postpostWorst) { 
      // remove next neighbor
      CPointCostSet::iterator postWorstIt = _pointCostSet.find(postWorst);
      if(postWorstIt == _pointCostSet.end()) {
        std::cerr << "ERROR: postWorst not found!!" << std::endl;
        return;
      }
      _pointCostSet.erase(postWorstIt);
    }
    
    // reduce and delete current point
    worst->Reduce();
    delete worst;
    _size--;
    
    if(prepreWorst) { 
      // insert next neighbor
      preWorst->AssesCost();
      _pointCostSet.insert(preWorst);
    }
    if(postpostWorst) { 
      // insert previous neighbor
      postWorst->AssesCost();
      _pointCostSet.insert(postWorst);
    }
  }
}


void CTrace::Push(double time, double lat, double lon, double alt)
{
  _pointCount++;
  
  // add new point to a list
  _back = new CPoint(time, lat, lon, alt, _back);
  _length += _back->_prevDistance;
  _size++;
  if(!_front)
    _front = _back;
  
  // first and last point are never a subject of optimization
  if(_size < 3)
    return;
  
  // add previous point to optimization pool
  _pointCostSet.insert(_back->_prev);
  
  // compress the trace
  // if(_size > _maxSize)
  //   Compress();

  CPoint *point = _front;
  while(point) {
    if(point->_prev) {
      if(point->_prevDistance - point->Distance(*point->_prev) > 0.001)
        std::cout << "point->_prevDistance: " << point->_prevDistance << " point->Distance(point->_prev): " << point->Distance(*point->_prev) << std::endl;
    }
    point = point->_next;
  }
}



std::ostream &operator<<(std::ostream &stream, const CTrace::CPoint &point)
{
  stream << "Time:      " << TimeToString(point._time) << std::endl;
  stream << "Latitude:  " << CoordToString(point._lat, true) << std::endl;
  stream << "Longitude: " << CoordToString(point._lon, false) << std::endl;
  stream << "Altitude:  " << static_cast<unsigned>(point._alt) << "m" << std::endl;
  stream << "Prev distance:  " << point._prevDistance << std::endl;
  stream << "Inherited cost: " << point._inheritedCost << std::endl;
  stream << "Distance Cost:  " << point._distanceCost << std::endl;
  stream << "Time Cost:      " << point._timeCost << std::endl;
  return stream;
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
  for(CTrace::CPointCostSet::const_iterator it=trace._pointCostSet.begin(); it!=trace._pointCostSet.end(); ++it) {
    stream << **it << std::endl;
  }
  
  return stream;
}


void CTrace::DistanceVerify() const
{
  CPoint *point = _front;
  double length = 0;
  while(point) {
    length += point->_prevDistance + point->_inheritedCost;
    point = point->_next;
  }
  if(length != _length) {
    std::cout << "length: " << length << " _length: " << _length << std::endl;
  }
  else
    std::cout << "SUCCESS!!!" << std::endl;
}
