/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgStartup.cpp,v 1.1 2011/12/21 10:29:29 root Exp $
 */

#include "externs.h"
#include "WindowControls.h"
#include "RGB.h"
#include "resource.h"
#include "LKObjects.h"
#include "LKProfiles.h"
#include "Dialogs.h"
#include "dlgTools.h"
#include "OS/Memory.h"
#include "Sound/Sound.h"
#include "ScreenGeometry.h"

extern void Shutdown(void);
extern void LoadSplash(LKSurface& Surface, const TCHAR *splashfile);

static WndForm *wf = NULL;

extern bool CheckSystemDefaultMenu(void);
extern bool CheckLanguageEngMsg(void);
extern bool CheckSystemBitmaps(void);
void RawWrite(LKSurface& Surface, const TCHAR *text, int line, short fsize, const LKColor& rgbcolor, int wtmode);

// This global is set true on startup only here, and it is cleared by the LoadNewTask
// LoadNewTask is called by event manager at start of normal run
bool FullResetAsked = false;

static bool OnTimerNotify() {
    // Do we have a premature shutdown request from system quit?
    if (RUN_MODE == RUN_SHUTDOWN) {
#if TESTBENCH
        StartupStore(_T("... dlGStartup shutdown requested\n"));
#endif
        wf->SetModalResult(mrOK);
    }
    return true;
}



// Syntax  hdc _Text linenumber fontsize 
// lines are: 0 - 9
// fsize 0 small 1 normal 2 big

void RawWrite(LKSurface& Surface, const TCHAR *text, int line, short fsize, const LKColor& rgbcolor, int wtmode) {
    const auto oldfont = Surface.SelectObject(MapWindowFont);
    switch (fsize) {
        case 0:
            Surface.SelectObject(TitleWindowFont);
            break;
        case 1:
            Surface.SelectObject(LK8MapFont);
            break;
        case 2:
            Surface.SelectObject(LK8MediumFont);
            break;
        case 3:
            Surface.SelectObject(LK8BigFont);
            break;
    }
    Surface.SetBackgroundTransparent();
    SIZE tsize;
    Surface.GetTextSize(text, _tcslen(text), &tsize);
    const int y = tsize.cy * (line - 1) + (tsize.cy / 2);

    MapWindow::LKWriteText(Surface, text, ScreenSizeX / 2, y, 0, wtmode, WTALIGN_CENTER, rgbcolor, false);
    Surface.SelectObject(oldfont);
}

