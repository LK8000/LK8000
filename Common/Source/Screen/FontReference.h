/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   FontReference.h
 * Author: Bruno de Lacheisserie
 *
 * Created on 7 décembre 2014, 19:03
 */

#ifndef FONTREFERENCE_H
#define	FONTREFERENCE_H

#ifdef USE_GDI

class LKFont;
class FontReference;

#else

class LKFont;
typedef const LKFont *FontReference;

#endif

#endif	/* FONTREFERENCE_H */

