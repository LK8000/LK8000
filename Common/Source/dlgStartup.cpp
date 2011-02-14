/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgStartup.cpp,v 8.4 2010/12/13 14:20:57 root Exp root $
*/

#include "StdAfx.h"
#include "XCSoar.h"

#include "externs.h"
#include "dlgTools.h"
#include "InfoBoxLayout.h"
#include "MapWindow.h"
#include "Utils2.h"

extern HFONT MapWindowFont;
extern HFONT TitleWindowFont;
extern HFONT InfoWindowFont;
extern void Shutdown(void);

static WndForm *wf=NULL;
static WndOwnerDrawFrame *wSplash=NULL;
#if !LKSTARTUP
static HBITMAP hSplash;
#endif
extern HINSTANCE hInst;

// lines are: 0 - 9
// fsize 0 small 1 normal 2 big
void RawWrite(TCHAR *text, int line, short fsize) { 
   HDC hDC = GetWindowDC(hWndMainWindow);
   HFONT oldfont=(HFONT)SelectObject(hDC,MapWindowFont);
   switch(fsize) {
	case 0:
		oldfont=(HFONT)SelectObject(hDC,TitleWindowFont);
		break;
	case 1:
		break;
	case 2:
		oldfont=(HFONT)SelectObject(hDC,InfoWindowFont);
		break;
   }
   SetBkMode(hDC,TRANSPARENT);
   SIZE tsize;
   GetTextExtentPoint(hDC, text, _tcslen(text), &tsize);
   int y;
   switch (line) {
	case 0:
		y=tsize.cy;
		break;
	case 1:
		y=(tsize.cy*2);
		break;
	case 2:
		y=(tsize.cy*3);
		break;
	case 7:
		y=ScreenSizeY-(tsize.cy*2);
		break;
	case 8:
		y=ScreenSizeY-(tsize.cy*1);
		break;
	case 9:
		y=ScreenSizeY;
		break;
	default:
		y=0;
		break;
   }


   MapWindow::LKWriteText(hDC,text,ScreenSizeX/2,y,0,WTMODE_NORMAL,WTALIGN_CENTER,RGB_AMBER,false);
   SelectObject(hDC,oldfont);
   ReleaseDC(hWndMainWindow, hDC);
}


static void OnSplashPaint(WindowControl * Sender, HDC hDC){

#if LKSTARTUP
 HBITMAP hWelcomeBitmap=NULL;
 TCHAR sDir[MAX_PATH];
 TCHAR srcfile[MAX_PATH];
 bool fullsize=true;

 LocalPath(sDir,TEXT(LKD_BITMAPS));

 // first look for lkstart_480x272.bmp for example
 _stprintf(srcfile,_T("%s\\LKSTART_%s.BMP"),sDir, GetSizeSuffix() );

 if (  GetFileAttributes(srcfile) == 0xffffffff ) {
	fullsize=false;
	switch(ScreenSize) {
                        case ss800x480:
                        case ss640x480:
                        case ss720x408:
                        case ss896x672:
                                _stprintf(srcfile,_T("%s\\LKSTART_LB.BMP"),sDir);
                                break;

                        case ss480x272:
                        case ss480x234:
                        case ss400x240:
                        case ss320x240:
                                _stprintf(srcfile,_T("%s\\LKSTART_LS.BMP"),sDir);
                                break;

                        case ss480x640:
                        case ss480x800:
                                _stprintf(srcfile,_T("%s\\LKSTART_PB.BMP"),sDir);
                                break;

                        case ss240x320:
                        case ss272x480:
                                _stprintf(srcfile,_T("%s\\LKSTART_PS.BMP"),sDir);
                                break;

                        default:
                                _stprintf(srcfile,_T("%s\\LKSTART_DEFAULT.BMP"),sDir);
                                break;
	}
 }

 #if (WINDOWSPC>0)
 hWelcomeBitmap=(HBITMAP)LoadImage(GetModuleHandle(NULL),srcfile,IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
 #else
 hWelcomeBitmap=(HBITMAP)SHLoadDIBitmap(srcfile);
 #endif
 if (hWelcomeBitmap==NULL) hWelcomeBitmap=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SWIFT));

 HDC hTempDC = CreateCompatibleDC(hDC);
 HBITMAP oldBitmap=(HBITMAP)SelectObject(hTempDC, hWelcomeBitmap);

 BITMAP bm;
 GetObject(hWelcomeBitmap,sizeof(bm), &bm);

 StretchBlt(hDC,0,0,
	ScreenSizeX,ScreenSizeY,
	hTempDC, 0, 0,
	2,2,
	BLACKNESS);

  if (fullsize) {
	BitBlt(hDC,0,0,bm.bmWidth,bm.bmHeight,hTempDC, 0, 0, SRCCOPY);
  } else {
  	if ( (bm.bmWidth >ScreenSizeX)||(bm.bmHeight>ScreenSizeY)) {
		StretchBlt(hDC,0,0,
			ScreenSizeX,ScreenSizeY-NIBLSCALE(35),
			hTempDC, 0, 0,
			bm.bmWidth,bm.bmHeight,
			SRCCOPY);
	  } else {
		BitBlt(hDC,(ScreenSizeX-bm.bmWidth)/2,0,bm.bmWidth,IBLSCALE(260),hTempDC, 0, 0, SRCCOPY);
	  }
  }

  DeleteObject(hWelcomeBitmap);
  SelectObject(hTempDC, oldBitmap);
  DeleteDC(hTempDC);



