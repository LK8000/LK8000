/*
 * PGTaskPt.h
 *
 *  Created on: 11 sept. 2012
 *      Author: Bruno
 */

#ifndef PGTASKPT_H_
#define PGTASKPT_H_

#include "externs.h"

class ProjPt {
public:

    inline ProjPt() : m_X(0.0), m_Y(0.0) {
    }

    inline ProjPt(const double x, const double y) : m_X(x), m_Y(y) {
    }

    inline bool operator==(const ProjPt& pt) const {
        return (pt.m_Y == m_Y) && (pt.m_Y == m_Y);
    }

    inline bool operator!=(const ProjPt& pt) const {
        return (pt.m_Y != m_Y) && (pt.m_Y != m_Y);
    }

    inline ProjPt operator-(const ProjPt& pt) const {
        return ProjPt(m_X - pt.m_X, m_Y - pt.m_Y);
    }
    inline ProjPt operator-() const {
        return ProjPt(-m_X, -m_Y);
    }

    inline ProjPt operator+(const ProjPt& pt) const {
        return ProjPt(m_X + pt.m_X, m_Y + pt.m_Y);
    }

    inline ProjPt operator*(const double c) const {
        return ProjPt(c * m_X, c * m_Y);
    }

    inline ProjPt operator/(const double c) const {
        return ProjPt(m_X / c, m_Y / c);
    }

    inline double length() const {
        return sqrt((m_X * m_X) + (m_Y * m_Y));
    }

    inline bool operator!() const {
        return m_X == 0.0 && m_Y == 0.0;
    }

    double m_X;
    double m_Y;
};

class PGTaskPt {
public:
    PGTaskPt();
    virtual ~PGTaskPt();

    const ProjPt& getOptimized() const;
    void Optimize(const ProjPt& prev, const ProjPt& next);

    bool CrossPoint(const ProjPt& prev, const ProjPt& next, ProjPt& optimized);

    ProjPt m_Center;
    ProjPt m_Optimized;
    double m_Radius;
    bool m_bExit;
};

inline double dot_product(const ProjPt& v, const ProjPt& w) {
    return (v.m_X * w.m_X + v.m_Y * w.m_Y);
}

inline double vector_angle(const ProjPt& v, const ProjPt& w) {
    double dot = dot_product(v, w);
    double d1 = v.length();
    double d2 = w.length();
    if ((d1 * d2) != 0.0) {
        return acos(dot / (d1 * d2));
    }
    return 0.0;
}


#endif /* PGTASKPT_H_ */
