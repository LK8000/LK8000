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

#include <utility>
#include "Screen/Color.hpp"

class LKColor : public Color {
public:
    using Color::Color;

    constexpr LKColor(Color other) noexcept : Color(std::move(other)) {}

    LKColor ContrastTextColor() const noexcept;
    LKColor ChangeBrightness(double fBrightFact) const noexcept;
    LKColor MixColors(const LKColor& Color2, double fFact1) const noexcept;
};


#endif	/* LKCOLOR_H */
