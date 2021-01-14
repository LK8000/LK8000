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
#include "Asset.hpp"
#include "Draw/LoadSplash.h"
#include "InfoBoxLayout.h"
#include "LKInterface.h"

#ifdef KOBO
  #include "Kobo/Kernel.hpp"
#endif


#define RGBDARKWHITE (IsDithered() ? RGB_WHITENOREV : RGB_DARKWHITE)
#define RGBLIGHTGREY (IsDithered() ? RGB_WHITENOREV : RGB_LIGHTGREY)
#define RGBICEWHITE  (IsDithered() ? RGB_WHITENOREV : RGB_ICEWHITE)
#define RGBAMBER     (IsDithered() ? RGB_WHITENOREV : RGB_AMBER)

#ifdef KOBO
#warning "Temporary : remove when we have KoboMenu"
bool RestartToNickel = true; // default to true, mandatory for avoid to brick device in case of abnormal termination.
#endif

extern void BeforeShutdown(void);

static WndForm *wf = NULL;
static LKBitmap StartBitmap;
static LKBitmap ProfileBitmap;

extern bool CheckSystemDefaultMenu(void);
extern bool CheckLanguageEngMsg(void);
extern bool CheckInfoUpdated(void);

#ifndef ANDROID
extern bool CheckSystemBitmaps(void);
#endif

void RawWrite(LKSurface& Surface, const TCHAR *text, int line, short fsize, const LKColor& rgbcolor, int wtmode);

// This global is set true on startup only here, and it is cleared by the LoadNewTask
// LoadNewTask is called by event manager at start of normal run
bool FullResetAsked = false;

