/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   PGTaskMgr.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 11 sept. 2012
 */

#include <type_traits>
#include "PGTaskMgr.h"
#include "externs.h"
#include "AATDistance.h"
#include "Waypointparser.h"
#include "McReady.h"

#include "algorithm"
#include "utils/stl_utils.h"
#include "math.h"
#include "PGCircleTaskPt.h"
#include "PGLineTaskPt.h"
#include "PGSectorTaskPt.h"
#include "PGEssCircleTaskPt.h"
#include "NavFunctions.h"
#include "Geographic/GeoPoint.h"
#include "Geographic/TransverseMercator.h"
#include "Draw/Task/TaskRendererMgr.h"

void PGTaskMgr::Initialize() {
    m_Task.clear();
    size_t tp_count = ValidTaskPointFast(0) ? 1 : 0;

    if(tp_count == 0) {
        // empty task
        return;
    }


    // build Mercator Reference Grid
    // find center of Task
    GeoPoint min = GetTurnpointPosition(0);
    GeoPoint max = min;

    for (; ValidTaskPointFast(tp_count); ++tp_count) {
        GeoPoint pos = GetTurnpointPosition(tp_count);

        min.latitude = std::min(min.latitude, pos.latitude);
        min.longitude = std::min(min.longitude, pos.longitude);

        max.latitude = std::max(max.latitude, pos.latitude);
        max.longitude = std::max(max.longitude, pos.longitude);
    }

    const GeoPoint center = {
        (min.latitude + max.latitude) / 2,
        (min.longitude + max.longitude) / 2
    };

    m_Projection = std::make_unique<TransverseMercator>(center);

    // build task point list
    for (int curwp = 0; ValidTaskPointFast(curwp); ++curwp) {
      task::invoke_for_task_point<AddTaskPt_Helper>(curwp, *this);
    }
}

void PGTaskMgr::AddTaskPt(int tp_index, const task::sector_data& data) {
  m_Task.emplace_back(Make<PGSectorTaskPt>(data.center));
}

void PGTaskMgr::AddTaskPt(int tp_index, const task::circle_data& data) {
  m_Task.emplace_back(Make<PGCircleTaskPt>(data.center, data.radius));
}

void PGTaskMgr::AddTaskPt(int tp_index, const task::dae_data& data) {
  m_Task.emplace_back(Make<PGSectorTaskPt>(data.center));
}

void PGTaskMgr::AddTaskPt(int tp_index, const task::line_data& data) {
  auto pTskPt = Make<PGLineTaskPt>(data.center);

  // Find prev Tp not same as current.
  int PrevIdx = tp_index - 1;
  while (PrevIdx > 0 && Task[PrevIdx].Index == Task[tp_index].Index) {
    --PrevIdx;
  }

  // Find next Tp not same as current.
  int NextIdx = tp_index + 1;
  while (ValidTaskPointFast(NextIdx) && Task[NextIdx].Index == Task[tp_index].Index) {
    ++NextIdx;
  }

  // Calc Cross Dir Vector
  ProjPt InB = {0, 0};
  ProjPt OutB = {0, 0};
  if (ValidTaskPointFast(NextIdx)) {
    OutB = m_Projection->Forward(GetTurnpointPosition(NextIdx));
    OutB = Normalize(OutB - pTskPt->m_Center);
  }
  else if (PrevIdx >= 0) {
    InB = m_Task[PrevIdx]->getCenter();
    InB = Normalize(pTskPt->m_Center - InB);
  }

  if (!IsNull(InB) && !IsNull(OutB)) {
    pTskPt->m_DirVector = Normalize(InB + OutB);
  }
  else if (!IsNull(InB)) {
    pTskPt->m_DirVector = InB;
  }
  else if (!IsNull(OutB)) {
    pTskPt->m_DirVector = OutB;
  }

  // Calc begin and end off line.
  ProjPt::scalar_type d = Length(pTskPt->m_DirVector);
  if (d > 0) {
    ProjPt u = Rotate90(pTskPt->m_DirVector) * data.radius;
    pTskPt->m_LineBegin = pTskPt->m_Center + u;  // begin of line
    pTskPt->m_LineEnd = pTskPt->m_Center - u;    // end of line
  }

  m_Task.emplace_back(std::move(pTskPt));
}

void PGTaskMgr::AddTaskPt(int tp_index, const task::ess_circle& data) {
  m_Task.emplace_back(Make<PGEssCircleTaskPt>(data.center, data.radius));
}

void PGTaskMgr::Optimize(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
    if (((size_t) ActiveTaskPoint) >= m_Task.size()) {
        return;
    }
    assert(m_Projection);

    GeoPoint prev_position;
    if (ValidWayPointFast(HomeWaypoint)) {
        prev_position = GetWaypointPosition(HomeWaypoint);
    } else if (ValidResWayPointFast(RESWP_TAKEOFF)) {
        prev_position = GetWaypointPosition(RESWP_TAKEOFF);
    } else {
        prev_position = GetTurnpointPosition(0);
    }
    ProjPt PrevPos = m_Projection->Forward(prev_position);

    for (size_t i = 0; i < m_Task.size(); ++i) {

        if (i == static_cast<size_t>(ActiveTaskPoint)) {
            PrevPos = m_Projection->Forward(GetCurrentPosition(*Basic));
        }

        // Optimize
        const auto& CurrPos = m_Task[i]->getCenter();
        size_t next = i + 1;
        while (next < m_Task.size() && CurrPos == m_Task[next]->getOptimized()) {
            ++next;
        }
        
        if (next < m_Task.size()) {
            m_Task[i]->Optimize(PrevPos, m_Task[next]->getOptimized());
        } else {
            m_Task[i]->Optimize(PrevPos, {0., 0.});
        }

        // Update previous Position for Next Loop
        PrevPos = m_Task[i]->getOptimized();
    }
}

GeoPoint PGTaskMgr::getOptimized(size_t i) const {
    assert(m_Projection);
    return m_Projection->Reverse(m_Task[i]->getOptimized());
}

void PGTaskMgr::UpdateTaskPoint(size_t i, TASK_POINT& TskPt ) const {
    const GeoPoint position = getOptimized(i);

    TskPt.AATTargetLat = position.latitude;
    TskPt.AATTargetLon = position.longitude;

    UpdateTargetAltitude(TskPt);

    m_Task[i]->UpdateTaskPoint(i, TskPt);
}
