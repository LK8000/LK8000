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
};


/**
 *  helper to get sector radius from Task definition.
 */
template <sector_type_t type, int task_type>
struct zone_radius;

template <sector_type_t type>
struct zone_radius<type, TSK_DEFAULT> {
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
struct zone_radius<sector_type_t::SECTOR, TSK_AAT> {
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
struct zone_radius<sector_type_t::CIRCLE, TSK_AAT> {
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
struct zone_radius<sector_type_t::LINE, TSK_AAT> 
    : public zone_radius<sector_type_t::CIRCLE, TSK_AAT> {};


/**
 *  helper to get zone data struct from Task definition.
 */
template <sector_type_t type, int task_type>
struct zone_data;

template <int task_type>
struct zone_data<sector_type_t::DAe, task_type> {
  static dae_data get(int tp_index) {
    return {
      from_task(tp_index),
      Task[tp_index].Bisector
    };
  }
};

template <int task_type>
struct zone_data<sector_type_t::LINE, task_type> {
  static line_data get(int tp_index) {
    return {
      from_task(tp_index),
      zone_radius<sector_type_t::LINE, task_type>::get(tp_index),
      Task[tp_index].Bisector,
      Task[tp_index].InBound
    };
  }
};

template <int task_type>
struct zone_data<sector_type_t::CIRCLE, task_type> {
  static circle_data get(int tp_index) {
    return {
      from_task(tp_index),
      zone_radius<sector_type_t::CIRCLE, task_type>::get(tp_index)
    };
  }
};

template <int task_type>
struct zone_data<sector_type_t::ESS_CIRCLE, task_type> 
    : public zone_data<sector_type_t::CIRCLE, task_type>  { };

template <>
struct zone_data<sector_type_t::SECTOR, TSK_DEFAULT> {
  static sector_data get(int tp_index) {
    return {
      from_task(tp_index),
      zone_radius<sector_type_t::SECTOR, TSK_DEFAULT>::get(tp_index),
      Task[tp_index].Bisector - 45.,
      Task[tp_index].Bisector + 45.,
    };
  }
};

template <>
struct zone_data<sector_type_t::SECTOR, TSK_AAT> {
  static sector_data get(int tp_index) {
    return {
      from_task(tp_index), 
      zone_radius<sector_type_t::SECTOR, TSK_AAT>::get(tp_index),
      Task[tp_index].AATStartRadial,
      Task[tp_index].AATFinishRadial,
      // TODO : implement AAT min radius
    };
  }
};


/**
 * call `_Fn::invoke<sector_type_t>(tp_index, ...)` if tp_index is valid task point,
 * call `_Fn::invalid()` in all other case.
 */
template<typename _Fn, typename ..._Args, typename _Return = typename _Fn::result_type>
_Return invoke_for_task_point(int tp_index, _Args&& ...args) {

  ScopeLock lock(CritSec_TaskData);
  if (!ValidTaskPointFast(tp_index)) {
    return _Fn::invalid(); // invalid task point
  }

  switch (task::get_zone_type(tp_index)) {
    case sector_type_t::CIRCLE:
      return _Fn::template invoke<sector_type_t::CIRCLE>(tp_index, std::forward<_Args>(args)...);
    case sector_type_t::SECTOR:
      return _Fn::template invoke<sector_type_t::SECTOR>(tp_index, std::forward<_Args>(args)...);
    case sector_type_t::DAe:
      return _Fn::template invoke<sector_type_t::DAe>(tp_index, std::forward<_Args>(args)...);
    case sector_type_t::LINE:
      return _Fn::template invoke<sector_type_t::LINE>(tp_index, std::forward<_Args>(args)...);
    case sector_type_t::ESS_CIRCLE:
      return _Fn::template invoke<sector_type_t::ESS_CIRCLE>(tp_index, std::forward<_Args>(args)...);
  }

  /* invalid zone type, if this happens, there is an unchecked 
   cast to `sector_type_t` somewhere => bug ... */
  return _Fn::invalid();
}


} // task

#endif // _CALC_TASK_TASK_ZONE_H_
