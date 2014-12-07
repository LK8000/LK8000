/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKBitmap.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 16 octobre 2014, 20:07
 */

#ifndef LKBITMAP_H
#define	LKBITMAP_H

#include "Bitmap.hpp"

#ifdef WIN32
#include <windows.h>
#endif
#ifdef __linux__
#include "tchar.h"
#include "types.h"

#define MAKEINTRESOURCE(i) (#i)

#endif

class LKBitmap : public Bitmap {
public:
	LKBitmap();
	LKBitmap(LKBitmap&& orig);
	LKBitmap(const LKBitmap& orig) = delete;

	virtual ~LKBitmap();

	LKBitmap& operator=(LKBitmap&& );
	LKBitmap& operator=(const LKBitmap& Bitmap) = delete;

	void Release() { Reset(); }

	bool LoadFromFile(const TCHAR* FilePath);
	bool LoadFromResource(const TCHAR* ResourceName);

    operator bool() const { return IsDefined(); }

#ifdef WIN32
	explicit LKBitmap(HBITMAP Bitmap) { bitmap = Bitmap; }
	operator HBITMAP() const { return GetNative(); }
#endif
};

#endif	/* LKBITMAP_H */

