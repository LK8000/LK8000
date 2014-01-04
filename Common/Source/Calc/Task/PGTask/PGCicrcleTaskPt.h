/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *  
 * File:   PGCicrcleTaskPt.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 6 octobre 2012, 12:27
 */

#ifndef PGCICRCLETASKPT_H
#define	PGCICRCLETASKPT_H

#include "PGTaskPt.h"

class PGCicrcleTaskPt : public PGTaskPt {
    friend class PGTaskMgr;
public:
    PGCicrcleTaskPt();
    virtual ~PGCicrcleTaskPt();

    virtual void Optimize(const ProjPt& prev, const ProjPt& next, double Alt);

protected: 
    bool CrossPoint(const ProjPt& prev, const ProjPt& next, ProjPt& optimized);

    double m_Radius;
    bool m_bExit;
};

#endif	/* PGCICRCLETASKPT_H */