static void OnSplashPaint(WindowControl * Sender, LKSurface& Surface) {

    TCHAR srcfile[MAX_PATH];

    if (RUN_MODE == RUN_SHUTDOWN) return;

    LoadSplash(Surface, (RUN_MODE == RUN_WELCOME) ? _T("LKSTART") : _T("LKPROFILE"));

    if (RUN_MODE == RUN_WELCOME) {
        TCHAR mes[100];
        int pos = 0;
        switch (ScreenSize) {
            case ss480x272:
                if (ScreenSizeX == 854)
                    pos = 14;
                else
#ifdef __linux__
                    pos = 12;
#else
                    pos = 11;
#endif
                break;
                // --------- portrait -------------
            case ss240x320:
#ifdef __linux__
                pos = 19;
#else
                pos = 17;
#endif
                break;
            default:
                // customized definition
                if (ScreenLandscape) {
                    switch (ScreenGeometry) {
                        case SCREEN_GEOMETRY_43:
                            pos = 12;
                            break;
                        case SCREEN_GEOMETRY_53:
                            pos = 12;
                            break;
                        case SCREEN_GEOMETRY_169:
                            pos = 11;
                            break;
                        default:
                            pos = 11;
                            break;
                    }
                } else {
                    // Portrait
                    // try to get a rule for text position...
                    switch (ScreenGeometry) {
                        case SCREEN_GEOMETRY_43:
#ifdef __linux__
                            pos = 18;
#else
                            pos = 17;
#endif
                            break;
                        case SCREEN_GEOMETRY_53:
                            pos = 20;
                            break;
                        case SCREEN_GEOMETRY_169:
#ifdef __linux__
                            pos = 22;
#else
                            pos = 20;
#endif
                            break;
                        default:
                            pos = 21;
                            break;
                    }

                }
                break;
        }
        if (FullResetAsked) {
            _stprintf(mes, _T("*** %s ***"), gettext(_T("_@M1757_")));
            RawWrite(Surface, mes, pos, 1, RGB_DARKWHITE, WTMODE_NORMAL);
        } else {
#ifndef LKCOMPETITION
            _stprintf(mes, _T("Version %s.%s (%s)"), _T(LKVERSION), _T(LKRELEASE), _T(__DATE__));
#else
            _stprintf(mes, _T("V%s.%s (%s) COMPETITION"), _T(LKVERSION), _T(LKRELEASE), _T(__DATE__));
#endif
            RawWrite(Surface, mes, pos, 1, RGB_DARKWHITE, WTMODE_NORMAL);
        }
    }

    if (RUN_MODE != RUN_WELCOME) {

        // FillRect(hDC,&ScreenSizeR, LKBrush_Black); // REMOVE 

        TCHAR mes[100];
#ifndef LKCOMPETITION
        _stprintf(mes, _T("%s v%s.%s - %s"), _T(LKFORK), _T(LKVERSION), _T(LKRELEASE), gettext(_T("_@M2054_")));
#else
        _stprintf(mes, _T("%sC v%s.%s - %s"), _T(LKFORK), _T(LKVERSION), _T(LKRELEASE), gettext(_T("_@M2054_")));
#endif
        RawWrite(Surface, mes, 1, 1, RGB_LIGHTGREY, WTMODE_NORMAL);

        size_t freeram = CheckFreeRam() / 1024;
        TCHAR buffer[MAX_PATH];
        LocalPath(buffer);
        size_t freestorage = FindFreeSpace(buffer);
        _stprintf(mes, _T("free ram %.1uM  storage %.1uM"), (unsigned int) freeram / 1024, (unsigned int) freestorage / 1024);
        RawWrite(Surface, mes, 3, 0, RGB_LIGHTGREY, WTMODE_NORMAL);

        if (ScreenSize != ss320x240 && ScreenLandscape)
            RawWrite(Surface, _T("_______________________"), 2, 2, RGB_LIGHTGREY, WTMODE_NORMAL);

        if (FullResetAsked) {
            _stprintf(mes, _T("%s"), gettext(_T("_@M1757_"))); // LK8000 PROFILES RESET
            RawWrite(Surface, mes, 5, 2, RGB_ICEWHITE, WTMODE_OUTLINED);
            _stprintf(mes, _T("%s"), gettext(_T("_@M1759_"))); // SELECTED IN SYSTEM
            RawWrite(Surface, mes, 6, 2, RGB_ICEWHITE, WTMODE_OUTLINED);
        } else {
            _stprintf(mes, _T("%s"), PilotName_Config);
            RawWrite(Surface, mes, 4, 2, RGB_ICEWHITE, WTMODE_OUTLINED);

            _stprintf(mes, _T("%s"), AircraftRego_Config);
            RawWrite(Surface, mes, 5, 2, RGB_AMBER, WTMODE_OUTLINED);

            _stprintf(mes, _T("%s"), AircraftType_Config);
            RawWrite(Surface, mes, 6, 2, RGB_AMBER, WTMODE_OUTLINED);

            LKASSERT(szPolarFile[0]);
            extern void LK_tsplitpath(const TCHAR* path, TCHAR* drv, TCHAR* dir, TCHAR* name, TCHAR * ext);
            LK_tsplitpath(szPolarFile, (TCHAR*) NULL, (TCHAR*) NULL, srcfile, (TCHAR*) NULL);

            _stprintf(mes, _T("%s %s"), gettext(_T("_@M528_")), srcfile); // polar file
            RawWrite(Surface, mes, 7, 2, RGB_AMBER, WTMODE_OUTLINED);

            LKASSERT(startProfileFile[0]);
            LK_tsplitpath(startProfileFile, (TCHAR*) NULL, (TCHAR*) NULL, srcfile, (TCHAR*) NULL);
            _stprintf(mes, _T("%s: %s"), MsgToken(1746), srcfile);
            RawWrite(Surface, mes, 11, 1, RGB_ICEWHITE, WTMODE_NORMAL);
        }


        // RawWrite(hDC,_T("_______________________"),8,2, RGB_LIGHTGREY,WTMODE_NORMAL); // REMOVE FOR THE 3.0

        return;
    }

}

