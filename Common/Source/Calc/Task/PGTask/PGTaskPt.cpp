/*
 * PGTaskPt.cpp
 *
 *  Created on: 11 sept. 2012
 *      Author: Bruno
 */

#include "PGTaskPt.h"
#include "Library/newuoa.h"

PGTaskPt::PGTaskPt() : m_Radius(0), m_bExit(false) {

}

PGTaskPt::~PGTaskPt() {

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

void PGTaskPt::Optimize(const ProjPt& prev, const ProjPt& next) {
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
        min_newuoa<double, OptimizedDistance > (1, &x0, Fmin);
        m_Optimized = ProjPt(m_Center.m_X + m_Radius * cos(x0), m_Center.m_Y + m_Radius * sin(x0));
    }
}

bool PGTaskPt::CrossPoint(const ProjPt& prev, const ProjPt& next, ProjPt& optimized) {
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

void PGTaskPt::OptimizeFinishLine(const ProjPt& prev, const ProjPt& prevCenter) {

    ProjPt u = prevCenter - m_Center; // center to prev Vector
    double d = u.length();
    if (d > 0) {
        u = (u * m_Radius) / d;
        // rotate vector 90°
        ProjPt u2;
        u2.m_X = u.m_X * cos(PI / 2) - u.m_Y * sin(PI / 2);
        u2.m_Y = u.m_X * sin(PI / 2) + u.m_Y * cos(PI / 2);

        ProjPt B = m_Center + u2; // begin of line
        ProjPt A = m_Center - u2; // end of line

        ProjPt AB = B - A;
        ProjPt AC = prev - A;

        double theta = vector_angle(AB, AC);
        if (theta >= PI / 2) {
            m_Optimized = A;
        } else {
            double b = cos(theta) * AC.length();
            double dAB = 2 * m_Radius/*AB.length()*/;
            if (b < dAB) {
                m_Optimized = A + (AB / dAB * b);
            } else {
                m_Optimized = B;
            }
        }
    }
}

