/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *  
 * File:   PGCircleTaskPt.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 6 octobre 2012, 12:27
 */

#ifndef PGCIRCLETASKPT_H
#define	PGCIRCLETASKPT_H

#include "PGTaskPt.h"

class PGCircleTaskPt : public PGTaskPt {
    friend class PGTaskMgr;
public:
    PGCircleTaskPt() = delete;
    PGCircleTaskPt(ProjPt&& point, double Radius);

    void Optimize(const ProjPt& prev, const ProjPt& next) override;

protected: 
    bool CrossPoint(const ProjPt& prev, const ProjPt& next, ProjPt& optimized);

    double m_Radius = 0;
};

#endif	/* PGCIRCLETASKPT_H */
