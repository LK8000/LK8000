/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: $
*/

#ifndef __TRACE_H__
#define __TRACE_H__

#include "Tools.h"
#include "Utils.h"
#include <list>
#include <set>
#include <cmath>
#include <iostream>


class CTrace {
public:
  class CPoint {
    friend class CTrace;
    
    // data from GPS
    double _time;
    double _lat;
    double _lon;
    double _alt;
    
  public:
    // trace optimisation values
    double _prevDistance;
    double _inheritedCost;
    double _distanceCost;
    double _timeCost;
    
    // list iterators
    CPoint *_prev;
    CPoint *_next;
    
    CPoint(const CPoint &);              /**< @brief Disallowed */
    CPoint &operator=(const CPoint &);   /**< @brief Disallowed */
    
  public:
    CPoint(double time, double lat, double lon, double alt, CPoint *prev):
      _time(time), _lat(lat), _lon(lon), _alt(alt),
      _prevDistance(prev ? prev->Distance(*this) : 0),
      _inheritedCost(0), _distanceCost(0), _timeCost(0),
      _prev(prev), _next(0)
    {
      if(_prev) {
        _prev->_next = this;
        if(_prev->_prev)
          _prev->AssesCost();
      }
    }
    
    ~CPoint()
    {
      if(_prev)
        _prev->_next = _next;
      if(_next)
        _next->_prev = _prev;
    }
    
    void Reduce()
    {
      if(!_prev)
        throw std::runtime_error("Reduce(), _prev");
      if(!_next)
        throw std::runtime_error("Reduce(), _next");
      
      // set new prevDistance for next point
      _next->_prevDistance = _prevDistance + _next->_prevDistance - _distanceCost;
      
      // asses new costs
      double cost = (_distanceCost + _inheritedCost) / 2.0;
      _prev->_inheritedCost += cost;
      _next->_inheritedCost += cost;
    }
    
    void AssesCost()
    {
      if(!_prev)
        throw std::runtime_error("AssesCost(), _prev");
      if(!_next)
        throw std::runtime_error("AssesCost(), _next");
      
      _distanceCost = _prevDistance + _next->_prevDistance - _next->Distance(*_prev);
      _timeCost = TimeDelta(*_prev) + _next->TimeDelta(*this);
    }
    
    bool operator==(const CPoint &ref) const { return _time == ref._time; }
    
    bool operator<(const CPoint &ref) const
    {
      // double leftCost = _distanceCost + _inheritedCost + _prev->_inheritedCost + _next->_inheritedCost;
      // double rightCost = ref._distanceCost + ref._inheritedCost + ref._prev->_inheritedCost + ref._next->_inheritedCost;
      // double leftCost = _distanceCost + _inheritedCost;
      // double rightCost = ref._distanceCost + ref._inheritedCost;
      double leftCost = _distanceCost;
      double rightCost = ref._distanceCost;
      if(leftCost > rightCost)
        return false;
      else if(leftCost < rightCost)
        return true;
      else if(_timeCost > ref._timeCost)
        return false;
      else if(_timeCost < ref._timeCost)
        return true;
      else
        return _time < ref._time;
    }
    
    double Time() const      { return _time; }
    double Latitude() const  { return _lat; }
    double Longitude() const { return _lon; }
    double Altitude() const  { return _alt; }
    
    double Distance(const CPoint &ref) const
    { 
      double dist;
      DistanceBearing(ref._lat, ref._lon, _lat, _lon, &dist, 0);
      return dist;
    }
    // double Distance(const CPoint &ref) const
    // { 
    //   double dx = fabs(ref._lon - _lon);
    //   double dy = fabs(ref._lat - _lat);
    //   return sqrt(dx*dx + dy*dy);
    // }

    double TimeDelta(const CPoint &ref) const { return _time - ref._time; }
    
    CPoint *Next() const { return _next; }
    
    friend std::ostream &operator<<(std::ostream &stream, const CPoint &point);
  };
  
private:
  /**
   * @brief A map of GPS points sorted from the most importand to the least important ones.
   * 
   */
  typedef std::set<CPoint *, CPtrCmp<CPoint *> > CPointCostSet;
  
  const unsigned _maxSize;
  unsigned _size;
  unsigned _pointCount;
  CPointCostSet _pointCostSet;
  CPoint *_front;
  CPoint *_back;
  double _length;
  
  CTrace(const CTrace &);              /**< @brief Disallowed */
  CTrace &operator=(const CTrace &);   /**< @brief Disallowed */
  
public:
  CTrace(unsigned maxSize);
  ~CTrace();
  
  unsigned Size() const { return _size; }
  unsigned PointCount() const { return _pointCount; }
  
  const CPoint *Front() const { return _front; }
  
  void Compress();
  void Push(double time, double lat, double lon, double alt);
  
  void DistanceVerify() const;
  
  friend std::ostream &operator<<(std::ostream &stream, const CTrace &trace);
};


#endif /* __TRACE_H__ */
