/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   PGCicrcleTaskPt.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 6 octobre 2012, 12:27
 */

#include "PGTaskPt.h"
#include "PGCircleTaskPt.h"
#include "Library/newuoa.h"

PGCicrcleTaskPt::PGCicrcleTaskPt(ProjPt&& point) 
    : PGTaskPt(std::forward<ProjPt>(point)) { }

class OptimizedDistance {
public:

    OptimizedDistance(const ProjPt& prev, const ProjPt& cur, const ProjPt& next, const double radius) : m_prev(prev), m_cur(cur), m_next(next), m_radius(radius) {
    }

    double operator()(int n, double *theta) const {
        ProjPt optPoint(m_cur.m_X + m_radius * cos(*theta), m_cur.m_Y + m_radius * sin(*theta));

        ProjPt a = m_prev - optPoint;
        ProjPt b = m_next - optPoint;

        return a.length() + b.length();
    }
private:
    const ProjPt& m_prev;
    const ProjPt& m_cur;
    const ProjPt& m_next;
    const double m_radius;
};

void PGCicrcleTaskPt::Optimize(const ProjPt& prev, const ProjPt& next, double Alt) {
    if(m_Radius == 0.0){
        return;
    }

    if (!m_Optimized) {
        // first run : init m_Optimized with center ...
        m_Optimized = m_Center;
    }

    if (!CrossPoint(prev, next ? next : m_Center, m_Optimized)) {
        OptimizedDistance Fmin(prev, m_Center, next, m_Radius);
        double x0 = 0;
        double d1 = min_newuoa<double, OptimizedDistance > (1, &x0, Fmin, PI, 0.01 / m_Radius);
        if (m_bExit) {
            double x1 = x0 + PI;
            double d2 = min_newuoa<double, OptimizedDistance > (1, &x1, Fmin, PI, 0.01 / m_Radius);

            x0 = (std::min(d1, d2) == d1) ? x0 : x1;
        }
        m_Optimized = ProjPt(m_Center.m_X + m_Radius * cos(x0), m_Center.m_Y + m_Radius * sin(x0));
    }
}

bool PGCicrcleTaskPt::CrossPoint(const ProjPt& prev, const ProjPt& next, ProjPt& optimized) {
    ProjPt A = prev - m_Center;
    ProjPt B = next - m_Center;
    if(A == B) {
        // Next and prev is same point -> ignore next...
        B = ProjPt::null;
    }
    ProjPt A2(A.m_X * A.m_X, A.m_Y * A.m_Y);
    ProjPt B2(B.m_X * B.m_X, B.m_Y * B.m_Y);
    double R2 = (m_Radius * m_Radius);

    bool PrevOutside = (A2.m_X + A2.m_Y) > R2;
    bool NextOutside = (B2.m_X + B2.m_Y) > R2;

    if (!PrevOutside && !NextOutside) {
        return false; // no cross point
    }

    ProjPt AB = B - A;

    double a = (AB.m_X * AB.m_X) + (AB.m_Y * AB.m_Y);
    double b = 2 * ((AB.m_X * A.m_X) + (AB.m_Y * A.m_Y));
    double c = A2.m_X + A2.m_Y - R2;

    double bb4ac = (b * b) -(4 * a * c);
    if (bb4ac < 0.0) {
        return false;
    }

    bool bCrossPoint = false;
    double k = 0.0;
    if (bb4ac == 0.0) {
        LKASSERT(a);
        // one point
        k = -b / (2 * a);
        bCrossPoint = true;
    }

    if (bb4ac > 0.0) {
        // Two point, 
        if ((PrevOutside && m_bExit) || (!PrevOutside && NextOutside)) {
            LKASSERT(a);
            k = (-b + sqrt(bb4ac)) / (2 * a); // output : prev outside && Exit TP || prev inside && next outside
            bCrossPoint = true;
        } else {
            LKASSERT(a);
            k = (-b - sqrt(bb4ac)) / (2 * a); // input : prev outside && Enter TP 
            bCrossPoint = true;
        }
    }

    if (bCrossPoint) {
        ProjPt O = prev + ((next - prev) * k);
        if (dot_product((next - prev), O - prev) > 0.0 &&
                dot_product((prev - next), O - next) > 0.0) {
            optimized = O;
            return true;
        }
    }

    // no point
    return false;
}
