/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   PGLineTaskPt.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 6 octobre 2012, 13:21
 */

#include "PGLineTaskPt.h"

PGLineTaskPt::PGLineTaskPt(ProjPt&& point)
    : PGTaskPt(std::forward<ProjPt>(point)), m_dAB() {
}

void PGLineTaskPt::Optimize(const ProjPt& prev, const ProjPt& next, double Alt) {
    //  Fail if either line segment is zero-length.
    if (m_LineBegin == m_LineEnd || prev == next) {
        return;
    }

    //  if the segments share an end-point.
    // return this end Point
    if (m_LineBegin == prev || m_LineBegin == next) {
        m_Optimized = m_LineBegin;
        return;
    }
    if (m_LineEnd == prev || m_LineEnd == next) {
        m_Optimized = m_LineEnd;
        return;
    }

    if (IsNull(m_AB)) {
        // First Run, this change only if Task is modified
        m_AB = m_LineEnd - m_LineBegin;
        m_dAB = Length(m_AB);
    }

    if (IsNull(next)) {
        OptimizeGoal(prev);
    } else {
        OptimizeRegular(prev, next);
    }
}

void PGLineTaskPt::OptimizeGoal(const ProjPt& prev) {
    ProjPt AC = prev - m_LineBegin;
    double theta = VectorAngle(m_AB, AC);
    if (theta >= PI / 2) {
        m_Optimized = m_LineBegin;
    } else {
        ProjPt::scalar_type b = cos(theta) * Length(AC);
        if (b < m_dAB) {
            LKASSERT(m_dAB);
            LKASSERT(b);
            m_Optimized = m_LineBegin + (m_AB / m_dAB * b);
        } else {
            m_Optimized = m_LineEnd;
        }
    }
}

void PGLineTaskPt::OptimizeRegular(const ProjPt& prev, const ProjPt& next) {
    double newX, theCos, theSin, ABpos;

    //  (1) Translate the system so that point m_LigneBegin is on the origin.
    ProjPt B = m_AB;
    ProjPt C = prev - m_LineBegin;
    ProjPt D = next - m_LineBegin;

    //  (2) Rotate the system so that point B is on the positive X axis.
    LKASSERT(m_dAB);
    theCos = B.x / m_dAB;
    theSin = B.y / m_dAB;
    newX = C.x * theCos + C.y*theSin;
    C.y = C.y * theCos - C.x*theSin;
    C.x = newX;
    newX = D.x * theCos + D.y*theSin;
    D.y = D.y * theCos - D.x*theSin;
    D.x = newX;

    //  if segment C-D doesn't cross line A-B Use Symmetrical point of D.
    if ((C.y < 0. && D.y < 0.) || (C.y >= 0. && D.y >= 0.)) {
        D.y *= -1;
    }

    //  (3) Discover the position of the intersection point along line A-B.
    LKASSERT(D.y - C.y);
    ABpos = D.x + (C.x - D.x) * D.y / (D.y - C.y);

    //  if segment C-D crosses line A-B outside of segment A-B return nearest endpoint
    if (ABpos < 0.) {
        m_Optimized = m_LineBegin;
        return;
    }

    if (ABpos > m_dAB) {
        m_Optimized = m_LineEnd;
        return;
    }

    //  (4) Apply the discovered position to line A-B in the original coordinate system.
    m_Optimized.x = m_LineBegin.x + ABpos*theCos;
    m_Optimized.y = m_LineBegin.y + ABpos*theSin;
}
