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

typedef Point2D<double> ProjPt;

inline
bool IsNull(const ProjPt& p) {
    return p == ProjPt(0., 0.);
}


inline double VectorAngle(const ProjPt& v, const ProjPt& w) {
    ProjPt::scalar_type dot = DotProduct(v, w);
    ProjPt::scalar_type d2 = Length(v) * Length(w);
    if (d2 != 0.0) {
        return acos(dot / d2);
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
        return IsNull(m_Optimized) ? m_Center : m_Optimized;
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
    ProjPt m_Optimized = { 0, 0 };
};

#endif /* PGTASKPT_H_ */
