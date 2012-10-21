/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *  
 * File:   PGCicrcleTaskPt.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 21 octobre 2012, 16:16
 */

#ifndef PGSECTORTASKPT_H
#define	PGSECTORTASKPT_H

#include "PGTaskPt.h"

class PGSectorTaskPt : public PGTaskPt {
    friend class PGTaskMgr;
public:
    PGSectorTaskPt();
    virtual ~PGSectorTaskPt();
    
    virtual void Optimize(const ProjPt& prev, const ProjPt& next);
    
protected:

};

#endif	/* PGSECTORTASKPT_H */

