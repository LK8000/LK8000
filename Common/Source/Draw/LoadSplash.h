/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 * 
 * File:   LoadSplash.h
 * Author: Bruno de Lacheisserie
 *
 * Created on March 3, 2016, 11:44 PM
 */

#ifndef LOADSPLASH_H
#define LOADSPLASH_H

LKBitmap LoadSplash(const TCHAR *splashfile);
void DrawSplash(LKSurface& Surface, const RECT& rcDraw, const LKBitmap& Bmp);

#endif /* LOADSPLASH_H */

