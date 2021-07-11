/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKPen.cpp
 * Author: bruno de Lacheisserie
 *
 * Created on 16 octobre 2014
 */

#include "LKPen.h"
#include "LKColor.h"

#include <utility>

#ifdef USE_GDI

const HPEN LK_NULL_PEN((HPEN)GetStockObject(NULL_PEN));
const HPEN LK_BLACK_PEN((HPEN)GetStockObject(BLACK_PEN));
const HPEN LK_WHITE_PEN((HPEN)GetStockObject(WHITE_PEN));

#else

const LKPen LK_NULL_PEN(Pen::BLANK, 0, COLOR_WHITE);
const LKPen LK_BLACK_PEN(Pen::SOLID, 1, COLOR_BLACK);
const LKPen LK_WHITE_PEN(Pen::SOLID, 1, COLOR_WHITE);

#endif
