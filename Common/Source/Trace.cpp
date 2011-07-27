/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#include "options.h"
#ifdef NEW_OLC

#include "Trace.h"
#include "XCSoar.h"
#include <tchar.h>

#include "utils/heapcheck.h"


/** 
 * @brief Constructor
 * 
 * @param maxSize Maximum number of GPS fixes to store inside a trace
 * @param timeLimit Maximum time period of a trace
 * @param algorithm The compression algorithm of a trace
 */
CTrace::CTrace(unsigned maxSize, unsigned timeLimit, unsigned algorithm):
  _maxSize(maxSize), _timeLimit(timeLimit), _algorithm(algorithm),
  _valid(true), _size(0), _analyzedPointCount(0), _front(0), _back(0)
{
}


/** 
 * @brief Destructor
 */
CTrace::~CTrace()
{
  Clear();
}


/** 
 * @brief Clears the trace
 */
void CTrace::Clear()
{
  CPoint *point = _front;
  while(point) {
    CPoint *next = point->_next;
    delete point;
    point = next;
  }

  _valid = true;
  _size = 0;
  _analyzedPointCount = 0;
  _compressionCostSet.clear();
  _front = 0;
  _back = 0;
}


/** 
 * @brief Adds a new point to a trace
 * 
 * @param point Point to add
 */
void CTrace::Push(CPoint *point)
{
  _analyzedPointCount++;
  
  if(!_valid) {
    delete point;
    return;
  }
  
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
      
      if(next != _back) {
        // _back and _front are not stored in a _compressionCostSet so skip below actions when next == _back
        CPointCostSet::iterator nextIt = _compressionCostSet.find(next);
        if(nextIt == _compressionCostSet.end()) {
#ifndef TEST_CONTEST
        StartupStore(_T("%s:%u - ERROR: next not found!!\n"), _T(__FILE__), __LINE__);
#endif
          _valid = false;
        }
        _compressionCostSet.erase(nextIt);
      }
      
      _front = next;
      _front->_prevDistance = 0;
      _front->_distanceCost = 0;
      _front->_timeCost = 0;
      _front->_prev = 0;
      
      if(!_valid)
        // interrupt further operations
        return;
    }
  }
  
  // first and last point are never a subject of optimization
  if(_size < 3)
    return;
  
  // add previous point to compression pool
  _compressionCostSet.insert(_back->_prev);
}


/** 
 * @brief Compresses the trace to the required size
 * 
 * Removes the least important points to maintain required trace size.
 * 
 * @param maxSize Optional argument specifying a new maximum size of the trace
 */
void CTrace::Compress(unsigned maxSize /* = 0 */)
{
  if(!_valid)
    return;
  
  if(maxSize)
    _maxSize = maxSize;
  
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
#ifndef TEST_CONTEST
        StartupStore(_T("%s:%u - ERROR: preWorst not found!!\n"), _T(__FILE__), __LINE__);
#endif
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
#ifndef TEST_CONTEST
        StartupStore(_T("%s:%u - ERROR: postWorst not found!!\n"), _T(__FILE__), __LINE__);
#endif
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


#endif /* NEW_OLC */
