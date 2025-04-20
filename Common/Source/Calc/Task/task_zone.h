/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   task_sector.h
 * Author: Bruno de Lacheisserie
 *
 * Created on November 23, 2023
 */
#ifndef _CALC_TASK_TASK_ZONE_H_
#define _CALC_TASK_TASK_ZONE_H_

#include "externs.h"

namespace task {

GeoPoint from_task(int tp_index);

sector_type_t get_zone_type(int tp_index);


/**
 *  struct to store data required to :
 *    - check if current position is inside Turnpoint.
 *    - drawing zone on map 
 */
struct sector_data {
  GeoPoint center;
  double max_radius;
  double start_radial;
  double end_radial;
};

struct circle_data {
  GeoPoint center;
  double radius;
};

struct dae_data {
  GeoPoint center;
  double bisector;
};

struct line_data {
  GeoPoint center;
  double radius;
  double bisector;
  double inbound;
  double outbound;
};

struct sgp_start_data : public line_data {
  sgp_start_data(line_data&& data) : line_data(std::move(data))  {}
};

/**
 *  helper to get sector radius from Task definition.
 */
template <sector_type_t type, task_type_t task_type>
struct zone_radius;

template <sector_type_t type>
struct zone_radius<type, task_type_t::DEFAULT> {
  static double get(int tp_index) {
    if (tp_index == 0) {
        return StartRadius;
    } else if (!ValidTaskPointFast(tp_index + 1)) {
        return FinishRadius;
    } else {
        return SectorRadius;
    }
  }
};

template<>
struct zone_radius<sector_type_t::SECTOR, task_type_t::AAT> {
  static double get(int tp_index) {
    if (tp_index == 0) {
        return StartRadius;
    } else if (!ValidTaskPointFast(tp_index + 1)) {
        return FinishRadius;
    } else {
        return Task[tp_index].AATSectorRadius;
    }
  }
};

template<>
struct zone_radius<sector_type_t::CIRCLE, task_type_t::AAT> {
  static double get(int tp_index) {
    if (tp_index == 0) {
        return StartRadius;
    } else if (!ValidTaskPointFast(tp_index + 1)) {
        return FinishRadius;
    } else {
        return Task[tp_index].AATCircleRadius;
    }
  }
};

template<>
struct zone_radius<sector_type_t::LINE, task_type_t::AAT> 
    : public zone_radius<sector_type_t::CIRCLE, task_type_t::AAT> {};

template<>
struct zone_radius<sector_type_t::LINE, task_type_t::GP> 
    : public zone_radius<sector_type_t::LINE, task_type_t::AAT> {};

template<>
struct zone_radius<sector_type_t::CIRCLE, task_type_t::GP> 
    : public zone_radius<sector_type_t::CIRCLE, task_type_t::AAT> {};

/**
 *  helper to get zone data struct from Task definition.
 */
template <sector_type_t type, task_type_t task_type>
struct zone_data;

template <task_type_t task_type>
struct zone_data<sector_type_t::DAe, task_type> {
  static dae_data get(int tp_index) {
    return {
      from_task(tp_index),
      Task[tp_index].Bisector
    };
  }
};

template <task_type_t task_type>
struct zone_data<sector_type_t::LINE, task_type> {
  static line_data get(int tp_index) {
    return {
      from_task(tp_index),
      zone_radius<sector_type_t::LINE, task_type>::get(tp_index),
      Task[tp_index].Bisector,
      Task[tp_index].InBound,
      Task[tp_index].OutBound
    };
  }
};

template <task_type_t task_type>
struct zone_data<sector_type_t::CIRCLE, task_type> {
  static circle_data get(int tp_index) {
    return {
      from_task(tp_index),
      zone_radius<sector_type_t::CIRCLE, task_type>::get(tp_index)
    };
  }
};

template <task_type_t task_type>
struct zone_data<sector_type_t::ESS_CIRCLE, task_type> 
    : public zone_data<sector_type_t::CIRCLE, task_type>  { };

template <>
struct zone_data<sector_type_t::SECTOR, task_type_t::DEFAULT> {
  static sector_data get(int tp_index) {
    return {
      from_task(tp_index),
      zone_radius<sector_type_t::SECTOR, task_type_t::DEFAULT>::get(tp_index),
      Task[tp_index].Bisector - 45.,
      Task[tp_index].Bisector + 45.,
    };
  }
};

template <>
struct zone_data<sector_type_t::SECTOR, task_type_t::AAT> {
  static sector_data get(int tp_index) {
    return {
      from_task(tp_index), 
      zone_radius<sector_type_t::SECTOR, task_type_t::AAT>::get(tp_index),
      Task[tp_index].AATStartRadial,
      Task[tp_index].AATFinishRadial,
      // TODO : implement AAT min radius
    };
  }
};

template <>
struct zone_data<sector_type_t::SECTOR, task_type_t::GP> {
  static sector_data get(int tp_index) {
    return zone_data<sector_type_t::SECTOR, task_type_t::AAT>::get(tp_index);
  }
};

template <task_type_t task_type>
struct zone_data<sector_type_t::SGP_START, task_type> {
  static sgp_start_data get(int tp_index) {
    return  zone_data<sector_type_t::LINE, task_type>::get(tp_index);
  }
};

template<typename _Fn, typename _Return = typename _Fn::result_type, typename ..._Args>
struct for_task_type_t {