#else
  RECT  rc;
  hSplash=LoadBitmap(hInst, MAKEINTRESOURCE(IDB_SWIFT));
  CopyRect(&rc, Sender->GetBoundRect());
  HDC hDCTemp = CreateCompatibleDC(hDC);
  SelectObject(hDCTemp, hSplash);
  StretchBlt(hDC, 
	     rc.left, rc.top, 
	     rc.right, rc.bottom,
	     hDCTemp, 0, 0, 318, 163, SRCCOPY); 
  DeleteObject(hSplash);
  DeleteDC(hDCTemp);
#endif

}

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;
  wf->SetModalResult(mrOK);
}
static void OnSIMClicked(WindowControl * Sender){
	(void)Sender;
  RUN_MODE=RUN_SIM;
  wf->SetModalResult(mrOK);
}
static void OnFLYClicked(WindowControl * Sender){
	(void)Sender;
  RUN_MODE=RUN_FLY;
  LKForceComPortReset=true;
  PortMonitorMessages=0;
  wf->SetModalResult(mrOK);
}
static void OnPROFILEClicked(WindowControl * Sender){
	(void)Sender;
  RUN_MODE=RUN_PROFILE;
  if (EnableSoundModes) LKSound(_T("LK_SLIDE.WAV"));
  wf->SetModalResult(mrOK);
}
static void OnEXITClicked(WindowControl * Sender){
	(void)Sender;
  RUN_MODE=RUN_EXIT;
  wf->SetModalResult(mrOK);
}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnSplashPaint),
  DeclareCallBackEntry(NULL)
};

extern TCHAR startProfileFile[];


