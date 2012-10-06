/* 
 * File:   PGCicrcleTaskPt.cpp
 * Author: bruno
 * 
 * Created on 6 octobre 2012, 12:27
 */

#include "PGTaskPt.h"
#include "PGCicrcleTaskPt.h"
#include "Library/newuoa.h"

PGCicrcleTaskPt::PGCicrcleTaskPt(): m_Radius(0), m_bExit(false) {
}

PGCicrcleTaskPt::~PGCicrcleTaskPt() {
}

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

void PGCicrcleTaskPt::Optimize(const ProjPt& prev, const ProjPt& next) {
    if (!m_Optimized) {
        // first run : init m_Optimized with center ...
        m_Optimized = m_Center;
    }

    if (!CrossPoint(prev, !!next ? next : m_Center, m_Optimized)) {
        ProjPt O;
        if (m_Optimized == m_Center) {
            // First Pass : In with prev bearing if Entry Next Bearing if Exit.
            O = (m_bExit ? next : prev) - m_Center;
        } else {
            // For All next Pass use previous Point.
            O = m_Optimized - m_Center;
        }
        double x0 = atan2(O.m_Y, O.m_X);

        OptimizedDistance Fmin(prev, m_Center, next, m_Radius);
        double d1 = min_newuoa<double, OptimizedDistance > (1, &x0, Fmin);
        if(m_bExit) {
            if (m_Optimized == m_Center) {
                // First Pass : In with prev bearing if Entry Next Bearing if Exit.
                O = next - m_Center;
            } else {
                // For All next Pass use previous Point.
                O = m_Center - m_Optimized;
            }            
            double x1 = atan2(O.m_Y, O.m_X);
            double d2 = min_newuoa<double, OptimizedDistance > (1, &x1, Fmin);
            
            x0 = (std::min(d1,d2)==d1)?x0:x1;
        }
        m_Optimized = ProjPt(m_Center.m_X + m_Radius * cos(x0), m_Center.m_Y + m_Radius * sin(x0));
    }
}

bool PGCicrcleTaskPt::CrossPoint(const ProjPt& prev, const ProjPt& next, ProjPt& optimized) {
    bool PrevOutside = (((m_Center.m_X - prev.m_X)*(m_Center.m_X - prev.m_X)) + ((m_Center.m_Y - prev.m_Y)*(m_Center.m_Y - prev.m_Y))) > (m_Radius * m_Radius);
    bool NextOutside = (((m_Center.m_X - next.m_X)*(m_Center.m_X - next.m_X)) + ((m_Center.m_Y - next.m_Y)*(m_Center.m_Y - next.m_Y))) > (m_Radius * m_Radius);

    if (!PrevOutside && !NextOutside) {
        return false; // no cross point
    }

    double a = ((next.m_X - prev.m_X)*(next.m_X - prev.m_X)) + ((next.m_Y - prev.m_Y)*(next.m_Y - prev.m_Y));
    double b = 2 * (((next.m_X - prev.m_X)*(prev.m_X - m_Center.m_X)) + ((next.m_Y - prev.m_Y)*(prev.m_Y - m_Center.m_Y)));
    double c = (m_Center.m_X * m_Center.m_X)+(m_Center.m_Y * m_Center.m_Y)+(prev.m_X * prev.m_X)+(prev.m_Y * prev.m_Y)
            - (2 * ((m_Center.m_X * prev.m_X)+(m_Center.m_Y * prev.m_Y))) - (m_Radius * m_Radius);

    double bb4ac = (b * b) -(4 * a * c);
    if (bb4ac < 0.0) {
        return false;
    }

    bool bCrossPoint = false;
    double k = 0.0;
    if (bb4ac == 0.0) {
        // one point
        k = -b / (2 * a);
        bCrossPoint = true;
    }

    if (bb4ac > 0.0) {
        // Two point, 
        if ((PrevOutside && m_bExit) || (!PrevOutside && NextOutside)) {
            k = (-b + sqrt(bb4ac)) / (2 * a); // ouput : prev ouside && Exit TP || prev inside && next outside
            bCrossPoint = true;
        } else {
            k = (-b - sqrt(bb4ac)) / (2 * a); // input : prev outside && Enter TP 
            bCrossPoint = true;
        }
    }

    if (bCrossPoint) {
        ProjPt O = prev + ((next - prev) * k);

        if (dot_product((next - prev), optimized - prev) >= 0.0 &&
                dot_product((prev - next), optimized - next) >= 0.0) {
            optimized = O;
            return true;
        }
    }

    // no point
    return false;
}

