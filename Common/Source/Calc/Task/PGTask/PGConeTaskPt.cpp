/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   PGConeTaskPt.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 19 décembre 2013, 20:15
 */

#include "PGConeTaskPt.h"
#include "Draw/Task/TaskRendererMgr.h"

PGConeTaskPt::PGConeTaskPt(ProjPt&& point) 
    : PGCicrcleTaskPt(std::forward<ProjPt>(point)) { }

void PGConeTaskPt::Optimize(const ProjPt& prev, const ProjPt& next, double Alt) {

    m_Radius = ConeRadius(Alt, m_AltBase, m_Slope, m_RadiusBase);
    if(m_Radius > 0.0) {
        PGCicrcleTaskPt::Optimize(prev, ProjPt(0, 0), Alt);
    }
    else {
        m_Optimized = m_Center;
    }
}

double PGConeTaskPt::ConeRadius(double Alt, double AltBase, double Slope, double RadiusBase) {
    return std::max(0.0, (( Alt - AltBase ) * Slope) + RadiusBase);
}

void PGConeTaskPt::UpdateTaskPoint(size_t idx, TASK_POINT& TskPt ) const {
    if(TskPt.AATCircleRadius != m_Radius) {
        TskPt.AATCircleRadius = m_Radius;
        
        const WAYPOINT& TaskWpt = WayPointList[TskPt.Index];
        const GeoPoint center(TaskWpt.Latitude, TaskWpt.Longitude);

        gTaskSectorRenderer.SetCircle(idx, center, TskPt.AATCircleRadius);
    }
}
