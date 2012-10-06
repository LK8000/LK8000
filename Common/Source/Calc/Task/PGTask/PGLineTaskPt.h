/* 
 * File:   PGLineTaskPt.h
 * Author: bruno
 *
 * Created on 6 octobre 2012, 13:21
 */

#ifndef PGLINETASKPT_H
#define	PGLINETASKPT_H

#include "PGTaskPt.h"

class PGLineTaskPt : public PGTaskPt {
    friend class PGTaskMgr;
public:
    PGLineTaskPt();
    virtual ~PGLineTaskPt();

    virtual void Optimize(const ProjPt& prev, const ProjPt& next);

protected:
    void OptimizeGoal(const ProjPt& prev);
    void OptimizeRegular(const ProjPt& prev, const ProjPt& next);
    
    ProjPt m_LineBegin;
    ProjPt m_LineEnd;

    ProjPt m_DirVector;
    
    // Internal Temp Var;
    ProjPt m_AB;
    double m_dAB;
};

#endif	/* PGLINETASKPT_H */

