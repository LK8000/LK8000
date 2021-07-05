/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *  
 * File:   PGLineTaskPt.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 6 octobre 2012, 13:21
 */

#ifndef PGLINETASKPT_H
#define	PGLINETASKPT_H

#include "PGTaskPt.h"

class PGLineTaskPt : public PGTaskPt {
    friend class PGTaskMgr;
public:
    PGLineTaskPt() = delete;
    PGLineTaskPt(ProjPt&& point);

    void Optimize(const ProjPt& prev, const ProjPt& next, double Alt) override;

protected:
    void OptimizeGoal(const ProjPt& prev);
    void OptimizeRegular(const ProjPt& prev, const ProjPt& next);

    ProjPt m_LineBegin = {0, 0};
    ProjPt m_LineEnd = {0, 0};

    ProjPt m_DirVector = {0, 0};

    // Internal Temp Var;
    ProjPt m_AB = {0, 0};
    ProjPt::scalar_type m_dAB = {};
};

#endif	/* PGLINETASKPT_H */
