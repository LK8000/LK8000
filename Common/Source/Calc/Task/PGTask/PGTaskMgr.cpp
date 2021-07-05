/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
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
#include "PGConeTaskPt.h"
#include "PGEssCircleTaskPt.h"
#include "NavFunctions.h"
#include "Geographic/GeoPoint.h"
#include "Geographic/TransverseMercator.h"
#include "Draw/Task/TaskRendererMgr.h"

namespace {

    inline
    GeoPoint GetTurnpointPosition(size_t idx) {
        return GeoPoint(WayPointList[Task[idx].Index].Latitude,
                WayPointList[Task[idx].Index].Longitude);
    }

    template<typename T> inline
    std::unique_ptr<T> getPGTaskPt(const TransverseMercator* pProjection, size_t idx) {
        static_assert(std::is_base_of<PGTaskPt, T>::value, "T must inherit from PGTaskPt");
        return std::make_unique<T>(pProjection->Forward(GetTurnpointPosition(idx)));
    }

} // namespace

void PGTaskMgr::Initialize() {

    m_Task.clear();

    // build Mercator Reference Grid
    // find center of Task
    double minlat = 0.0, minlon = 0.0, maxlat = 0.0, maxlon = 0.0;
    for (int curwp = 0; ValidTaskPoint(curwp); ++curwp) {
        if (curwp == 0) {
            maxlat = minlat = WayPointList[Task[curwp].Index].Latitude;
            maxlon = minlon = WayPointList[Task[curwp].Index].Longitude;
        } else {
            minlat = std::min(minlat, WayPointList[Task[curwp].Index].Latitude);
            maxlat = std::max(maxlat, WayPointList[Task[curwp].Index].Latitude);

            minlon = std::min(minlon, WayPointList[Task[curwp].Index].Longitude);
            maxlon = std::max(maxlon, WayPointList[Task[curwp].Index].Longitude);
        }
    }

    const GeoPoint center((minlat + maxlat) / 2, (minlon + maxlon) / 2);
    m_Projection = std::make_unique<TransverseMercator>(center);

    // build task point list
    for (int curwp = 0; ValidTaskPoint(curwp); ++curwp) {
        int TpType = 0;
        double Radius;
        GetTaskSectorParameter(curwp, &TpType, &Radius);
        switch (TpType) {
            case CIRCLE:
                AddCircle(curwp);
                break;
            case SECTOR:
            case DAe:
                AddSector(curwp);
                break;
            case LINE:
                AddLine(curwp);
                break;
            case CONE:
                AddCone(curwp);
                break;
            case ESS_CIRCLE:
                AddEssCircle(curwp);
                break;
        }
    }
}

void PGTaskMgr::AddCircle(int TskIdx) {
    assert(m_Projection);

    auto pTskPt = getPGTaskPt<PGCicrcleTaskPt>(m_Projection.get(), TskIdx);

    if (TskIdx == 0) {
        pTskPt->m_Radius = StartRadius;
    } else if (ValidTaskPoint(TskIdx + 1)) {
        pTskPt->m_Radius = (Task[TskIdx].AATCircleRadius);
    } else {
        pTskPt->m_Radius = FinishRadius;
    }

    pTskPt->m_bExit = ((TskIdx > 0) ? (Task[TskIdx].OutCircle) : !PGStartOut);

    m_Task.emplace_back(std::move(pTskPt));
}

void PGTaskMgr::AddEssCircle(int TskIdx) {
    assert(m_Projection);

    auto pTskPt = getPGTaskPt<PGEssCicrcleTaskPt>(m_Projection.get(), TskIdx);

    if (TskIdx == 0) {
        pTskPt->m_Radius = StartRadius;
    } else if (ValidTaskPoint(TskIdx + 1)) {
        pTskPt->m_Radius = (Task[TskIdx].AATCircleRadius);
    } else {
        pTskPt->m_Radius = FinishRadius;
    }

    pTskPt->m_bExit = ((TskIdx > 0) ? (Task[TskIdx].OutCircle) : !PGStartOut);

    m_Task.emplace_back(std::move(pTskPt));
}

