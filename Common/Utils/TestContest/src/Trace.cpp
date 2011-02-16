/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#include "Trace.h"
#include "Tools.h"
#include <iostream>


CTrace::CTrace(unsigned maxSize):
  _maxSize(maxSize), _pointCount(0)
{
}


CTrace::~CTrace()
{
  Purge(_pointList);
}


void CTrace::Push(const CPointGPS *point)
{
  _pointList.push_front(point);
  _pointCount++;
  
  // first and last point are never a subject of optimization
  if(_pointList.size() < 3)
    return;
  
  // add previous point to optimization pool
  CGPSPointList::iterator it=_pointList.begin(), it3=it, it2=++it, it1=++it;
  _pointCostMap[CPointCost(**it2, **it1, **it3)] = it2;
  
  // optimise trace
  if(_pointList.size() > _maxSize) {
    
    // get the worst point
    CGPSPointCostMap::iterator worst = _pointCostMap.begin();
    CGPSPointList::iterator it1 = worst->second, it2 = it1, worstIt = it1;
    
    // remove the worst point
    _pointCostMap.erase(worst);
    
    // find time neighbors
    CGPSPointList::iterator preWorstIt = ++it1;
    CGPSPointList::iterator postWorstIt = --it2;
    
    // find previous time neighbor
    CGPSPointList::iterator prepreWorstIt = ++it1;
    if(prepreWorstIt != _pointList.end()) { 
      CGPSPointCostMap::iterator preWorst = _pointCostMap.find(CPointCost(**preWorstIt, **prepreWorstIt, **worstIt));
      
      // remove previous neighbor from cost map
      _pointCostMap.erase(preWorst);
      
      // insert previous neighbor to a map with new cost
      _pointCostMap[CPointCost(**preWorstIt, **prepreWorstIt, **postWorstIt)] = preWorstIt;
      if(preWorst == _pointCostMap.end()) {
        std::cerr << "ERROR: preWorst not found!!" << std::endl;
        return;
      }
    }
    
    // find next time neighbor
    CGPSPointList::iterator postpostWorstIt = --it2;
    if(postpostWorstIt != _pointList.end()) { 
      CGPSPointCostMap::iterator postWorst = _pointCostMap.find(CPointCost(**postWorstIt, **worstIt, **postpostWorstIt));
      
      // remove next neighbor from cost map
      _pointCostMap.erase(postWorst);
      
      // insert next neighbor to a map with new cost
      _pointCostMap[CPointCost(**postWorstIt, **preWorstIt, **postpostWorstIt)] = postWorstIt;
      if(postWorst == _pointCostMap.end()) {
        std::cerr << "ERROR: postWorst not found!!" << std::endl;
        return;
      }
    }
    
    // delete the worst point
    delete *worstIt;
    _pointList.erase(worstIt);
  }
}


std::ostream &operator<<(std::ostream &stream, const CTrace::CPointCost &cost)
{
  stream << "Area Cost: " << cost._areaCost * 1000 << std::endl;
  stream << "Time Cost: " << cost._timeCost << std::endl;
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
  for(CTrace::CGPSPointList::const_iterator it=trace._pointList.begin(); it!=trace._pointList.end(); ++it)
    stream << **it << std::endl;
  
  stream << "-------------" << std::endl;
  stream << "Cost ordered:" << std::endl;
  stream << "-------------" << std::endl;
  for(CTrace::CGPSPointCostMap::const_iterator it=trace._pointCostMap.begin(); it!=trace._pointCostMap.end(); ++it) {
    stream << it->first;
    stream << **it->second << std::endl;
  }
  
  return stream;
}
