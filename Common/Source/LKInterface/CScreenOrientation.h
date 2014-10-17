/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   CScreenOrientation.cpp
 * Author: Bruno de Lacheisserie
 *
 * Created on 13 février 2014, 23:03
 */

#ifndef CSCREENORIENTATION_H
#define	CSCREENORIENTATION_H

class CScreenOrientation : private boost::noncopyable {
public:
    CScreenOrientation(const LPCTSTR szPath);
    virtual ~CScreenOrientation();

private:
    static unsigned short GetSavedSetting(LPCTSTR szFileName);

    static bool Save(LPCTSTR szFileName);
    static bool Restore(LPCTSTR szFileName);
    
    static unsigned short GetScreenSetting();
    static bool SetScreenSetting(unsigned short);

    std::wstring mLKFilePath; // file used for save screen orientation of LK.
    std::wstring mOSFilePath; // file used for save screen orientation at startup and restore it at shutdown.
    
    static const unsigned short invalid = (~0);
};

#endif	/* CSCREENORIENTATION_H */