static void OnCloseClicked(WndButton* pWnd) {

    LKSound(_T("LK_SLIDE.WAV"));
    switch (RUN_MODE) {
        case RUN_DUALPROF:
            RUN_MODE = RUN_WELCOME;
            break;
    }
    wf->SetModalResult(mrOK);
}

static void OnSIMClicked(WndButton* pWnd) {
    RUN_MODE = RUN_SIM;
    wf->SetModalResult(mrOK);
}

static void OnFLYClicked(WndButton* pWnd) {
    RUN_MODE = RUN_FLY;
    //  Removed 110605: we now run devInit on startup for all devices, and we dont want an immediate and useless reset.
    //  LKForceComPortReset=true;
    PortMonitorMessages = 0;
    wf->SetModalResult(mrOK);
}

static void OnDUALPROFILEClicked(WndButton* pWnd) {
    RUN_MODE = RUN_DUALPROF;
    LKSound(_T("LK_SLIDE.WAV"));
    wf->SetModalResult(mrOK);
}

static void OnEXITClicked(WndButton* pWnd) {
    RUN_MODE = RUN_EXIT;
    wf->SetModalResult(mrOK);
}

static void OnPROFILEClicked(WndButton* pWnd) {
    RUN_MODE = RUN_PROFILE;
    LKSound(_T("LK_SLIDE.WAV"));
    wf->SetModalResult(mrOK);
}

static void OnAIRCRAFTClicked(WndButton* pWnd) {
    RUN_MODE = RUN_AIRCRAFT;
    LKSound(_T("LK_SLIDE.WAV"));
    wf->SetModalResult(mrOK);
}

static void OnDEVICEClicked(WndButton* pWnd) {
    RUN_MODE = RUN_DEVICE;
    LKSound(_T("LK_SLIDE.WAV"));
    wf->SetModalResult(mrOK);
}

static void OnPILOTClicked(WndButton* pWnd) {
    RUN_MODE = RUN_PILOT;
    LKSound(_T("LK_SLIDE.WAV"));
    wf->SetModalResult(mrOK);
}

static CallBackTableEntry_t CallBackTable[] = {
    OnPaintCallbackEntry(OnSplashPaint),
    ClickNotifyCallbackEntry(OnPILOTClicked),
    ClickNotifyCallbackEntry(OnDEVICEClicked),
    ClickNotifyCallbackEntry(OnAIRCRAFTClicked),
    ClickNotifyCallbackEntry(OnPROFILEClicked),
    ClickNotifyCallbackEntry(OnEXITClicked),
    ClickNotifyCallbackEntry(OnDUALPROFILEClicked),
    ClickNotifyCallbackEntry(OnFLYClicked),
    ClickNotifyCallbackEntry(OnSIMClicked),
    ClickNotifyCallbackEntry(OnCloseClicked),
    EndCallBackEntry()
};


