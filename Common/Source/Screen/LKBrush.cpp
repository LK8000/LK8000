/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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

const HBRUSH LK_WHITE_BRUSH((HBRUSH)GetStockObject(WHITE_BRUSH));
const HBRUSH LK_BLACK_BRUSH((HBRUSH)GetStockObject(BLACK_BRUSH));
const HBRUSH LK_HOLLOW_BRUSH((HBRUSH)GetStockObject(HOLLOW_BRUSH));

LKBrush::LKBrush(LKBrush&& Brush) {
    brush = Brush.brush;
    Brush.brush = nullptr;
}

#else
const LKBrush  LK_WHITE_BRUSH(COLOR_WHITE);
const LKBrush  LK_BLACK_BRUSH(COLOR_BLACK);
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
