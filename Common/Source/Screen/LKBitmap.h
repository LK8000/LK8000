/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LKBitmap.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 16 octobre 2014, 20:07
 */

#ifndef LKBITMAP_H
#define	LKBITMAP_H

#include "Screen/Bitmap.hpp"

class LKBitmap final : public Bitmap {
public:
	LKBitmap();
	LKBitmap(LKBitmap&& orig) = default;
	LKBitmap(const LKBitmap& orig) = delete;

	~LKBitmap();

	LKBitmap& operator=(LKBitmap&& ) = default;
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
