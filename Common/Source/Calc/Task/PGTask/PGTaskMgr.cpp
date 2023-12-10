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

namespace {

    /**
     * T must inherit from PGTaskPt
     */
    template<typename T, typename ...Args>
    std::enable_if_t<std::is_base_of_v<PGTaskPt, T>, std::unique_ptr<T>>
    getPGTaskPt(const TransverseMercator* pProjection, size_t idx, Args&&... args) {
        assert(pProjection);
        return std::make_unique<T>(pProjection->Forward(GetTurnpointPosition(idx)), std::forward<Args>(args)...);
    }

} // namespace

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
        sector_param param = GetTaskSectorParameter(curwp);
        switch (param.type) {
            case sector_type_t::CIRCLE:
                AddCircle(curwp, param.radius);
                break;
            case sector_type_t::SECTOR:
            case sector_type_t::DAe:
                AddSector(curwp);
                break;
            case sector_type_t::LINE:
                AddLine(curwp, param.radius);
                break;
            case sector_type_t::ESS_CIRCLE:
                AddEssCircle(curwp, param.radius);
                break;
        }
    }
}

void PGTaskMgr::AddCircle(int TskIdx, double Radius) {
    auto pTskPt = getPGTaskPt<PGCircleTaskPt>(m_Projection.get(), TskIdx, Radius);
    m_Task.emplace_back(std::move(pTskPt));
}

void PGTaskMgr::AddEssCircle(int TskIdx, double Radius) {
    auto pTskPt = getPGTaskPt<PGEssCircleTaskPt>(m_Projection.get(), TskIdx, Radius);
    m_Task.emplace_back(std::move(pTskPt));
}

void PGTaskMgr::AddLine(int TskIdx, double Radius) {
    auto pTskPt = getPGTaskPt<PGLineTaskPt>(m_Projection.get(), TskIdx);

    // Find prev Tp not same as current.
    int PrevIdx = TskIdx - 1;
    while (PrevIdx > 0 && Task[PrevIdx].Index == Task[TskIdx].Index) {
        --PrevIdx;
    }

    // Find next Tp not same as current.
    int NextIdx = TskIdx + 1;
    while (ValidTaskPointFast(NextIdx) && Task[NextIdx].Index == Task[TskIdx].Index) {
        ++NextIdx;
    }

    // Calc Cross Dir Vector
    ProjPt InB = { 0, 0 };
    ProjPt OutB = { 0, 0 };
    if (ValidTaskPointFast(NextIdx)) {
        OutB = m_Projection->Forward(GetTurnpointPosition(NextIdx));
        OutB = Normalize(OutB - pTskPt->m_Center);
    } else if (PrevIdx >= 0) {
        InB = m_Task[PrevIdx]->getCenter();
        InB = Normalize(pTskPt->m_Center - InB);
    }

    if (!IsNull(InB) && !IsNull(OutB)) {
        pTskPt->m_DirVector = Normalize(InB + OutB);
    } else if (!IsNull(InB)) {
        pTskPt->m_DirVector = InB;
    } else if (!IsNull(OutB)) {
        pTskPt->m_DirVector = OutB;
    }

    // Calc begin and end off line.
    ProjPt::scalar_type d = Length(pTskPt->m_DirVector);
    if (d > 0) {
        ProjPt u = Rotate90(pTskPt->m_DirVector) * Radius;
        pTskPt->m_LineBegin = pTskPt->m_Center + u; // begin of line
        pTskPt->m_LineEnd = pTskPt->m_Center - u; // end of line
    }

    m_Task.emplace_back(std::move(pTskPt));
}

void PGTaskMgr::AddSector(int TskIdx) {
    m_Task.emplace_back(getPGTaskPt<PGSectorTaskPt>(m_Projection.get(), TskIdx));

	//TODO : Handle Sector Turn Point
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