static WndForm* InitFlySim() {
    
    WndForm* pWndForm = dlgLoadFromXML(CallBackTable,
                ScreenLandscape ? TEXT("dlgFlySim_L.xml") : TEXT("dlgFlySim_P.xml"),
                ScreenLandscape ? IDR_XML_FLYSIM_L : IDR_XML_FLYSIM_P);
    if(pWndForm) {
    
        WindowControl * pWnd = nullptr;

        if (ScreenLandscape) {
            const unsigned int SPACEBORDER = NIBLSCALE(2);
            const unsigned int w = (ScreenSizeX - (SPACEBORDER * 5)) / 4;
            unsigned int lx = SPACEBORDER - 1; // count from 0

            pWnd = pWndForm->FindByName(TEXT("cmdFLY"));
            if(pWnd) {
                pWnd->SetWidth(w);
                pWnd->SetLeft(lx);
            }

            lx += w + SPACEBORDER;
            pWnd = pWndForm->FindByName(TEXT("cmdDUALPROFILE"));
            if(pWnd) {
                pWnd->SetWidth(w);
                pWnd->SetLeft(lx);
            }

            lx += w + SPACEBORDER;
            pWnd = pWndForm->FindByName(TEXT("cmdEXIT"));
            if(pWnd) {
                pWnd->SetWidth(w);
                pWnd->SetLeft(lx);
            }

            lx += w + SPACEBORDER;
            pWnd = pWndForm->FindByName(TEXT("cmdSIM"));
            if(pWnd) {
                pWnd->SetWidth(w);
                pWnd->SetLeft(lx);
            }

        } else {
            const int h = ScreenSizeY - IBLSCALE(90); // 40+5+40+5
            pWnd = pWndForm->FindByName(TEXT("cmdFLY"));
            if(pWnd) {
                pWnd->SetTop(h + IBLSCALE(45));
                pWnd->SetHeight(IBLSCALE(40));
            }

            pWnd = pWndForm->FindByName(TEXT("cmdDUALPROFILE"));
            if(pWnd) {
                pWnd->SetTop(h);
                pWnd->SetHeight(IBLSCALE(40));
            }

            pWnd = pWndForm->FindByName(TEXT("cmdEXIT"));
            if(pWnd) {
                pWnd->SetTop(h);
                pWnd->SetHeight(IBLSCALE(40));
            }

            pWnd = pWndForm->FindByName(TEXT("cmdSIM"));
            if(pWnd) {
                pWnd->SetTop(h + IBLSCALE(45));
                pWnd->SetHeight(IBLSCALE(40));
            }            
        }
    }    
    return pWndForm;
}

static WndForm* InitDualProfile() {

    WndForm* pWndForm = dlgLoadFromXML(CallBackTable,
                ScreenLandscape ? TEXT("dlgDualProfile_L.xml") : TEXT("dlgDualProfile_P.xml"),
                ScreenLandscape ? IDR_XML_DUALPROFILE_L : IDR_XML_DUALPROFILE_P);

    if(pWndForm) {
        WindowControl * pWnd = nullptr;

        if (ScreenLandscape) {

            const unsigned int SPACEBORDER = NIBLSCALE(2);

            const unsigned w =  (ScreenSizeX - (SPACEBORDER * 6)) / 5;

            int lx = SPACEBORDER - 1; // count from 0
            pWnd = pWndForm->FindByName(TEXT("cmdAIRCRAFT"));
            if(pWnd) {
                pWnd->SetLeft(lx);
                pWnd->SetWidth(w);
            }

            lx += w + SPACEBORDER;
            pWnd = pWndForm->FindByName(TEXT("cmdPROFILE"));
            if(pWnd) {
                pWnd->SetLeft(lx);
                pWnd->SetWidth(w);
            }

            lx += w + SPACEBORDER;
            pWnd = pWndForm->FindByName(TEXT("cmdDEVICE"));
            if(pWnd) {
                pWnd->SetLeft(lx);
                pWnd->SetWidth(w);
            }

            lx += w + SPACEBORDER;
            pWnd = pWndForm->FindByName(TEXT("cmdPILOT"));
            if(pWnd) {
                pWnd->SetLeft(lx);
                pWnd->SetWidth(w);
            }

            lx += w + SPACEBORDER;
            pWnd = pWndForm->FindByName(TEXT("cmdCLOSE"));
            if(pWnd) {
                pWnd->SetLeft(lx);
                pWnd->SetWidth(w);
            }

        } else {
            const unsigned SPACEBORDER = NIBLSCALE(2);
            unsigned w = (ScreenSizeX - (SPACEBORDER * 3)) / 2;
            int h = ScreenSizeY - IBLSCALE(90); // 40+5+40+5

            int lx = SPACEBORDER - 1; // count from 0
            pWnd = pWndForm->FindByName(TEXT("cmdAIRCRAFT"));
            if(pWnd) {
                pWnd->SetLeft(lx);
                pWnd->SetWidth(w);
                pWnd->SetTop(h);
            }

            lx += w + SPACEBORDER;
            pWnd = pWndForm->FindByName(TEXT("cmdPROFILE"));
            if(pWnd) {
                pWnd->SetLeft(lx);
                pWnd->SetWidth(w);
                pWnd->SetTop(h);
            }

            w = (ScreenSizeX - (SPACEBORDER * 4)) / 3;
            lx = SPACEBORDER - 1; // count from 0
            pWnd = pWndForm->FindByName(TEXT("cmdDEVICE"));
            if(pWnd) {
                pWnd->SetLeft(lx);
                pWnd->SetWidth(w);
                pWnd->SetTop(h + IBLSCALE(45));
            }

            lx += w + SPACEBORDER;
            pWnd = pWndForm->FindByName(TEXT("cmdPILOT"));
            if(pWnd) {
                pWnd->SetLeft(lx);
                pWnd->SetWidth(w);
                pWnd->SetTop(h + IBLSCALE(45));
            }

            lx += w + SPACEBORDER;
            pWnd = pWndForm->FindByName(TEXT("cmdCLOSE"));
            if(pWnd) {
                pWnd->SetLeft(lx);
                pWnd->SetWidth(w);
                pWnd->SetTop(h + IBLSCALE(45));
            }
        }
    }
    return pWndForm;
}

