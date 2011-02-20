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
#include <limits>


class CTrace {
public:
  enum TAlgorithm {
    ALGORITHM_DISTANCE                = 0x0001,
    ALGORITHM_TRIANGLES               = 0x0002,

    ALGORITHM_INHERITED               = 0x0010,

    ALGORITHM_TIME_DELTA              = 0x0100
  };

  class CPoint {
    struct CDijkstraCmp {
      bool operator()(const CPoint *left, const CPoint *right)
      {
        if(left->_pathLength > right->_pathLength)
          return true;
        if(left->_pathLength < right->_pathLength)
          return false;
        return left->_time < right->_dest._time;
      }
    };
    

    class CLink {
      friend class CTrace;
      CPoint &_dest;
      float _distance;
    public:
      CLink(CPoint &dest, double distance):
        _dest(dest), _distance(distance) {}
      
      bool operator==(const CLink &ref) const { return &_dest == &ref._dest; }
      
      bool operator<(const CLink &ref) const
      {
        if(_distance > ref._distance)
          return true;
        else if(_distance < ref._distance)
          return false;
        else
          return _dest._time < ref._dest._time;
      }
    };
    typedef std::set<CLink> CLinkSet;
    
    friend class CTrace;
    
    static const unsigned AVG_TASK_TIME  =  3 * 3600; // 3h
    static const unsigned MAX_TIME_DELTA = 20 * 3600; // 20h
    static const unsigned DAY_SECONDS    = 24 * 3600; // 24h
    //    static const float MAX_LINKS_RATIO   = 0.2;
    static const double DOUBLE_INFINITY  = numeric_limits<double>::max();
    
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
    
    double _pathLength;
    CPoint *_pathPrevious;
    //    CLinkSet _linkSet;
    
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
      double distanceCost;
      if(CTrace::_algorithm & ALGORITHM_TRIANGLES)
        distanceCost = _prevDistance + _next->_prevDistance - _next->Distance(*_prev);
      else
        distanceCost = _distanceCost; 
      
      double cost = (distanceCost + _inheritedCost) / 2.0;
      _prev->_inheritedCost += cost;
      _next->_inheritedCost += cost;
    }
    
    void AssesCost()
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
    
    bool operator==(const CPoint &ref) const { return _time == ref._time; }
    
    bool operator<(const CPoint &ref) const
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
    
    double TimeDelta(const CPoint &ref) const
    {
      double delta = _time - ref._time;
      if(delta < 0) {
        if(delta < -(DAY_SECONDS - MAX_TIME_DELTA))
          delta += DAY_SECONDS;
        else
          std::cerr << "Delta time calculation error: " << delta << std::endl;
      }
      return delta;
    }
    
    CPoint *Next() const { return _next; }
    
    friend std::ostream &operator<<(std::ostream &stream, const CPoint &point);
  };
  
private:
  /**
   * @brief A map of GPS points sorted from the most importand to the least important ones.
   * 
   */
  typedef std::set<CPoint *, CPtrCmp<CPoint *> > CPointCostSet;
  
  static unsigned _maxSize;
  static unsigned _algorithm;
  unsigned _size;
  unsigned _pointCount;
  CPointCostSet _pointCostSet;
  CPoint *_front;
  CPoint *_back;
  double _length;
  
  CTrace(const CTrace &);              /**< @brief Disallowed */
  CTrace &operator=(const CTrace &);   /**< @brief Disallowed */
  
public:
  CTrace(unsigned maxSize, unsigned algorithm);
  ~CTrace();
  
  unsigned Size() const { return _size; }
  unsigned PointCount() const { return _pointCount; }
  
  const CPoint *Front() const { return _front; }
  
  void Compress();
  void Push(double time, double lat, double lon, double alt);
  
  void DistanceVerify() const;

  void Solve();
  
  friend std::ostream &operator<<(std::ostream &stream, const CTrace &trace);
};


#endif /* __TRACE_H__ */
