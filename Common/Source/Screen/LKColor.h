/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKColor.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 15 octobre 2014, 20:16
 */

#ifndef LKCOLOR_H
#define	LKCOLOR_H

#include "Screen/Color.hpp"

class LKColor : public Color {
public:
    LKColor() = default;

    LKColor ContrastTextColor() const;
    LKColor ChangeBrightness(double fBrightFact) const;
    LKColor MixColors(const LKColor& Color2, double fFact1) const;

    constexpr LKColor(uint8_t r, uint8_t g, uint8_t b) : Color(r,g,b) {}
    constexpr LKColor(Color _color) : Color(_color) {}

#ifdef USE_GDI
    constexpr explicit LKColor(COLORREF _color) : Color(_color) {}
#endif
};


#endif	/* LKCOLOR_H */
