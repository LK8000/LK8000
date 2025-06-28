/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *  
 * File:   PGEssCircle.h
 * Author: Bruno de Lacheisserie
 *
 * Created on September 20, 2016, 11:47 PM
 */

#ifndef PGESSCIRCLE_H
#define PGESSCIRCLE_H

#include "PGCircleTaskPt.h"

class PGEssCircleTaskPt : public PGCircleTaskPt {
    friend class PGTaskOptimizer; 
public:
    PGEssCircleTaskPt() = delete;
    PGEssCircleTaskPt(ProjPt&& point, double Radius);
    
    void Optimize(const ProjPt& prev, const ProjPt& next) override;
};

#endif /* PGESSCIRCLE_H */
