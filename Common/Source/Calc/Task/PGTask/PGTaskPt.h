/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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

typedef Point2D<double> ProjPt;

inline
bool IsNull(const ProjPt& p) {
    return p == ProjPt(0., 0.);
}

class PGTaskPt {
    friend class PGTaskOptimizer;
public:
    PGTaskPt() = delete;
    PGTaskPt(ProjPt&& point) : m_Center(std::forward<ProjPt>(point)) { };
    virtual ~PGTaskPt() {};

    const ProjPt& getOptimized() const {
        return IsNull(m_Optimized) ? m_Center : m_Optimized;
    }

    const ProjPt& getCenter() const {
        return m_Center;
    }

    /*
     * @prev : previous optimized Target point
     * @next : next optimized target point
     * @Alt : estimated Arrival Altitude
     */
    virtual void Optimize(const ProjPt& prev, const ProjPt& next) = 0;

    virtual void UpdateTaskPoint(size_t idx, TASK_POINT& TskPt ) const {};

protected:
    const ProjPt m_Center;
    ProjPt m_Optimized = { 0, 0 };
};

#endif /* PGTASKPT_H_ */
