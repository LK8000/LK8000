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
#include "externs.h"
#include "CScreenOrientation.h"
#include "Hardware/RotateDisplay.hpp"

#if !defined(UNDER_CE) && !defined(WIN32)
#include "DisplayOrientation.hpp"
#include "OS/FileUtil.hpp"
#endif

#ifdef OPENVARIO
#include <fstream>
#include <regex>
#include "OS/Process.hpp"
#endif

CScreenOrientation::CScreenOrientation(const LPCTSTR szPath) : mLKFilePath(szPath), mOSFilePath(szPath)  {

    if(!mOSFilePath.empty() && (*mOSFilePath.rbegin()) != L'\\') {
        mOSFilePath += _T(DIRSEP);
    }
    mOSFilePath += TEXT(LKD_CONF);
    mOSFilePath += TEXT(DIRSEP ".OSScreen");

    // if we have OS saved state, previous don't shutdown correctly
    // so don't save current state.
    if(GetSavedSetting(mOSFilePath.c_str()) == invalid) {
        if(!Save(mOSFilePath.c_str())) {
            // TODO : Log Error;
        }
    }


    if(!mLKFilePath.empty() && (*mLKFilePath.rbegin()) != L'\\') {
        mLKFilePath += _T(DIRSEP);
    }
    mLKFilePath += TEXT(LKD_CONF);
    mLKFilePath += TEXT(DIRSEP ".LKScreen");

    if(!Restore(mLKFilePath.c_str())) {
        // TODO : Log Error;
    }
}

CScreenOrientation::~CScreenOrientation() {
    if(!Save(mLKFilePath.c_str())) {
        // TODO : Log Error;
    }
    if(!Restore(mOSFilePath.c_str())) {
        // TODO : Log Error;
    }
    lk::filesystem::deleteFile(mOSFilePath.c_str());
}

unsigned short CScreenOrientation::GetSavedSetting(LPCTSTR szFileName) {
    unsigned short ScreenO = invalid;
    FILE *fp = _tfopen(szFileName, TEXT("rb"));
    if(fp) {
        int temp;
        if(fscanf(fp, "%d", &temp) == 1) {
            ScreenO = temp;
        }
        fclose(fp);
    }
    return ScreenO;
}

bool CScreenOrientation::Save(LPCTSTR szFileName) {
    unsigned short ScreenO = GetScreenSetting();
    if(ScreenO != invalid) {
        FILE *fp = _tfopen(szFileName, _T("wb"));
        if(fp) {
            fprintf(fp, "%d", ScreenO);
            fclose(fp);
            return true;
        }
    }
    return false;
}

bool CScreenOrientation::Restore(LPCTSTR szFileName) {
    unsigned short newO = GetSavedSetting(szFileName);
    if(newO != invalid) {
        return SetScreenSetting(newO);
    }
    return true;
}

unsigned short CScreenOrientation::GetScreenSetting() {
#ifdef UNDER_CE
    DEVMODE devMode;
    memset(&devMode, 0, sizeof(devMode));
    devMode.dmSize=sizeof(devMode);
    devMode.dmFields = DM_DISPLAYORIENTATION;
    if(DISP_CHANGE_SUCCESSFUL == ChangeDisplaySettingsEx(NULL, &devMode, NULL, CDS_TEST, NULL)) {
        return devMode.dmDisplayOrientation;
    }
#elif defined(KOBO)

    char szLine[100] = {};
    if(File::ReadString("/sys/class/graphics/fb0/rotate", szLine, sizeof(szLine))) {
        int tmp = strtol(szLine, nullptr, 10);
        switch(tmp) {
            case 0: return static_cast<short>(DisplayOrientation_t::LANDSCAPE);
            case 1: return static_cast<short>(DisplayOrientation_t::REVERSE_PORTRAIT);
            case 2: return static_cast<short>(DisplayOrientation_t::REVERSE_LANDSCAPE);
            case 3: return static_cast<short>(DisplayOrientation_t::PORTRAIT);
            default: return static_cast<short>(DisplayOrientation_t::DEFAULT);
        }
    }
#elif defined(OPENVARIO)

    Run("/bin/mount", "/dev/mmcblk0p1", "/boot");
    std::ifstream ifs("/boot/config.uEnv", std::ifstream::in);
    if(ifs.is_open()) {
        std::regex pair_regex("^(rotation)=([0-3])$");
        std::smatch pair_match;
        std::string line;
        while (std::getline (ifs, line)) {
            if (std::regex_match(line, pair_match, pair_regex)) {
                if (pair_match.size() == 3) {
                    return static_cast<short>(strtoul(pair_match[2].str().c_str(), nullptr, 10));
                }
            }
        }
        ifs.close();
    }
    Run("/bin/umount", "/boot"); 
#endif
    return invalid;
}

bool CScreenOrientation::SetScreenSetting(unsigned short NewO) {
#ifdef UNDER_CE
    DEVMODE devMode;
    memset(&devMode, 0, sizeof(devMode));
    devMode.dmSize=sizeof(devMode);
    devMode.dmFields = DM_DISPLAYORIENTATION;

    if(DISP_CHANGE_SUCCESSFUL == ChangeDisplaySettingsEx(NULL, &devMode, NULL, CDS_TEST, NULL)) {
        if(devMode.dmDisplayOrientation != NewO) {
            devMode.dmDisplayOrientation = NewO;
            return (DISP_CHANGE_SUCCESSFUL == ChangeDisplaySettingsEx(NULL,&devMode,NULL,CDS_RESET,NULL));
        }
        return true;
    }
#elif defined(KOBO)
    if(Display::Rotate(static_cast<DisplayOrientation_t>(NewO))) {
      event_queue->SetDisplayOrientation(static_cast<DisplayOrientation_t>(NewO));
      return true;
    }
#endif
    return false;
}
