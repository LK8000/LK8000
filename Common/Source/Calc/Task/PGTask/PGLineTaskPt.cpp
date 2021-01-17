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

    if (!m_AB) {
        // First Run, this change only if Task is modified
        m_AB = m_LineEnd - m_LineBegin;
        m_dAB = m_AB.length();
    }

    if (next) {
        OptimizeRegular(prev, next);
    } else {
        OptimizeGoal(prev);
    }
}

void PGLineTaskPt::OptimizeGoal(const ProjPt& prev) {
    ProjPt AC = prev - m_LineBegin;
    double theta = vector_angle(m_AB, AC);
    if (theta >= PI / 2) {
        m_Optimized = m_LineBegin;
    } else {
        double b = cos(theta) * AC.length();
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
    theCos = B.m_X / m_dAB;
    theSin = B.m_Y / m_dAB;
    newX = C.m_X * theCos + C.m_Y*theSin;
    C.m_Y = C.m_Y * theCos - C.m_X*theSin;
    C.m_X = newX;
    newX = D.m_X * theCos + D.m_Y*theSin;
    D.m_Y = D.m_Y * theCos - D.m_X*theSin;
    D.m_X = newX;

    //  if segment C-D doesn't cross line A-B Use Symmetrical point of D.
    if ((C.m_Y < 0. && D.m_Y < 0.) || (C.m_Y >= 0. && D.m_Y >= 0.)) {
        D.m_Y *= -1;
    }

    //  (3) Discover the position of the intersection point along line A-B.
    LKASSERT(D.m_Y - C.m_Y);
    ABpos = D.m_X + (C.m_X - D.m_X) * D.m_Y / (D.m_Y - C.m_Y);

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
    m_Optimized.m_X = m_LineBegin.m_X + ABpos*theCos;
    m_Optimized.m_Y = m_LineBegin.m_Y + ABpos*theSin;
}