static WndForm* InitStartup(BYTE mode) {
    WndForm * pWndForm = dlgLoadFromXML(CallBackTable,
                ScreenLandscape ? TEXT("dlgStartup_L.xml") : TEXT("dlgStartup_P.xml"),
                ScreenLandscape ? IDR_XML_STARTUP_L : IDR_XML_STARTUP_P);
    if(pWndForm) {
        
        WindowControl * pWndClose = pWndForm->FindByName(TEXT("cmdClose"));
        WndProperty* pWndProfile = static_cast<WndProperty*>(pWndForm->FindByName(TEXT("prpProfile")));
        
        if (ScreenLandscape) {
            const int PROFWIDTH = IBLSCALE(256);
            const int PROFACCEPTWIDTH = NIBLSCALE(60);
            const int PROFHEIGHT = NIBLSCALE(30);
            const int PROFSEPARATOR = NIBLSCALE(4);

            if(pWndClose) {
                pWndClose->SetWidth(PROFACCEPTWIDTH);
                pWndClose->SetLeft((((ScreenSizeX - PROFWIDTH - PROFSEPARATOR - PROFACCEPTWIDTH) / 2) + PROFSEPARATOR + PROFWIDTH) - NIBLSCALE(2));
                pWndClose->SetHeight(PROFHEIGHT - NIBLSCALE(4));
            }
            
            if(pWndProfile) {
                pWndProfile->SetLeft(((ScreenSizeX - PROFWIDTH - PROFSEPARATOR - PROFACCEPTWIDTH) / 2) - NIBLSCALE(2));
                pWndProfile->SetHeight(PROFHEIGHT);
                pWndProfile->SetWidth(PROFWIDTH);
            }

            
        } else {
            const int PROFWIDTH = IBLSCALE(236);
            const int PROFHEIGHT = NIBLSCALE(25);
            int h = ScreenSizeY - IBLSCALE(65); // 

            if(pWndClose) {
                pWndClose->SetWidth(ScreenSizeX - NIBLSCALE(6));
                pWndClose->SetLeft(NIBLSCALE(2));
                pWndClose->SetTop(h);
            }
            if(pWndProfile) {
                pWndProfile->SetTop(h + IBLSCALE(35));
                pWndProfile->SetLeft(0);
                pWndProfile->SetHeight(PROFHEIGHT);
                pWndProfile->SetWidth(PROFWIDTH);
            }

        }
        //
        // File selection shared  by PROFILEs choices
        //
        if (pWndProfile) {
            DataFieldFileReader* dfe = static_cast<DataFieldFileReader*>(pWndProfile->GetDataField());
            if(dfe) {
                TCHAR temp[MAX_PATH];
                if (mode == RUN_PROFILE) {
                    _stprintf(temp, _T("*%s"), _T(LKS_PRF));
                    dfe->ScanDirectoryTop(_T(LKD_CONF), temp);
                    dfe->addFile(gettext(_T("_@M1741_")), _T("PROFILE_RESET"));
                    dfe->Lookup(startProfileFile);

                } else if (mode == RUN_AIRCRAFT) {
                    _stprintf(temp, _T("*%s"), _T(LKS_AIRCRAFT));
                    dfe->ScanDirectoryTop(_T(LKD_CONF), temp);
                    dfe->Lookup(startAircraftFile);

                } else if (mode == RUN_DEVICE) {
                    _stprintf(temp, _T("*%s"), _T(LKS_DEVICE));
                    dfe->ScanDirectoryTop(_T(LKD_CONF), temp);
                    dfe->Lookup(startDeviceFile);

                } else if (mode == RUN_PILOT) {
                    _stprintf(temp, _T("*%s"), _T(LKS_PILOT));
                    dfe->ScanDirectoryTop(_T(LKD_CONF), temp);
                    dfe->Lookup(startPilotFile);
                }
            }
            pWndProfile->RefreshDisplay();
        }        
        
    }
    return pWndForm;
}

