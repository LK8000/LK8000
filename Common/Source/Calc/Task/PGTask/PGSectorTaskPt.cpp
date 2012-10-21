/* 
 * File:   PGSectorTaskPt.cpp
 * Author: bruno
 * 
 * Created on 21 octobre 2012, 16:16
 */

#include "PGSectorTaskPt.h"

PGSectorTaskPt::PGSectorTaskPt() {
}

PGSectorTaskPt::~PGSectorTaskPt() {
}

void PGSectorTaskPt::Optimize(const ProjPt& prev, const ProjPt& next) {
    m_Optimized = m_Center;
    
    //TODO : optimize Sector Turn Point
}