void PGTaskMgr::AddLine(int TskIdx) {
    assert(m_Projection);
    if(!m_Projection) {
        return;
    }

    auto pTskPt = getPGTaskPt<PGLineTaskPt>(m_Projection.get(), TskIdx);

    double radius = 0;
    if (TskIdx == 0) {
        radius = StartRadius;
    } else if (ValidTaskPoint(TskIdx + 1)) {
        radius = (Task[TskIdx].AATCircleRadius);
    } else {
        radius = FinishRadius;
    }

    // Find prev Tp not same as current.
    int PrevIdx = TskIdx - 1;
    while (PrevIdx > 0 && Task[PrevIdx].Index == Task[TskIdx].Index) {
        --PrevIdx;
    }

    // Find next Tp not same as current.
    int NextIdx = TskIdx + 1;
    while (ValidTaskPoint(NextIdx) && Task[NextIdx].Index == Task[TskIdx].Index) {
        ++NextIdx;
    }

    // Calc Cross Dir Vector
    ProjPt InB, OutB;
    if (ValidTaskPoint(NextIdx)) {
        OutB = m_Projection->Forward(GetTurnpointPosition(TskIdx));
        OutB = OutB - pTskPt->m_Center;

        ProjPt::scalar_type d = Length(OutB);
        if (d != 0.0) {
            OutB = OutB / d;
        }
    } else if (PrevIdx >= 0) {
        InB = m_Task[PrevIdx]->getCenter();

        InB = pTskPt->m_Center - InB;

        ProjPt::scalar_type d = Length(InB);
        if (d != 0.0) {
            InB = InB / d;
        }
    }

    if (!IsNull(InB) && !IsNull(OutB)) {
        pTskPt->m_DirVector = InB + OutB;
        ProjPt::scalar_type d = Length(pTskPt->m_DirVector);
        if (d != 0.0) {
            pTskPt->m_DirVector = pTskPt->m_DirVector / d;
        }
    } else if (!IsNull(InB)) {
        pTskPt->m_DirVector = InB;
    } else if (!IsNull(OutB)) {
        pTskPt->m_DirVector = OutB;
    }

    // Calc begin and end off line.
    ProjPt::scalar_type d = Length(pTskPt->m_DirVector);
    if (d > 0) {
        // rotate vector 90°
        ProjPt u;
        u.x = pTskPt->m_DirVector.x * cos(PI / 2) - pTskPt->m_DirVector.y * sin(PI / 2);
        u.y = pTskPt->m_DirVector.x * sin(PI / 2) + pTskPt->m_DirVector.y * cos(PI / 2);

        u = u * radius;

        pTskPt->m_LineBegin = pTskPt->m_Center + u; // begin of line
        pTskPt->m_LineEnd = pTskPt->m_Center - u; // end of line
    }

    m_Task.emplace_back(std::move(pTskPt));
}

void PGTaskMgr::AddSector(int TskIdx) {
    assert(m_Projection);

    m_Task.emplace_back(getPGTaskPt<PGSectorTaskPt>(m_Projection.get(), TskIdx));

	//TODO : Handle Sector Turn Point
}

void PGTaskMgr::AddCone(int TskIdx) {
    assert(m_Projection);

    auto pTskPt = getPGTaskPt<PGConeTaskPt>(m_Projection.get(), TskIdx);

    pTskPt->m_Slope = Task[TskIdx].PGConeSlope;
    pTskPt->m_AltBase = Task[TskIdx].PGConeBase;
    pTskPt->m_RadiusBase = Task[TskIdx].PGConeBaseRadius;

    pTskPt->m_bExit = false;
//    pTskPt->m_bExit = ((TskIdx > 0) ? (Task[TskIdx].OutCircle) : !PGStartOut);

    m_Task.emplace_back(std::move(pTskPt));
}

void PGTaskMgr::Optimize(NMEA_INFO *Basic, DERIVED_INFO *Calculated) {
    if (((size_t) ActiveTaskPoint) >= m_Task.size()) {
        return;
    }
    assert(m_Projection);

    GeoPoint prev_position(Basic->Latitude, Basic->Longitude);
    ProjPt PrevPos = m_Projection->Forward(prev_position);
    
    double NextAltitude = Basic->Altitude;
    for (size_t i = ActiveTaskPoint; i < m_Task.size(); ++i) {
        
        // Calc Arrival Altitude
        const GeoPoint position = getOptimized(i);
        double Distance, Bearing;
        prev_position.Reverse(position, Bearing, Distance);
        double GrndAlt = AltitudeFromTerrain(position.latitude, position.longitude);
        if(NextAltitude > GrndAlt) {
            NextAltitude  -= GlidePolar::MacCreadyAltitude( MACCREADY, Distance, Bearing, Calculated->WindSpeed, Calculated->WindBearing, 0, 0, true, 0);
        }

        if(NextAltitude < GrndAlt) {
            NextAltitude = GrndAlt;
        }
        
        // Optimize
        if ((i + 1) < m_Task.size()) {
            m_Task[i]->Optimize(PrevPos, m_Task[i + 1]->getOptimized(), NextAltitude);
        } else {
            m_Task[i]->Optimize(PrevPos, ProjPt(0, 0), NextAltitude);
        }

        // Update previous Position for Next Loop
        PrevPos = m_Task[i]->getOptimized();
        prev_position = position;
    }
}

GeoPoint PGTaskMgr::getOptimized(const int i) const {
    assert(m_Projection);
    return m_Projection->Reverse(m_Task[i]->getOptimized());
}

void PGTaskMgr::UpdateTaskPoint(const int i, TASK_POINT& TskPt ) const {
    const GeoPoint position = getOptimized(i);

    TskPt.AATTargetLat = position.latitude;
    TskPt.AATTargetLon = position.longitude;

    UpdateTargetAltitude(TskPt);

    m_Task[i]->UpdateTaskPoint(i, TskPt);
}
