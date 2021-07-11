/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
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
#include "FontReference.h"
#endif

#ifdef WIN32
class LKFont;
class FontReference {
public:
	FontReference() : _font_ref()  { }
	FontReference(const LKFont* ref) : _font_ref(ref)  { }
	inline operator HFONT();
private:
	const LKFont* _font_ref;
};

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
#endif
    operator FontReference() const { return this; }
};

static_assert(sizeof(LKFont) == sizeof(Font), "not same size");

#ifdef WIN32

inline FontReference::operator HFONT() {
	return _font_ref?_font_ref->Native():NULL;
}

#endif

#endif	/* LKFONT_H */
