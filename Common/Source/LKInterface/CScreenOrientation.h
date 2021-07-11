/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   CScreenOrientation.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 13 février 2014, 23:03
 */

#ifndef CSCREENORIENTATION_H
#define	CSCREENORIENTATION_H
#include <tchar.h>
#include "Util/tstring.hpp"

class CScreenOrientation {
public:
    CScreenOrientation(const TCHAR* szPath);
    virtual ~CScreenOrientation();

    CScreenOrientation() = delete;
    CScreenOrientation(CScreenOrientation&) = delete;
    CScreenOrientation(CScreenOrientation&&) = delete;

    static unsigned short GetScreenSetting();

private:
    static unsigned short GetSavedSetting(const TCHAR* szFileName);

    static bool Save(const TCHAR* szFileName);
    static bool Restore(const TCHAR* szFileName);

    static bool SetScreenSetting(unsigned short);

    tstring mLKFilePath; // file used for save screen orientation of LK.
    tstring mOSFilePath; // file used for save screen orientation at startup and restore it at shutdown.

    static const unsigned short invalid = (~0);
};

#endif	/* CSCREENORIENTATION_H */
