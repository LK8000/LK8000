/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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

namespace {

class OptimizedDistance final {
public:

    OptimizedDistance(const ProjPt& prev, const ProjPt& cur, const ProjPt& next, const double radius) 
            : m_prev(prev), m_cur(cur), m_next(next), m_radius(radius) { }

    double operator()(int n, double *theta) const {
        ProjPt optPoint(m_cur.x + m_radius * cos(*theta), m_cur.y + m_radius * sin(*theta));

        ProjPt a = m_prev - optPoint;
        ProjPt b = m_next - optPoint;

        return Length(a) + Length(b);
    }
private:
    const ProjPt& m_prev;
    const ProjPt& m_cur;
    const ProjPt& m_next;
    const double m_radius;
};

} // namespace

PGCicrcleTaskPt::PGCicrcleTaskPt(ProjPt&& point) 
    : PGTaskPt(std::forward<ProjPt>(point)) { }

void PGCicrcleTaskPt::Optimize(const ProjPt& prev, const ProjPt& next, double Alt) {
    if(m_Radius == 0.0){
        return;
    }

    if (IsNull(m_Optimized)) {
        // first run : init m_Optimized with center ...
        m_Optimized = m_Center;
    }
    const auto& a = prev;
    const auto& b = IsNull(next) ? m_Center : next;

    if (!CrossPoint(a, b, m_Optimized)) {
        OptimizedDistance Fmin(a, m_Center, b, m_Radius);
        double x0 = 0;
        double d1 = min_newuoa<double, OptimizedDistance > (1, &x0, Fmin, PI, 0.01 / m_Radius);
        if (m_bExit) {
            double x1 = x0 + PI;
            double d2 = min_newuoa<double, OptimizedDistance > (1, &x1, Fmin, PI, 0.01 / m_Radius);

            x0 = (std::min(d1, d2) == d1) ? x0 : x1;
        }
        m_Optimized = ProjPt(m_Center.x + m_Radius * cos(x0), m_Center.y + m_Radius * sin(x0));
    }
}

bool PGCicrcleTaskPt::CrossPoint(const ProjPt& prev, const ProjPt& next, ProjPt& optimized) {
    ProjPt A = prev - m_Center;
    ProjPt B = next - m_Center;
    if(A == B) {
        // Next and prev is same point -> ignore next...
        B = ProjPt(0, 0);
    }
    ProjPt A2(A.x * A.x, A.y * A.y);
    ProjPt B2(B.x * B.x, B.y * B.y);
    ProjPt::scalar_type R2 = (m_Radius * m_Radius);

    bool PrevOutside = (A2.x + A2.y) > R2;
    bool NextOutside = (B2.x + B2.y) > R2;

    if (!PrevOutside && !NextOutside) {
        return false; // no cross point
    }

    ProjPt AB = B - A;

    ProjPt::scalar_type a = (AB.x * AB.x) + (AB.y * AB.y);
    ProjPt::scalar_type b = 2 * ((AB.x * A.x) + (AB.y * A.y));
    ProjPt::scalar_type c = A2.x + A2.y - R2;

    double bb4ac = (b * b) -(4 * a * c);
    if (bb4ac < 0) {
        return false;
    }

    bool bCrossPoint = false;
    double k = 0;
    if (bb4ac == 0) {
        LKASSERT(a);
        // one point
        k = -b / (2 * a);
        bCrossPoint = true;
    }

    if (bb4ac > 0) {
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
        if (DotProduct((next - prev), O - prev) > 0 &&
                DotProduct((prev - next), O - next) > 0) {
            optimized = O;
            return true;
        }
    }

    // no point
    return false;
}
