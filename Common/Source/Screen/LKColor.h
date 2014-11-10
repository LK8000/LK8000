/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKColor.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 15 octobre 2014, 20:16
 */

#ifndef LKCOLOR_H
#define	LKCOLOR_H

#include <stdint.h>

class LKColor {
public:
    LKColor() = default;

    LKColor ContrastTextColor() const;
    LKColor ChangeBrightness(double fBrightFact) const;
    LKColor MixColors(const LKColor& Color2, double fFact1) const;

#ifdef WIN32
    constexpr LKColor(uint8_t r, uint8_t g, uint8_t b) : _Color(RGB(r,g,b)) {}
    constexpr explicit LKColor(COLORREF Color) : _Color(Color) {}

    constexpr operator COLORREF() const { return _Color; }

    constexpr uint8_t Red() const { return GetRValue(_Color); }
    constexpr uint8_t Green() const { return GetGValue(_Color); }
    constexpr uint8_t Blue() const { return GetBValue(_Color) ; }
protected:
    COLORREF _Color;
#elif __linux__
    constexpr LKColor(uint8_t r, uint8_t g, uint8_t b) : _Red(r), _Green(g), _Blue(b) {};
    
    constexpr uint8_t Red() const { return _Red; }
    constexpr uint8_t Green() const { return _Green; };
    constexpr uint8_t Blue() const { return _Blue; };
    
    constexpr bool operator==(const LKColor& Color) { return (Red() == Color.Red() && Green()==Color.Green() && Blue()==Color.Blue()); }
    
protected:
    uint8_t _Red;
    uint8_t _Green;
    uint8_t _Blue;
    
#endif

};


#endif	/* LKCOLOR_H */
