/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKBrush.cpp
 * Author: Bruno de Lacheisserie
 * 
 * Created on 23 octobre 2014, 20:28
 */
#ifdef USE_GDI
#include <windows.h>
#endif

#include "LKColor.h"
#include "LKBrush.h"
#include "LKBitmap.h"

#include <utility>

#ifdef USE_GDI

const LKBrush LK_WHITE_BRUSH = LKBrush::MakeStock(WHITE_BRUSH);
const LKBrush  LK_BLACK_BRUSH = LKBrush::MakeStock(BLACK_BRUSH);
const LKBrush  LK_HOLLOW_BRUSH = LKBrush::MakeStock(HOLLOW_BRUSH);

LKBrush::LKBrush(LKBrush&& Brush) {
    brush = Brush.brush;
    Brush.brush = nullptr;
}

#else
#warning "TODO : ..."
const LKBrush  LK_WHITE_BRUSH;
const LKBrush  LK_BLACK_BRUSH;
const LKBrush  LK_HOLLOW_BRUSH;

#endif


LKBrush::~LKBrush() {

}

LKBrush& LKBrush::operator= (LKBrush&& Brush) {
#ifdef USE_GDI
    std::swap(brush, Brush.brush);
#else
    std::swap(color, Brush.color);
#endif    
    return *this;
}
