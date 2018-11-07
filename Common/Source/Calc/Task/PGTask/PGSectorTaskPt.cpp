/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   PGSectorTaskPt.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 21 octobre 2012, 16:16
 */

#include "PGSectorTaskPt.h"

PGSectorTaskPt::PGSectorTaskPt() {
}

PGSectorTaskPt::~PGSectorTaskPt() {
}

void PGSectorTaskPt::Optimize(const ProjPt& prev, const ProjPt& next, double Alt) {
    m_Optimized = m_Center;
    
    //TODO : optimize Sector Turn Point
}
