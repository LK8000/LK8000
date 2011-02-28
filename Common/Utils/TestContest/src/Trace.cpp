/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#include "Trace.h"
#include <iostream>


unsigned CTrace::_maxSize;
unsigned CTrace::_algorithm;


CTrace::CTrace(unsigned maxSize, unsigned algorithm, unsigned startHeightLoss /*= 100*/, bool gpsPointsOwner /*= true*/):
  _gpsPointsOwner(gpsPointsOwner), _startHeightLoss(startHeightLoss),
  _size(0), _analyzedPointCount(0), _front(0), _back(0),
  _startDetected(false)
{
  _maxSize = maxSize;
  _algorithm = algorithm;
}


CTrace::~CTrace()
{
  CPoint *point = _front;
  while(point) {
    CPoint *next = point->_next;
    if(_gpsPointsOwner)
      delete point->_gps;
    delete point;
    point = next;
  }
}


void CTrace::Push(CPoint *point)
{
  _analyzedPointCount++;
  
  // add new point to a list
  _back = point;
  _size++;
  if(!_front)
    _front = _back;
  
  // first and last point are never a subject of optimization
  if(_size < 3)
    return;
  
  // add previous point to compression pool
  _compressionCostSet.insert(_back->_prev);
}


void CTrace::Push(double time, double lat, double lon, double alt)
{
  Push(new CPoint(new CPointGPS(time, lat, lon, alt), _back));
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
    if(_gpsPointsOwner)
      delete worst->_gps;
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


double CTrace::Solve(CSolution &solution)
{
  CTrace trace(7, CTrace::ALGORITHM_DISTANCE, 0, false);
  
  const CPoint *point = _front;
  while(point) {
    trace.Push(new CPoint(*point, trace._back));
    point = point->_next;
  }
  trace.Compress();
  
  point = trace.Front();
  double length = 0;
  while(point) {
    length += point->_prevDistance;
    solution.push_back(point->_gps);
    point = point->_next;
  }
  
  return length;
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




CTrace::CPoint::CPoint(const CPointGPS *pointGPS, CPoint *prev):
  _gps(pointGPS),
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


CTrace::CPoint::CPoint(const CPoint &ref, CPoint *prev):
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
  
  // set new prevDistance for next point
  _next->_prevDistance = _prevDistance + _next->_prevDistance - _distanceCost;
  
  // asses new costs
  double distanceCost;
  if(CTrace::_algorithm & ALGORITHM_TRIANGLES)
    distanceCost = _prevDistance + _next->_prevDistance - _next->_gps->Distance(*_prev->_gps);
  else
    distanceCost = _distanceCost; 
      
  double cost = (distanceCost + _inheritedCost) / 2.0;
  _prev->_inheritedCost += cost;
  _next->_inheritedCost += cost;
}


void CTrace::CPoint::AssesCost()
{
  if(!_prev)
    throw std::runtime_error("AssesCost(), _prev");
  if(!_next)
    throw std::runtime_error("AssesCost(), _next");
  
  if(CTrace::_algorithm & ALGORITHM_TRIANGLES) {
    double ax = _gps->Longitude(); double ay = _gps->Latitude();
    double bx = _prev->_gps->Longitude();    double by = _prev->_gps->Latitude();
    double cx = _next->_gps->Longitude();    double cy = _next->_gps->Latitude();
    _distanceCost = fabs(ax*(by-cy) + bx*(cy-ay) + cx*(ay-by));
  }
  else {
    _distanceCost = _prevDistance + _next->_prevDistance - _next->_gps->Distance(*_prev->_gps);
  }
  _timeCost = _gps->TimeDelta(*_prev->_gps);// + _next->TimeDelta(*this);
}



std::ostream &operator<<(std::ostream &stream, const CTrace::CPoint &point)
{
  stream << "Time:      " << TimeToString(point._gps->Time()) << std::endl;
  stream << "Latitude:  " << CoordToString(point._gps->Latitude(), true) << std::endl;
  stream << "Longitude: " << CoordToString(point._gps->Longitude(), false) << std::endl;
  stream << "Altitude:  " << static_cast<unsigned>(point._gps->Altitude()) << "m" << std::endl;
  stream << "Prev distance:  " << point._prevDistance << std::endl;
  stream << "Inherited cost: " << point._inheritedCost << std::endl;
  stream << "Distance Cost:  " << point._distanceCost << std::endl;
  stream << "Time Cost:      " << point._timeCost << std::endl;
  return stream;
}




// CTrace::CDijkstra::CDijkstra(CTrace &trace, CPoint &startPoint):
//   _trace(trace)
// {
//   CPoint *point = _trace._front;
//   while(point) {
//     point->_pathLength = 0;
//     point->_pathPrevious = 0;
//     point->_pathHops = 0;
//     point = point->_next;
//   }
// }


// double CTrace::CDijkstra::Solve(CSolution &solution)
// {
//   while(!_nodeSet.empty()) {
//     std::cout << "Set size: " << _nodeSet.size() << std::endl;
//     for(CNodeSet::iterator it=_nodeSet.begin(); it!=_nodeSet.end(); ++it) {
//       std::cout << TimeToString((*it)->_time) << " - " << (*it)->_pathLength << std::endl;
//     }
    
//     // get point with largest distance
//     CNodeSet::iterator it = _nodeSet.begin();
//     CPoint *prevPoint = *it;
//     double prevPathLength = prevPoint->_pathLength;
//     if(prevPathLength == DBL_MIN) {
//       // all remaining points are inaccessible from start
//       throw std::runtime_error("Solve -1!!!");
//       break;
//     }
    
//     // check if finish
    
//     // remove the point from further analysis
//     std::cout << "Removing: " << TimeToString((*it)->_time) << std::endl;
//     _nodeSet.erase(it);

//     // update all remaining points
//     CPoint *point = prevPoint->_next;
//     while(point) {
//       double pathLength = prevPathLength + (-prevPoint->Distance(*point));
//       if(pathLength < point->_pathLength) {
//         it = _nodeSet.find(point);
//         if(it != _nodeSet.end())
//           _nodeSet.erase(it);
//         point->_pathLength = pathLength;
//         point->_pathPrevious = prevPoint;
//         if(it != _nodeSet.end())
//           _nodeSet.insert(point);
//       }
//       point = point->Next();
//     }

//     std::cout << "Points:" << std::endl;
//     point = _trace._front;
//     while(point) {
//       std::cout << TimeToString(point->_time) << " - " << point->_pathLength << std::endl;
//       point = point->Next();
//     }
//   }
  
//   const CPoint *point = _trace.Front();
//   const CPoint *bestFinish = point;
//   while(point) {
//     if(point->_pathLength < bestFinish->_pathLength)
//       bestFinish = point;
//     point = point->Next();
//   }
  
//   point = bestFinish;
//   while(point) {
//     solution.push_front(point);
//     point = point->_pathPrevious;
//   }
  
//   return bestFinish->_pathLength;
// }



// double CTrace::CDijkstra::Solve(CTrace &trace, CSolution &solution)
// {
  // const unsigned MAX_HOPS = 7;
  // const CPoint *point = _trace._front;
  // while(point) {
  //   CPoint *neighborPoint = point->_next;
  //   while(neighborPoint) {
  //     if(point->_pathHops < MAX_HOPS) {
  //       if(neighborPoint->_pathLength <= point->_pathLength + point->Distance(*neighborPoint)) {
  //         neighborPoint->_pathLength = point->_pathLength + point->Distance(*neighborPoint);
  //         neighborPoint->_pathHops = point->_pathHops + 1;
  //         neighborPoint->_pathPrevious = point;
  //       }
  //     }
  //     neighborPoint = neighborPoint->_next;
  //   }
  //   point = point->_next;
  // }
// }