static bool OnTimerNotify(WndForm* pWnd) {
    // Do we have a premature shutdown request from system quit?
    if (RUN_MODE == RUN_SHUTDOWN) {
#if TESTBENCH
        StartupStore(_T("... dlGStartup shutdown requested\n"));
#endif
      if(pWnd) {
        pWnd->SetModalResult(mrOK);
      }
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
    Surface.GetTextSize(text, &tsize);
    const int y = tsize.cy * (line - 1) + (tsize.cy / 2);

    MapWindow::LKWriteText(Surface, text, ScreenSizeX / 2, y, wtmode, WTALIGN_CENTER, rgbcolor, false);
    Surface.SelectObject(oldfont);
}

static void OnSplashPaint(WindowControl * Sender, LKSurface& Surface) {

    TCHAR srcfile[MAX_PATH];

    if (RUN_MODE == RUN_SHUTDOWN) return;

    if(RUN_MODE == RUN_WELCOME) {
        if(!StartBitmap) {
            StartBitmap = LoadSplash(_T("LKSTART"));
        }
        if(StartBitmap) {
            DrawSplash(Surface, Sender->GetClientRect(), StartBitmap);
        }
    } else {
        if(!ProfileBitmap) {
            ProfileBitmap = LoadSplash(_T("LKPROFILE"));
        }
        if(ProfileBitmap) {
            DrawSplash(Surface, Sender->GetClientRect(), ProfileBitmap);
        }
    }

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
            _stprintf(mes, _T("*** %s ***"), MsgToken(1757));
            RawWrite(Surface, mes, pos, 1, RGBDARKWHITE, WTMODE_NORMAL);
        } else {
#ifndef LKCOMPETITION
            _stprintf(mes, _T("Version %s.%s (%s)"), _T(LKVERSION), _T(LKRELEASE), _T(__DATE__));
#else
            _stprintf(mes, _T("V%s.%s (%s) COMPETITION"), _T(LKVERSION), _T(LKRELEASE), _T(__DATE__));
#endif
            RawWrite(Surface, mes, pos, 1, RGBDARKWHITE, WTMODE_NORMAL);
#ifdef KOBO
            if(IsKoboOTGKernel()) {
                RawWrite(Surface, _T("- USB host kernel -"), pos+1, 1, RGBDARKWHITE, WTMODE_NORMAL);
            }
#endif
        }
    }

    if (RUN_MODE != RUN_WELCOME) {

        TCHAR mes[100];
#ifndef LKCOMPETITION
        _stprintf(mes, _T("%s v%s.%s - %s"), _T(LKFORK), _T(LKVERSION), _T(LKRELEASE), MsgToken(2054));
#else
        _stprintf(mes, _T("%sC v%s.%s - %s"), _T(LKFORK), _T(LKVERSION), _T(LKRELEASE), MsgToken(2054));
#endif
        RawWrite(Surface, mes, 1, 1, RGBLIGHTGREY, (IsDithered() ? WTMODE_OUTLINED : WTMODE_NORMAL));

        size_t freeram = CheckFreeRam() / 1024;
        TCHAR buffer[MAX_PATH];
        LocalPath(buffer);
        size_t freestorage = FindFreeSpace(buffer);
        _stprintf(mes, _T("free ram %.1uM  storage %.1uM"), (unsigned int) freeram / 1024, (unsigned int) freestorage / 1024);
        RawWrite(Surface, mes, 3, 0, RGBLIGHTGREY, IsDithered() ? WTMODE_OUTLINED : WTMODE_NORMAL);

        if (ScreenSize != ss320x240 && ScreenLandscape)
            RawWrite(Surface, _T("_______________________"), 2, 2, RGBLIGHTGREY, WTMODE_NORMAL);

        if (FullResetAsked) {
            _stprintf(mes, _T("%s"), MsgToken(1757)); // LK8000 PROFILES RESET
            RawWrite(Surface, mes, 5, 2, RGBICEWHITE, WTMODE_OUTLINED);
            _stprintf(mes, _T("%s"), MsgToken(1759)); // SELECTED IN SYSTEM
            RawWrite(Surface, mes, 6, 2, RGBICEWHITE, WTMODE_OUTLINED);
        } else {
            _stprintf(mes, _T("%s"), PilotName_Config);
            RawWrite(Surface, mes, 4, 2, RGBICEWHITE, WTMODE_OUTLINED);

            _stprintf(mes, _T("%s"), AircraftRego_Config);
            RawWrite(Surface, mes, 5, 2, RGBAMBER, WTMODE_OUTLINED);

            _stprintf(mes, _T("%s"), AircraftType_Config);
            RawWrite(Surface, mes, 6, 2, RGBAMBER, WTMODE_OUTLINED);

            LKASSERT(szPolarFile[0]);
            LK_tsplitpath(szPolarFile, NULL, NULL, srcfile, NULL);

            _stprintf(mes, _T("%s %s"), MsgToken(528), srcfile); // polar file
            RawWrite(Surface, mes, 7, 2, RGBAMBER, WTMODE_OUTLINED);

            LKASSERT(startProfileFile[0]);
            LK_tsplitpath(startProfileFile, NULL, NULL, srcfile, NULL);
            _stprintf(mes, _T("%s: %s"), MsgToken(1746), srcfile);
            RawWrite(Surface, mes, 11, 1, RGBICEWHITE, WTMODE_NORMAL);
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

  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnSIMClicked(WndButton* pWnd) {
  RUN_MODE = RUN_SIM;
  #ifdef TESTBENCH
  StartupStore(_T("... **** SIM MODE SELECTED ****") NEWLINE);
  #endif
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnFLYClicked(WndButton* pWnd) {
  RUN_MODE = RUN_FLY;
  #ifdef TESTBENCH
  StartupStore(_T("... **** FLY MODE SELECTED ****") NEWLINE);
  #endif
  //  Removed 110605: we now run devInit on startup for all devices, and we dont want an immediate and useless reset.
  //  LKForceComPortReset=true;
  PortMonitorMessages = 0;
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnDUALPROFILEClicked(WndButton* pWnd) {
  RUN_MODE = RUN_DUALPROF;
  LKSound(_T("LK_SLIDE.WAV"));
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnEXITClicked(WndButton* pWnd) {
  RUN_MODE = RUN_EXIT;
  #ifdef TESTBENCH
  StartupStore(_T("... EXIT MODE SELECTED") NEWLINE);
  #endif
#ifdef KOBO
  RestartToNickel = false;
#endif
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnPROFILEClicked(WndButton* pWnd) {
  RUN_MODE = RUN_PROFILE;
  LKSound(_T("LK_SLIDE.WAV"));
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnAIRCRAFTClicked(WndButton* pWnd) {
  RUN_MODE = RUN_AIRCRAFT;
  LKSound(_T("LK_SLIDE.WAV"));
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnDEVICEClicked(WndButton* pWnd) {
  RUN_MODE = RUN_DEVICE;
  LKSound(_T("LK_SLIDE.WAV"));
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

static void OnPILOTClicked(WndButton* pWnd) {
  RUN_MODE = RUN_PILOT;
  LKSound(_T("LK_SLIDE.WAV"));
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}

#ifdef KOBO
static void OnNickelClick(WndButton* pWnd) {
  RUN_MODE = RUN_EXIT;
  RestartToNickel = true;
  LKSound(_T("LK_SLIDE.WAV"));
  if(pWnd) {
    WndForm * pForm = pWnd->GetParentWndForm();
    if(pForm) {
      pForm->SetModalResult(mrOK);
    }
  }
}
#endif


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

    WndForm* pWndForm = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_FLYSIM_L : IDR_XML_FLYSIM_P);
    if(pWndForm) {

        WindowControl * pWnd = nullptr;

        if (ScreenLandscape) {

#ifdef KOBO
            const unsigned int SPACEBORDER = 1;
            const unsigned int w = (ScreenSizeX - (SPACEBORDER * 6)) / 5;
#else
            const unsigned int SPACEBORDER = NIBLSCALE(2);
            const unsigned int w = (ScreenSizeX - (SPACEBORDER * 5)) / 4;
#endif
            unsigned int lx = SPACEBORDER - 1; // count from 0

            pWnd = pWndForm->FindByName(TEXT("cmdFLY"));
            if(pWnd) {
                pWnd->SetWidth(w);
                pWnd->SetLeft(lx);
            }

#ifdef KOBO
            lx += w + SPACEBORDER;
            WndButton* pWndNickel = new WndButton(pWndForm, _T("cmdNICKEL"), _T("KOBO"), lx , IBLSCALE(205), w, IBLSCALE(30), &OnNickelClick );
            if(pWndNickel) {

            }
#endif
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
#ifdef KOBO
                pWnd->SetCaption(MsgToken(1901)); // POWER OFF
#endif
            }

            lx += w + SPACEBORDER;
            pWnd = pWndForm->FindByName(TEXT("cmdSIM"));
            if(pWnd) {
                pWnd->SetWidth(w);
                pWnd->SetLeft(lx);
            }
        } else {
            const unsigned SPACEBORDER = NIBLSCALE(2);
            unsigned w = (ScreenSizeX - (SPACEBORDER * 3)) / 2;
            int h = ScreenSizeY - IBLSCALE(90); // 40+5+40+5

            int lx = SPACEBORDER - 1; // count from 0
            pWnd = pWndForm->FindByName(TEXT("cmdFLY"));
            if(pWnd) {
                pWnd->SetTop(h + IBLSCALE(45));
                pWnd->SetLeft(lx);
                pWnd->SetHeight(IBLSCALE(40));
                pWnd->SetWidth(w);
            }

            lx += w + SPACEBORDER;
            pWnd = pWndForm->FindByName(TEXT("cmdSIM"));
            if(pWnd) {
                pWnd->SetTop(h + IBLSCALE(45));
                pWnd->SetLeft(lx);
                pWnd->SetHeight(IBLSCALE(40));
                pWnd->SetWidth(w);
            }


#ifdef KOBO
            lx = SPACEBORDER - 1; // count from 0

            WndButton* pWndNickel = new WndButton(pWndForm, _T("cmdNICKEL"), _T("KOBO"), lx , h, w, IBLSCALE(40), &OnNickelClick );
            if(pWndNickel) {
                w = (ScreenSizeX - (SPACEBORDER * 4)) / 3;
                pWndNickel->SetTop(h);
                pWndNickel->SetLeft(lx);
                pWndNickel->SetHeight(IBLSCALE(40));
                pWndNickel->SetWidth(w);
            }

            lx += w + SPACEBORDER;
#else
            lx = SPACEBORDER - 1; // count from 0
#endif
            pWnd = pWndForm->FindByName(TEXT("cmdDUALPROFILE"));
            if(pWnd) {
                pWnd->SetTop(h);
                pWnd->SetLeft(lx);
                pWnd->SetHeight(IBLSCALE(40));
                pWnd->SetWidth(w);
            }

            lx += w + SPACEBORDER;
            pWnd = pWndForm->FindByName(TEXT("cmdEXIT"));
            if(pWnd) {
                pWnd->SetTop(h);
                pWnd->SetLeft(lx);
                pWnd->SetHeight(IBLSCALE(40));
                pWnd->SetWidth(w);
#ifdef KOBO
                pWnd->SetCaption(MsgToken(1901)); // POWER OFF
#endif
            }
        }
    }
    return pWndForm;
}

static WndForm* InitDualProfile() {

    WndForm* pWndForm = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_DUALPROFILE_L : IDR_XML_DUALPROFILE_P);

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
    WndForm * pWndForm = dlgLoadFromXML(CallBackTable, ScreenLandscape ? IDR_XML_STARTUP_L : IDR_XML_STARTUP_P);
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
                if (mode == RUN_PROFILE) {
                    dfe->ScanDirectoryTop(_T(LKD_CONF), _T(LKS_PRF));
                    dfe->addFile(MsgToken(1741), _T("PROFILE_RESET"));
                    dfe->Lookup(startProfileFile);

                } else if (mode == RUN_AIRCRAFT) {
                    dfe->ScanDirectoryTop(_T(LKD_CONF), _T(LKS_AIRCRAFT));
                    dfe->Lookup(startAircraftFile);

                } else if (mode == RUN_DEVICE) {
                    dfe->ScanDirectoryTop(_T(LKD_CONF), _T(LKS_DEVICE));
                    dfe->Lookup(startDeviceFile);

                } else if (mode == RUN_PILOT) {
                    dfe->ScanDirectoryTop(_T(LKD_CONF), _T(LKS_PILOT));
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
        wSplash->SetWidth(wf->GetWidth());
        wSplash->SetHeight(wf->GetHeight());
    }




    if (!CheckRootDir()) {
        TCHAR mydir[MAX_PATH];

        LocalPath(mydir);
        MessageBoxX(_T("NO LK8000 DIRECTORY\nCheck Installation!"), _T("FATAL ERROR 000"), mbOk);
        MessageBoxX(mydir, _T("NO LK8000 DIRECTORY"), mbOk, true);
        RUN_MODE = RUN_EXIT;
        BeforeShutdown();
        goto _exit;
    }

#if !defined(ANDROID) && !defined(OPENVARIO)
    if (!CheckDataDir()) {
        TCHAR mydir[MAX_PATH];
        TCHAR mes[MAX_PATH];

        SystemPath(mydir, _T(LKD_SYSTEM));
        _stprintf(mes, _T("%s"), mydir);
        MessageBoxX(_T("NO SYSTEM DIRECTORY\nCheck Installation!"), _T("FATAL ERROR 001"), mbOk);
        MessageBoxX(mes, _T("NO SYSTEM DIRECTORY"), mbOk, true);
        RUN_MODE = RUN_EXIT;
        BeforeShutdown();
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
        BeforeShutdown();
        goto _exit;
    }
    if (!CheckLanguageEngMsg()) {
        TCHAR mydir[MAX_PATH];
        TCHAR mes[MAX_PATH];
        StartupStore(_T("... CHECK LANGUAGE en.json FAILED!%s"), NEWLINE);
        LocalPath(mydir, _T(LKD_LANGUAGE));
        _stprintf(mes, _T("%s/en.json"), mydir);
        MessageBoxX(_T("en.json MISSING in LANGUAGE\nCheck Language Install"), _T("FATAL ERROR 012"), mbOk);
        MessageBoxX(mes, _T("MISSING FILE!"), mbOk, true);
        RUN_MODE = RUN_EXIT;
        BeforeShutdown();
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
        BeforeShutdown();
        goto _exit;
    }

#ifndef ANDROID
    /*
     * Bitmaps files are inside apk, no need to check
     */

    if (!CheckSystemBitmaps()) {
        TCHAR mydir[MAX_PATH];
        TCHAR mes[MAX_PATH];
        StartupStore(_T("... CHECK SYSTEM _BITMAPSH FAILED!%s"), NEWLINE);
        SystemPath(mydir, _T(LKD_BITMAPS));
        _stprintf(mes, _T("%s/_BITMAPSH"), mydir);
        MessageBoxX(_T("_BITMAPSH MISSING in SYSTEM Bitmaps\nCheck System Install"), _T("FATAL ERROR 032"), mbOk);
        MessageBoxX(mes, _T("MISSING FILE!"), mbOk, true);
        RUN_MODE = RUN_EXIT;
        BeforeShutdown();
        goto _exit;
    }
#endif

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
        BeforeShutdown();
        goto _exit;
    }
#endif

    if (!CheckFilesystemWritable()) {
        MessageBoxX(_T("LK8000 CANNOT WRITE IN MEMORY CARD!\nCARD IS LOCKED, OR DAMAGED, OR FULL."), _T("CRITICAL PROBLEM"), mbOk);
        RUN_MODE = RUN_EXIT;
        BeforeShutdown();
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
                    const TCHAR* szFileName = dfe->GetPathFile();
                    if (_tcscmp(szFileName, _T("PROFILE_RESET")) == 0) {
#if TESTBENCH
                        StartupStore(_T("... Selected FULL RESET virtual profile\n"));
#endif
                        _tcscpy(startProfileFile,szFileName);

                        MessageBoxX(MsgToken(1758), MsgToken(1757), mbOk);
                        FullResetAsked = true;
                        LKProfileResetDefault();
                    } else {
#if TESTBENCH
                        StartupStore(_T("... Selected new profile, preloading..\n"));
#endif
                        LocalPath(startProfileFile, _T(LKD_CONF), szFileName);
                        LKProfileLoad(startProfileFile);
                        FullResetAsked = false;
                    }
                    LKProfileInitRuntime();                    
                }
            }
        }
        if (RUN_MODE == RUN_AIRCRAFT) {
            if (_tcslen(dfe->GetPathFile()) > 0) {
                if (_tcscmp(dfe->GetPathFile(), startAircraftFile)) { // if they are not the same
                    LocalPath(startAircraftFile, _T(LKD_CONF), dfe->GetPathFile());
#if TESTBENCH
                    StartupStore(_T("... Selected new aircraft, preloading..\n"));
#endif
                    LKProfileLoad(startAircraftFile);
                    LKProfileInitRuntime();
                }
            }
        }
        if (RUN_MODE == RUN_DEVICE) {
            if (_tcslen(dfe->GetPathFile()) > 0) {
                if (_tcscmp(dfe->GetPathFile(), startDeviceFile)) { // if they are not the same
                    LocalPath(startDeviceFile, _T(LKD_CONF), dfe->GetPathFile());
#if TESTBENCH
                    StartupStore(_T("... Selected new device, preloading..\n"));
#endif
                    LKProfileLoad(startDeviceFile);
                    LKProfileInitRuntime();
                }
            }
        }
        if (RUN_MODE == RUN_PILOT) {
            if (_tcslen(dfe->GetPathFile()) > 0) {
                if (_tcscmp(dfe->GetPathFile(), startPilotFile)) { // if they are not the same
                    LocalPath(startPilotFile, _T(LKD_CONF), dfe->GetPathFile());
#if TESTBENCH
                    StartupStore(_T("... Selected new pilot, preloading..\n"));
#endif
                    LKProfileLoad(startPilotFile);
                    LKProfileInitRuntime();
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
                MsgToken(198),
                TEXT("LK8000"), mbYesNo) == IdYes) {
            BeforeShutdown();
        } else {
            RUN_MODE = RUN_WELCOME;
        }
    }


_exit:
    if (wf != NULL) {
        delete wf;
        wf = NULL;
    }

    StartBitmap.Release();
    ProfileBitmap.Release();


    if (RUN_MODE == RUN_FLY || RUN_MODE == RUN_SIM) {
        LKSound(_T("LK_SLIDE.WAV"));
        if (CheckInfoUpdated()) dlgChecklistShowModal(3);
        return 0; // do not repeat dialog
    }

    if (RUN_MODE == RUN_EXIT || RUN_MODE == RUN_SHUTDOWN)
        return -1; // terminate
    else
        return 1; // repeat dialog

}
