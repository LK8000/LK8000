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

#ifdef WIN32
#include <windows.h>
#endif
#ifdef __linux__
#include "linuxcompat/tchar.h"
#include "linuxcompat/types.h"
#endif

class LKBitmap {
public:
	LKBitmap();
	LKBitmap(LKBitmap&& orig);
	LKBitmap(const LKBitmap& orig);

	virtual ~LKBitmap();

	LKBitmap& operator=(LKBitmap&& );
	LKBitmap& operator=(const LKBitmap& Bitmap);

	void Release();

	bool LoadFromFile(const TCHAR* FilePath);
	bool LoadFromResource(const TCHAR* ResourceName);

	SIZE GetSize() const;

#ifdef WIN32
public:

	explicit LKBitmap(HBITMAP Bitmap) : _Bitmap(Bitmap), _Destroy(false) { }

	operator HBITMAP() const { return _Bitmap; }
	operator bool() const { return (_Bitmap != NULL); }

protected:
	HBITMAP _Bitmap;
	bool _Destroy;
#endif

};

#endif	/* LKBITMAP_H */

