/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKFont.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 23 octobre 2014, 20:28
 */

#ifndef LKFONT_H
#define	LKFONT_H

#include "Screen/Font.hpp"
#ifndef USE_GDI
#include "Screen/Custom/Cache.hpp"
#endif

class LKFont final : public Font {
public:
    LKFont() {};
    LKFont(LKFont&& Font) = delete;
    LKFont(const LKFont& Font) = delete;
    ~LKFont() { }

    LKFont& operator=(LKFont&& Font) = delete;
    LKFont& operator=(const LKFont& Font) = delete;

    void Create(LOGFONT* pLogFont) { Load(*pLogFont); }
    void Release() { 
#ifndef USE_GDI
        TextCache::Flush(); // for compat with resolution change
#endif
        Destroy(); 
    }

    operator bool() const { return IsDefined(); }
    
#ifdef WIN32        
public:
    explicit LKFont(HFONT Font) {  font = Font; }
    
    operator HFONT() const { return Native(); }
#else
    operator const LKFont*() const { return this; }
    
#endif
};

static_assert(sizeof(LKFont) == sizeof(Font), "not same size");

#endif	/* LKFONT_H */

