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


CTrace::CTrace(unsigned maxSize, unsigned algorithm):
  _size(0), _pointCount(0), _front(0), _back(0), _length(0)
{
  _maxSize = maxSize;
  _algorithm = algorithm;
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
  if(_size > _maxSize)
    Compress();
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


// void CTrace::Solve()
// {
//   typedef std::vector<CPoint *> CSolution;
//   CSolution solution;
//   unsigned wpNum;
  
//   CPoint *point = _front;
//   while(point) {
//     CPoint *nextPoint = point->_next;
//     while(nextPoint) {
//       point->_linkSet.insert(CPoint::CLink(*nextPoint, point->Distance(*nextPoint)));
//       nextPoint = nextPoint->_next;
//     }
//     point = point->_next;
//   }

//   point = _front;
//   while(point) {
//     point->_dijkstraWeight = 0;
//     point = point->_next;
//   }
  
//   if(solution.size() == wpNum) {
    
//   }
// }



// unsigned CTrace::SolveIterate(const CPoint *pointArray[], unsigned idx, unsigned stage, CSolutionArray &solution)
// {
//   CSolutionArray bestSolution;
//   unsigned bestLength = 0;
//   for(unsigned i=idx+1; i<_maxSize; i++) {
//     solution[stage] = idx;
//     solution[stage + 1] = i;
//     unsigned length = pointArray[idx]->Distance(*pointArray[i]);
//     if(stage == 5) {
//       std::cout << " - ";
//       for(unsigned k=0; k<solution.size(); k++) {
//         std::cout << solution[k] << " ";
//       }
//       std::cout << std::endl;
//     }
//     else {
//       length += SolveIterate(pointArray, i, stage + 1, solution);
//     }
    
//     if(length > bestLength) {
//       bestSolution = solution;
//       bestLength = length;
//     }
//   }
//   solution = bestSolution;
//   //  std::cout << "Exit: " << stage << std::endl;
//   return bestLength;
// }


// void CTrace::Solve()
// {
//   const CPoint **pointArray = new const CPoint *[_maxSize];
//   CPoint *point = _front;
//   unsigned i=0;
//   while(point) {
//     pointArray[i++] = point;
//     point = point->_next;
//   }
  
//   CSolutionArray solution(7, 0);
//   unsigned length = SolveIterate(pointArray, 0, 0, solution);
  
//   std::cout << "Solution [" << length << "]: " << std::endl;
//   for(unsigned i=0; i<solution.size(); i++) {
//     std::cout << " - " << i << ": " << solution[i] << std::endl;
//   }
  
//   delete pointArray;
// }

  // if(point->_prev) {
  //     if(point->_prevDistance - point->Distance(*point->_prev) > 0.001)
  //       std::cout << "point->_prevDistance: " << point->_prevDistance << " point->Distance(point->_prev): " << point->Distance(*point->_prev) << std::endl;
  //   }
  //   point = point->_next;
  // }


double CTrace::Solve(CSolution &solution)
{
  CDijkstra dijkstra(*this, *_front);
  return dijkstra.Solve(solution);
}



CTrace::CPoint::CPoint(double time, double lat, double lon, double alt, CPoint *prev):
  _time(time), _lat(lat), _lon(lon), _alt(alt),
  _prevDistance(prev ? prev->Distance(*this) : 0),
  _inheritedCost(0), _distanceCost(0), _timeCost(0),
  _pathLength(-1), _pathPrevious(0),
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
    distanceCost = _prevDistance + _next->_prevDistance - _next->Distance(*_prev);
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
    double ax = _lon; double ay = _lat;
    double bx = _prev->_lon;    double by = _prev->_lat;
    double cx = _next->_lon;    double cy = _next->_lat;
    _distanceCost = fabs(ax*(by-cy) + bx*(cy-ay) + cx*(ay-by));
  }
  else {
    _distanceCost = _prevDistance + _next->_prevDistance - _next->Distance(*_prev);
  }
  _timeCost = TimeDelta(*_prev);// + _next->TimeDelta(*this);
}


bool CTrace::CPoint::operator<(const CPoint &ref) const
{
  double leftCost = 0;
  double rightCost = 0;
      
  leftCost += _distanceCost;
  rightCost += ref._distanceCost;
      
  if(CTrace::_algorithm & ALGORITHM_INHERITED) {
    leftCost += _inheritedCost;
    rightCost += ref._inheritedCost;
  }
  if(CTrace::_algorithm & ALGORITHM_TIME_DELTA) {
    leftCost *= _timeCost;
    rightCost *= ref._timeCost;
  }
      
  if(leftCost > rightCost)
    return false;
  else if(leftCost < rightCost)
    return true;
  else if(_timeCost > ref._timeCost)
    return false;
  else if(_timeCost < ref._timeCost)
    return true;
  else
    return _time > ref._time;
}




CTrace::CDijkstra::CDijkstra(CTrace &trace, CPoint &startPoint):
  _trace(trace)
{
  CPoint *point = _trace._front;
  while(point) {
    point->_pathLength = (point == &startPoint) ? 0 : -1;
    point->_pathPrevious = 0;
    _nodeSet.insert(point);
    point = point->_next;
  }
}


double CTrace::CDijkstra::Solve(CSolution &solution)
{
  while(!_nodeSet.empty()) {
    std::cout << "Set size: " << _nodeSet.size() << std::endl;
    for(CNodeSet::iterator it=_nodeSet.begin(); it!=_nodeSet.end(); ++it) {
      std::cout << TimeToString((*it)->_time) << " - " << (*it)->_pathLength << std::endl;
    }
    
    // get point with largest distance
    CNodeSet::iterator it = _nodeSet.begin();
    CPoint *prevPoint = *it;
    double prevPathLength = prevPoint->_pathLength;
    if(prevPathLength == -1) {
      // all remaining points are inaccessible from start
      throw std::runtime_error("Solve -1!!!");
      break;
    }
    
    // check if finish
    
    // remove the point from further analysis
    std::cout << "Removing: " << TimeToString((*it)->_time) << std::endl;
    _nodeSet.erase(it);

    // update all remaining points
    CPoint *point = prevPoint->_next;
    while(point) {
      double pathLength = prevPathLength + prevPoint->Distance(*point);
      if(pathLength > point->_pathLength) {
        it = _nodeSet.find(point);
        if(it != _nodeSet.end())
          _nodeSet.erase(it);
        point->_pathLength = pathLength;
        point->_pathPrevious = prevPoint;
        if(it != _nodeSet.end())
          _nodeSet.insert(point);
      }
      point = point->Next();
    }

    std::cout << "Points:" << std::endl;
    point = _trace._front;
    while(point) {
      std::cout << TimeToString(point->_time) << " - " << point->_pathLength << std::endl;
      point = point->Next();
    }
  }
  
  const CPoint *point = _trace.Front();
  const CPoint *bestFinish = point;
  while(point) {
    if(point->_pathLength > bestFinish->_pathLength)
      bestFinish = point;
    point = point->Next();
  }
  
  point = bestFinish;
  while(point) {
    solution.push_front(point);
    point = point->_pathPrevious;
  }
  
  return bestFinish->_pathLength;
}