#if LKSTARTUP
bool dlgStartupShowModal(void){
#else
void dlgStartupShowModal(void){
#endif
  WndProperty* wp;

  #if LKSTARTUP
  StartupStore(TEXT(". Startup dialog, RUN_MODE=%d %s"),RUN_MODE,NEWLINE);
  #else 
  StartupStore(TEXT(". Startup dialog%s"),NEWLINE);
  #endif

  char filename[MAX_PATH];
  strcpy(filename,"");
  if (RUN_MODE==RUN_WELCOME) {
	if (!InfoBoxLayout::landscape) {
		LocalPathS(filename, TEXT("dlgFlySim_L.xml"));
		wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_FLYSIM_L"));
	} else {
		LocalPathS(filename, TEXT("dlgFlySim.xml"));
		wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_FLYSIM"));
	}
	#if LKSTARTUP
	if (!wf) {
		return false;
	}
	#else
	if (!wf) return;
	#endif
  } else {
	if (!InfoBoxLayout::landscape) {
		LocalPathS(filename, TEXT("dlgStartup_L.xml"));
		wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_STARTUP_L"));
	} else {
		LocalPathS(filename, TEXT("dlgStartup.xml"));
		wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_STARTUP"));
	}
	#if LKSTARTUP
	if (!wf) return false;
	#else
	if (!wf) return;
	#endif
  }

  wSplash = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmSplash")); 
  wSplash->SetWidth(ScreenSizeX);


  #if LKSTARTUP
  int  PROFWIDTH=0, PROFACCEPTWIDTH=0, PROFHEIGHT=0, PROFSEPARATOR=0;
  if (RUN_MODE==RUN_WELCOME) {
	((WndButton *)wf->FindByName(TEXT("cmdFLY"))) ->SetOnClickNotify(OnFLYClicked);
	((WndButton *)wf->FindByName(TEXT("cmdSIM"))) ->SetOnClickNotify(OnSIMClicked);
	((WndButton *)wf->FindByName(TEXT("cmdPROFILE"))) ->SetOnClickNotify(OnPROFILEClicked);
	((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetOnClickNotify(OnEXITClicked);
	if (ScreenLandscape) {
		
		PROFWIDTH=(ScreenSizeX-IBLSCALE(320))/3; 


		switch(ScreenSize) {
			case ss800x480:
			case ss400x240:
				((WndButton *)wf->FindByName(TEXT("cmdFLY"))) ->SetWidth(IBLSCALE(110));
				((WndButton *)wf->FindByName(TEXT("cmdSIM"))) ->SetWidth(IBLSCALE(110));
				((WndButton *)wf->FindByName(TEXT("cmdSIM"))) ->SetLeft(IBLSCALE(208)+PROFWIDTH*3);
				((WndButton *)wf->FindByName(TEXT("cmdPROFILE"))) ->SetLeft(IBLSCALE(88)+PROFWIDTH);
				((WndButton *)wf->FindByName(TEXT("cmdPROFILE"))) ->SetWidth(IBLSCALE(92)+PROFWIDTH/6);
				((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetLeft(IBLSCALE(161)+PROFWIDTH*2);
				((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetWidth(IBLSCALE(65)+PROFWIDTH/5);
				break;
			case ss480x272:
				((WndButton *)wf->FindByName(TEXT("cmdFLY"))) ->SetWidth(IBLSCALE(117));
				((WndButton *)wf->FindByName(TEXT("cmdFLY"))) ->SetHeight(IBLSCALE(38));
				((WndButton *)wf->FindByName(TEXT("cmdFLY"))) ->SetTop(IBLSCALE(197));
				((WndButton *)wf->FindByName(TEXT("cmdSIM"))) ->SetWidth(IBLSCALE(117));
				((WndButton *)wf->FindByName(TEXT("cmdSIM"))) ->SetLeft(IBLSCALE(201)+PROFWIDTH*3);
				((WndButton *)wf->FindByName(TEXT("cmdSIM"))) ->SetHeight(IBLSCALE(38));
				((WndButton *)wf->FindByName(TEXT("cmdSIM"))) ->SetTop(IBLSCALE(197));
				((WndButton *)wf->FindByName(TEXT("cmdPROFILE"))) ->SetLeft(IBLSCALE(88)+PROFWIDTH);
				((WndButton *)wf->FindByName(TEXT("cmdPROFILE"))) ->SetWidth(IBLSCALE(99)+PROFWIDTH/6);
				((WndButton *)wf->FindByName(TEXT("cmdPROFILE"))) ->SetHeight(IBLSCALE(38));
				((WndButton *)wf->FindByName(TEXT("cmdPROFILE"))) ->SetTop(IBLSCALE(197));
				((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetLeft(IBLSCALE(161)+PROFWIDTH*2);
				((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetWidth(IBLSCALE(65)+PROFWIDTH/5);
				((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetHeight(IBLSCALE(38));
				((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetTop(IBLSCALE(197));
				break;
			case ss640x480:
			case ss320x240:
				((WndButton *)wf->FindByName(TEXT("cmdPROFILE"))) ->SetLeft(IBLSCALE(93)+PROFWIDTH);
				((WndButton *)wf->FindByName(TEXT("cmdPROFILE"))) ->SetWidth(IBLSCALE(73)+PROFWIDTH/6);
				((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetLeft(IBLSCALE(166)+PROFWIDTH*2);
				((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetWidth(IBLSCALE(60)+PROFWIDTH/5);
				((WndButton *)wf->FindByName(TEXT("cmdSIM"))) ->SetLeft(IBLSCALE(228)+PROFWIDTH*3);
				((WndButton *)wf->FindByName(TEXT("cmdSIM"))) ->SetWidth(IBLSCALE(88));
				break;
			default:
				((WndButton *)wf->FindByName(TEXT("cmdPROFILE"))) ->SetLeft(IBLSCALE(93)+PROFWIDTH);
				((WndButton *)wf->FindByName(TEXT("cmdPROFILE"))) ->SetWidth(IBLSCALE(73)+PROFWIDTH/6);
				((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetLeft(IBLSCALE(166)+PROFWIDTH*2);
				((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetWidth(IBLSCALE(60)+PROFWIDTH/5);
				((WndButton *)wf->FindByName(TEXT("cmdSIM"))) ->SetLeft(IBLSCALE(228)+PROFWIDTH*3);
				break;
		}	
	} else {
		PROFWIDTH=IBLSCALE(236);
		PROFACCEPTWIDTH=NIBLSCALE(45);
		PROFHEIGHT=NIBLSCALE(25);
		PROFSEPARATOR=NIBLSCALE(2);
	}
  }

  if (RUN_MODE==RUN_PROFILE) {
	((WndButton *)wf->FindByName(TEXT("cmdClose"))) ->SetOnClickNotify(OnCloseClicked);
	if (ScreenLandscape) {
		PROFWIDTH=IBLSCALE(256);
		PROFACCEPTWIDTH=NIBLSCALE(60);
		PROFHEIGHT=NIBLSCALE(30);
		PROFSEPARATOR=NIBLSCALE(4);
		((WndButton *)wf->FindByName(TEXT("cmdClose"))) ->SetWidth(PROFACCEPTWIDTH);
		((WndButton *)wf->FindByName(TEXT("cmdClose"))) ->
			SetLeft((((ScreenSizeX-PROFWIDTH-PROFSEPARATOR-PROFACCEPTWIDTH)/2)+PROFSEPARATOR+PROFWIDTH)-NIBLSCALE(2));
		((WndButton *)wf->FindByName(TEXT("cmdClose"))) ->SetHeight(PROFHEIGHT-NIBLSCALE(4));
	} else {
		PROFWIDTH=IBLSCALE(236);
		PROFACCEPTWIDTH=NIBLSCALE(45);
		PROFHEIGHT=NIBLSCALE(25);
		PROFSEPARATOR=NIBLSCALE(2);
		((WndButton *)wf->FindByName(TEXT("cmdClose"))) ->SetWidth(ScreenSizeX-NIBLSCALE(6));
		((WndButton *)wf->FindByName(TEXT("cmdClose"))) -> SetLeft(NIBLSCALE(2));
	}
  }


  #else
  ((WndButton *)wf->FindByName(TEXT("cmdClose"))) ->SetOnClickNotify(OnCloseClicked);
  ((WndButton *)wf->FindByName(TEXT("cmdClose"))) ->SetWidth(ScreenSizeX-NIBLSCALE(6));
  #endif

  TCHAR temp[MAX_PATH];

  wf->SetHeight(ScreenSizeY);
  wf->SetWidth(ScreenSizeX);

  wp = ((WndProperty *)wf->FindByName(TEXT("prpProfile")));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();
    _stprintf(temp,_T("*%S"),LKS_PRF); 
    dfe->ScanDirectoryTop(_T(LKD_CONF),temp); 
    dfe->Lookup(startProfileFile);

    #if LKSTARTUP
    wp->SetHeight(PROFHEIGHT);
    wp->SetWidth(PROFWIDTH);
    if (ScreenLandscape)
    	wp->SetLeft(((ScreenSizeX-PROFWIDTH-PROFSEPARATOR-PROFACCEPTWIDTH)/2)-NIBLSCALE(2));
    else
    	wp->SetLeft(0);
    #else

    wp->SetHeight(NIBLSCALE(25));
    int xs=ScreenSizeX-NIBLSCALE(2)-(NIBLSCALE(60));
    if (!ScreenLandscape) xs+=NIBLSCALE(20);
    wp->SetWidth(xs);
    wp->SetLeft(( ScreenSizeX-xs)/2);
    #endif

    wp->RefreshDisplay();
    #ifndef LKSTARTUP
    if (dfe->GetNumFiles()<=2) {
      delete wf;
      wf = NULL;
      return;
    }
    #endif
  }

  if  (!CheckDataDir()) {
	TCHAR mydir[MAX_PATH];
	TCHAR mes[MAX_PATH];

	_stprintf(mes,_T("%s v%s.%s"),_T(LKFORK),_T(LKVERSION),_T(LKRELEASE));
	RawWrite(mes,1,1);
	LocalPath(mydir,_T(LKD_SYSTEM));
	_stprintf(mes,_T("%s"),mydir);
	RawWrite(_T("Directory or configuration files missing"),8,1);
	RawWrite(mes,9,0);
	MessageBoxX(hWndMainWindow, _T("Check Installation"), _T("FATAL ERROR 001"), MB_OK|MB_ICONQUESTION);
	Shutdown();
  }

  if  (!CheckLanguageDir()) {
	TCHAR mydir[MAX_PATH];
	TCHAR mes[MAX_PATH];
	StartupStore(_T("... CHECK LANGUAGE DIRECTORY FAILED!%s"),NEWLINE);

	_stprintf(mes,_T("%s v%s.%s"),_T(LKFORK),_T(LKVERSION),_T(LKRELEASE));
	RawWrite(mes,1,1);
	LocalPath(mydir,_T(LKD_LANGUAGE));
	_stprintf(mes,_T("%s"),mydir);
	RawWrite(_T("Directory or configuration files missing"),8,1);
	RawWrite(mes,9,0);
	MessageBoxX(hWndMainWindow, _T("Check Language Install"), _T("FATAL ERROR 002"), MB_OK|MB_ICONQUESTION);
	Shutdown();
  }
  wf->ShowModal();

  wp = (WndProperty*)wf->FindByName(TEXT("prpProfile"));
  if (wp) {
	DataFieldFileReader* dfe;
	dfe = (DataFieldFileReader*)wp->GetDataField();
	if (_tcslen(dfe->GetPathFile())>0) {
		_tcscpy(startProfileFile,dfe->GetPathFile());
	}
	if (EnableSoundModes) LKSound(_T("LK_SLIDE.WAV"));
	RUN_MODE=RUN_WELCOME;
  }
  if (RUN_MODE==RUN_EXIT) {
	if (EnableSoundModes) LKSound(_T("LK_SLIDE.WAV"));
	if (MessageBoxX(hWndMainWindow, 
	// LKTOKEN  _@M198_ = "Confirm Exit?" 
		gettext(TEXT("_@M198_")), 
		TEXT("LK8000"), MB_YESNO|MB_ICONQUESTION) == IDYES) {
		Shutdown();
	} else
		RUN_MODE=RUN_WELCOME;
  }

  delete wf;

  wf = NULL;

  #if LKSTARTUP
  if (RUN_MODE==RUN_FLY) {
	if (EnableSoundModes) LKSound(_T("LK_SLIDE.WAV"));
	return false;
  }
  if (RUN_MODE==RUN_SIM) {
	if (EnableSoundModes) LKSound(_T("LK_SLIDE.WAV"));
	return false;
  }

  return true; // else repeat dialog
  #endif

}

