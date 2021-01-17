/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   PGTaskPt.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 11 sept. 2012
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
        return (pt.m_X == m_X) && (pt.m_Y == m_Y);
    }

    inline bool operator!=(const ProjPt& pt) const {
        return !((*this) == pt);
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
        LKASSERT(c);
        return ProjPt(m_X / c, m_Y / c);
    }

    inline double length() const {
        return sqrt((m_X * m_X) + (m_Y * m_Y));
    }

    inline bool operator!() const {
        return m_X == 0.0 && m_Y == 0.0;
    }

    inline operator bool () const {
        return (*this) != null;
    }

    double m_X;
    double m_Y;

    const static ProjPt null;
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

class PGTaskPt {
    friend class PGTaskMgr;
public:
    PGTaskPt() = delete;
    PGTaskPt(ProjPt&& point);
    virtual ~PGTaskPt();

    inline const ProjPt& getOptimized() const {
        return (!m_Optimized) ? m_Center : m_Optimized;
    }

    inline const ProjPt& getCenter() const {
        return m_Center;
    }
    
    /*
     * @prev : previous optimized Target point
     * @next : next optimized target point
     * @Alt : estimated Arrival Altitude
     */
    virtual void Optimize(const ProjPt& prev, const ProjPt& next, double Alt) = 0;
    
    virtual bool UpdateTaskPoint(TASK_POINT& TskPt ) const;

protected:
    const ProjPt m_Center;
    ProjPt m_Optimized;
};

#endif /* PGTASKPT_H_ */