  static _Return invalid_task_type(task_type_t type) {
    StartupStore(_T("Invalid task type %d"), static_cast<int>(type));
    assert(false);
    return _Return();
  }

  static _Return invalid_tp_type(sector_type_t type) {
    StartupStore(_T("Invalid task point type %d"), static_cast<int>(type));
    assert(false);
    return _Return();
  }

  template <sector_type_t type>
  static _Return dispatch(int tp_index, _Args&& ...args) {
    switch (gTaskType) {
      case task_type_t::DEFAULT:
        return _Fn::template invoke<type, task_type_t::DEFAULT>(tp_index, std::forward<_Args>(args)...);
      case task_type_t::AAT:
        return _Fn::template invoke<type, task_type_t::AAT>(tp_index, std::forward<_Args>(args)...);
      case task_type_t::GP:
        return _Fn::template invoke<type, task_type_t::GP>(tp_index, std::forward<_Args>(args)...);
    }
    return invalid_task_type(gTaskType);  // unknown task type
  }
};

/**
 * call `_Fn::invoke<sector_type_t, task_type_t>(tp_index, ...)` if tp_index is valid task point,
 */
template<typename _Fn, typename ..._Args, typename _Return = typename _Fn::result_type>
_Return invoke_for_task_point(int tp_index, _Args&& ...args) {

  ScopeLock lock(CritSec_TaskData);
  if (!ValidTaskPointFast(tp_index)) {
    return _Return(); // invalid task point
  }

  using dispatch_t = for_task_type_t<_Fn, _Return, _Args...>;


  switch (get_zone_type(tp_index)) {
    case sector_type_t::CIRCLE:
      return dispatch_t::template dispatch<sector_type_t::CIRCLE>(tp_index, std::forward<_Args>(args)...);
    case sector_type_t::SECTOR:
      return dispatch_t::template dispatch<sector_type_t::SECTOR>(tp_index, std::forward<_Args>(args)...);
    case sector_type_t::DAe:
      return dispatch_t::template dispatch<sector_type_t::DAe>(tp_index, std::forward<_Args>(args)...);
    case sector_type_t::LINE:
      return dispatch_t::template dispatch<sector_type_t::LINE>(tp_index, std::forward<_Args>(args)...);
    case sector_type_t::ESS_CIRCLE:
      return dispatch_t::template dispatch<sector_type_t::ESS_CIRCLE>(tp_index, std::forward<_Args>(args)...);
    case sector_type_t::SGP_START:
      return dispatch_t::template dispatch<sector_type_t::SGP_START>(tp_index, std::forward<_Args>(args)...);
  }

  /* invalid zone type, if this happens, there is an unchecked 
   cast to `sector_type_t` somewhere => bug ... */
  return dispatch_t::invalid_tp_type(get_zone_type(tp_index));
}

} // task

#endif // _CALC_TASK_TASK_ZONE_H_