// This function will be recalled as long it returns 1.
// Returning 0 will indicate proceed with startup
// Returning -1 will indicate exit program

short dlgStartupShowModal(void) {
    WndProperty* wp = nullptr;

#if TESTBENCH
    StartupStore(TEXT(". Startup dialog, RUN_MODE=%d %s"), RUN_MODE, NEWLINE);
#endif

    switch(RUN_MODE) {
        case RUN_WELCOME: // FLY SIM PROFILE EXIT
            wf = InitFlySim();
            break;
        case RUN_DUALPROF: //  PROFILE AIRCRAFT  CLOSE
            wf = InitDualProfile();
            break;
        case RUN_PROFILE:
        case RUN_AIRCRAFT:
        case RUN_PILOT:
        case RUN_DEVICE:
            // CHOOSE PROFILE
            wf = InitStartup(RUN_MODE);
            break;
    }
    if (!wf) {
        return 0;
    }

    wf->SetHeight(ScreenSizeY);
    wf->SetWidth(ScreenSizeX);
    
    WindowControl* wSplash = wf->FindByName(TEXT("frmSplash"));
    if(wSplash) {
        wSplash->SetWidth(ScreenSizeX);
        // wSplash->SetHeight(ScreenSizeY);// - IBLSCALE(55));
    }




    if (!CheckRootDir()) {
        TCHAR mydir[MAX_PATH];
        TCHAR mes[MAX_PATH];

        LocalPath(mydir, _T(""));
        _stprintf(mes, _T("%s"), mydir);
        MessageBoxX(_T("NO LK8000 DIRECTORY\nCheck Installation!"), _T("FATAL ERROR 000"), mbOk);
        MessageBoxX(mes, _T("NO LK8000 DIRECTORY"), mbOk, true);
        RUN_MODE = RUN_EXIT;
        Shutdown();
        goto _exit;
    }

    if (!CheckDataDir()) {
        TCHAR mydir[MAX_PATH];
        TCHAR mes[MAX_PATH];

        SystemPath(mydir, _T(LKD_SYSTEM));
        _stprintf(mes, _T("%s"), mydir);
        MessageBoxX(_T("NO SYSTEM DIRECTORY\nCheck Installation!"), _T("FATAL ERROR 001"), mbOk);
        MessageBoxX(mes, _T("NO SYSTEM DIRECTORY"), mbOk, true);
        RUN_MODE = RUN_EXIT;
        Shutdown();
        goto _exit;
    }

    if (!CheckLanguageDir()) {
        TCHAR mydir[MAX_PATH];
        TCHAR mes[MAX_PATH];
        StartupStore(_T("... CHECK LANGUAGE DIRECTORY FAILED!%s"), NEWLINE);

        LocalPath(mydir, _T(LKD_LANGUAGE));
        _stprintf(mes, _T("%s"), mydir);
        MessageBoxX(_T("LANGUAGE DIRECTORY CHECK FAIL\nCheck Language Install"), _T("FATAL ERROR 002"), mbOk);
        MessageBoxX(mes, _T("NO LANGUAGE DIRECTORY"), mbOk, true);
        RUN_MODE = RUN_EXIT;
        Shutdown();
        goto _exit;
    }
    if (!CheckLanguageEngMsg()) {
        TCHAR mydir[MAX_PATH];
        TCHAR mes[MAX_PATH];
        StartupStore(_T("... CHECK LANGUAGE ENG_MSG FAILED!%s"), NEWLINE);
        LocalPath(mydir, _T(LKD_LANGUAGE));
        _stprintf(mes, _T("%s/ENG_MSG.TXT"), mydir);
        MessageBoxX(_T("ENG_MSG.TXT MISSING in LANGUAGE\nCheck Language Install"), _T("FATAL ERROR 012"), mbOk);
        MessageBoxX(mes, _T("MISSING FILE!"), mbOk, true);
        RUN_MODE = RUN_EXIT;
        Shutdown();
        goto _exit;
    }
    if (!CheckSystemDefaultMenu()) {
        TCHAR mydir[MAX_PATH];
        TCHAR mes[MAX_PATH];
        StartupStore(_T("... CHECK SYSTEM DEFAULT_MENU.TXT FAILED!%s"), NEWLINE);
        SystemPath(mydir, _T(LKD_SYSTEM));
        _stprintf(mes, _T("%s/DEFAULT_MENU.TXT"), mydir);
        MessageBoxX(_T("DEFAULT_MENU.TXT MISSING in SYSTEM\nCheck System Install"), _T("FATAL ERROR 022"), mbOk);
        MessageBoxX(mes, _T("MISSING FILE!"), mbOk, true);
        RUN_MODE = RUN_EXIT;
        Shutdown();
        goto _exit;
    }

    if (!CheckSystemBitmaps()) {
        TCHAR mydir[MAX_PATH];
        TCHAR mes[MAX_PATH];
        StartupStore(_T("... CHECK SYSTEM _BITMAPSH FAILED!%s"), NEWLINE);
        SystemPath(mydir, _T(LKD_BITMAPS));
        _stprintf(mes, _T("%s/_BITMAPSH"), mydir);
        MessageBoxX(_T("_BITMAPSH MISSING in SYSTEM Bitmaps\nCheck System Install"), _T("FATAL ERROR 032"), mbOk);
        MessageBoxX(mes, _T("MISSING FILE!"), mbOk, true);
        RUN_MODE = RUN_EXIT;
        Shutdown();
        goto _exit;
    }

    extern unsigned short Bitmaps_Errors;
    if (Bitmaps_Errors) {
        TCHAR mes[MAX_PATH];
        _stprintf(mes, _T("MISSING %d SYSTEM BITMAPS! CHECK INSTALLATION."), Bitmaps_Errors);
        MessageBoxX(mes, _T("MISSING FILES!"), mbOk, true);
    }

    if (!CheckPolarsDir()) {
        TCHAR mydir[MAX_PATH];
        TCHAR mes[MAX_PATH];
        StartupStore(_T("... CHECK POLARS DIRECTORY FAILED!%s"), NEWLINE);

        LocalPath(mydir, _T(LKD_POLARS));
        _stprintf(mes, _T("%s"), mydir);
        MessageBoxX(_T("NO POLARS DIRECTORY\nCheck Install"), _T("FATAL ERROR 003"), mbOk);
        MessageBoxX(mes, _T("NO POLARS DIRECTORY"), mbOk, true);
        RUN_MODE = RUN_EXIT;
        Shutdown();
        goto _exit;
    }

    extern bool CheckFilesystemWritable(void);
    if (!CheckFilesystemWritable()) {
        MessageBoxX(_T("LK8000 CANNOT WRITE IN MEMORY CARD!\nCARD IS LOCKED, OR DAMAGED, OR FULL."), _T("CRITICAL PROBLEM"), mbOk);
        RUN_MODE = RUN_EXIT;
        Shutdown();
        goto _exit;
    }



    // Standby for a system request to close the application during this phase.
    wf->SetTimerNotify(500, OnTimerNotify);
    if (wf->ShowModal() == mrCancel) {
        RUN_MODE = RUN_EXIT;
    }
    if (RUN_MODE == RUN_SHUTDOWN) goto _exit;

    wp = (WndProperty*) wf->FindByName(TEXT("prpProfile"));
    if (wp) {
        DataFieldFileReader* dfe = (DataFieldFileReader*) wp->GetDataField();

        if (RUN_MODE == RUN_PROFILE) {
            if (_tcslen(dfe->GetPathFile()) > 0) {
                if (_tcscmp(dfe->GetPathFile(), startProfileFile)) { // if they are not the same
                    _tcscpy(startProfileFile, dfe->GetPathFile());
                    if (_tcscmp(startProfileFile, _T("PROFILE_RESET")) == 0) {
#if TESTBENCH
                        StartupStore(_T("... Selected FULL RESET virtual profile\n"));
#endif
                        if (MessageBoxX(gettext(TEXT("_@M1758_")),
                                gettext(TEXT("_@M1757_")), mbOk));
                        FullResetAsked = true;
                    } else {
#if TESTBENCH
                        StartupStore(_T("... Selected new profile, preloading..\n"));
#endif
                        LKProfileLoad(startProfileFile);
                        extern void InitLKFonts();
                        InitLKFonts();
                        FullResetAsked = false;
                    }
                }
            }
        }
        if (RUN_MODE == RUN_AIRCRAFT) {
            if (_tcslen(dfe->GetPathFile()) > 0) {
                if (_tcscmp(dfe->GetPathFile(), startAircraftFile)) { // if they are not the same
                    _tcscpy(startAircraftFile, dfe->GetPathFile());
#if TESTBENCH
                    StartupStore(_T("... Selected new aircraft, preloading..\n"));
#endif
                    LKProfileLoad(startAircraftFile);
                }
            }
        }
        if (RUN_MODE == RUN_DEVICE) {
            if (_tcslen(dfe->GetPathFile()) > 0) {
                if (_tcscmp(dfe->GetPathFile(), startDeviceFile)) { // if they are not the same
                    _tcscpy(startDeviceFile, dfe->GetPathFile());
#if TESTBENCH
                    StartupStore(_T("... Selected new device, preloading..\n"));
#endif
                    LKProfileLoad(startDeviceFile);
                }
            }
        }
        if (RUN_MODE == RUN_PILOT) {
            if (_tcslen(dfe->GetPathFile()) > 0) {
                if (_tcscmp(dfe->GetPathFile(), startPilotFile)) { // if they are not the same
                    _tcscpy(startPilotFile, dfe->GetPathFile());
#if TESTBENCH
                    StartupStore(_T("... Selected new pilot, preloading..\n"));
#endif
                    LKProfileLoad(startPilotFile);
                }
            }
        }
        RUN_MODE = RUN_DUALPROF;
    }
    if (RUN_MODE == RUN_EXIT) {
#if __linux__
        RUN_MODE = RUN_WELCOME;
#endif
        LKSound(_T("LK_SLIDE.WAV"));
        if (MessageBoxX(
                // LKTOKEN  _@M198_ = "Confirm Exit?" 
                gettext(TEXT("_@M198_")),
                TEXT("LK8000"), mbYesNo) == IdYes) {
            Shutdown();
        } else
            RUN_MODE = RUN_WELCOME;
    }


_exit:
    if (wf != NULL) {
        delete wf;
        wf = NULL;
    }

    if (RUN_MODE == RUN_FLY || RUN_MODE == RUN_SIM) {
        LKSound(_T("LK_SLIDE.WAV"));
        return 0; // do not repeat dialog
    }

    if (RUN_MODE == RUN_EXIT || RUN_MODE == RUN_SHUTDOWN)
        return -1; // terminate
    else
        return 1; // repeat dialog

}

