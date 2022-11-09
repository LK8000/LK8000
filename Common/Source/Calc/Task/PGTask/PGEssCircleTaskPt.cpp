/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *  
 * File:   PGEssCircle.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on September 20, 2016, 11:47 PM
 */

#include "PGEssCircleTaskPt.h"

PGEssCircleTaskPt::PGEssCircleTaskPt(ProjPt&& point, double Radius)
    : PGCircleTaskPt(std::forward<ProjPt>(point), Radius) { }

void PGEssCircleTaskPt::Optimize(const ProjPt& prev, const ProjPt& next, double Alt) {
    PGCircleTaskPt::Optimize(prev, ProjPt(0.,0.), Alt);
}
