/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   task_sector.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on November 23, 2023
 */

#include "task_zone.h"


GeoPoint task::from_task(int tp_index) {
  return {
    WayPointList[Task[tp_index].Index].Latitude,
    WayPointList[Task[tp_index].Index].Longitude
  };
}


sector_type_t task::get_zone_type(int tp_index) {
  if (tp_index == 0) {
    // start ...
    return StartLine;
  }

  if (!ValidTaskPointFast(tp_index + 1)) {
    // finish
    return FinishLine;
  }

  if (UseAATTarget()) { 
    // GP or AAT task 
    return Task[tp_index].AATType;
  }
  return SectorType;
}

