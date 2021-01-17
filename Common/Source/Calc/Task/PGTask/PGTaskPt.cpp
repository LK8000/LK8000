/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   PGTaskPt.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 11 sept. 2012
 */

#include "PGTaskPt.h"

PGTaskPt::PGTaskPt(ProjPt&& point)
    : m_Center(std::forward<ProjPt>(point)) { }

PGTaskPt::~PGTaskPt() { }
