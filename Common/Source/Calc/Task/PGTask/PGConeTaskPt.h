/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *  
 * File:   PGConeTaskPt.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 19 décembre 2013, 20:15
 */

#ifndef PGCONETASKPT_H
#define	PGCONETASKPT_H

#include "PGCicrcleTaskPt.h"

class PGConeTaskPt : public PGCicrcleTaskPt {
    friend class PGTaskMgr;
public:
    PGConeTaskPt();
    virtual ~PGConeTaskPt();
    
    virtual void Optimize(const ProjPt& prev, const ProjPt& next, double Alt);
    virtual void UpdateTaskPoint(TASK_POINT& TskPt ) const;

	static double ConeRadius(double Alt, double AltBase, double Slope, double RadiusBase);

protected:
    double m_Slope;
    double m_AltBase;
    double m_RadiusBase;
};

#endif	/* PGCONETASKPT_H */

