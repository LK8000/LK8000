/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: dlgStartup.cpp,v 1.1 2011/12/21 10:29:29 root Exp $
*/

#include "externs.h"
#include "InfoBoxLayout.h"
#include "RGB.h"
#include "resource.h"
#include "LKObjects.h"
#include "LKProfiles.h"
#include "Dialogs.h"


extern void Shutdown(void);
extern void LoadSplash(HDC hDC,TCHAR *splashfile);

static WndForm *wf=NULL;
static WndOwnerDrawFrame *wSplash=NULL;

extern bool CheckSystemDefaultMenu(void);
extern bool CheckLanguageEngMsg(void);
extern bool CheckSystemBitmaps(void);
void RawWrite(HDC hDC, TCHAR *text, int line, short fsize,COLORREF rgbcolor,int wtmode);

bool fullresetasked=false;

static int OnTimerNotify(WindowControl *Sender)
{

   // Do we have a premature shutdown request from system quit?
   if (RUN_MODE==RUN_SHUTDOWN) {
	#if TESTBENCH
	StartupStore(_T("... dlGStartup shutdown requested\n"));
	#endif
	wf->SetModalResult(mrOK);
   }

   if (RUN_MODE==RUN_WELCOME) {

	static unsigned short i=0;
	//if((i++ % 2) != 0) // called at 2hz, run once per second
	//	return 0;
   	int pos=0;
   	switch (ScreenSize) {
		case ss800x480:
			pos=12;
			break;
		case ss400x240:
			pos=12;
			break;
		case ss480x272:
			if (ScreenSizeX==854)
				pos=14;
			else
				pos=11;
			break;
		case ss640x480:
			pos=12;
			break;
		case ss320x240:
			pos=12;
			break;
		case ss896x672:
			pos=14;
			break;
		// --------- portrait -------------
		case ss240x320:
			pos=17;
			break;
		case ss480x640:
			pos=17;
			break;
		case ss272x480:
			pos=18;
			break;
		case ss240x400:
			pos=16;
			break;
		case ss480x800:
			pos=18;
			break;
		default:
			pos=11;
			break;
   	}

	TCHAR mes[100];
	if (fullresetasked) {
		_stprintf(mes,_T("*** %s ***"),gettext(_T("_@M1757_")));
		RawWrite(Sender->GetDeviceContext(),mes,pos,1, RGB_DARKWHITE,WTMODE_NORMAL);
	} else {

		#ifndef LKCOMPETITION
		_stprintf(mes,_T("Version %S.%S (%S)"),LKVERSION,LKRELEASE,__DATE__);
		#else
		_stprintf(mes,_T("V%S.%S (%S) COMPETITION"),LKVERSION,LKRELEASE,__DATE__);
		#endif

		short k=0;
		short r,g,b;

		r=rand()%256; g=rand()%256; b=rand()%256;
		RawWrite(Sender->GetDeviceContext(),mes,pos,1, RGB(r,g,r),WTMODE_NORMAL);
	}
   }


   return 0;
}
  


// Syntax  hdc _Text linenumber fontsize 
// lines are: 0 - 9
// fsize 0 small 1 normal 2 big
void RawWrite(HDC hDC, TCHAR *text, int line, short fsize,COLORREF rgbcolor,int wtmode) { 
   HFONT oldfont=(HFONT)SelectObject(hDC,MapWindowFont);
   switch(fsize) {
	case 0:
		SelectObject(hDC,TitleWindowFont);
		break;
	case 1:
		SelectObject(hDC,LK8MapFont);
		break;
	case 2:
		SelectObject(hDC,LK8MediumFont);
		break;
	case 3:
		SelectObject(hDC,LK8BigFont);
		break;
   }
   SetBkMode(hDC,TRANSPARENT);
   SIZE tsize;
   GetTextExtentPoint(hDC, text, _tcslen(text), &tsize);

   int y;

   if (RUN_MODE==RUN_WELCOME)
	y=tsize.cy*(line-1) + (tsize.cy);
   else
	y=tsize.cy*(line-1) + (tsize.cy/2);

   MapWindow::LKWriteText(hDC,text,ScreenSizeX/2,y,0,wtmode,WTALIGN_CENTER,rgbcolor,false);
   SelectObject(hDC,oldfont);
}



static void OnSplashPaint(WindowControl * Sender, HDC hDC){

 TCHAR srcfile[MAX_PATH];
 TCHAR fprefix[20];

 if (RUN_MODE==RUN_SHUTDOWN) return;

 if (RUN_MODE==RUN_WELCOME) 
	_tcscpy(fprefix,_T("LKSTART"));
 else
	_tcscpy(fprefix,_T("LKPROFILE"));

 LoadSplash(hDC,fprefix);

#if 0
  if (RUN_MODE==RUN_WELCOME) {
	TCHAR mes[100];
	int pos=0;
	switch (ScreenSize) {
		case ss800x480:
			pos=12;
			break;
		case ss400x240:
			pos=12;
			break;
		case ss480x272:
			if (ScreenSizeX==854)
				pos=14;
			else
				pos=11;
			break;
		case ss640x480:
			pos=12;
			break;
		case ss320x240:
			pos=12;
			break;
		case ss896x672:
			pos=14;
			break;
		// --------- portrait -------------
		case ss240x320:
			pos=17;
			break;
		case ss480x640:
			pos=17;
			break;
		case ss272x480:
			pos=18;
			break;
		case ss240x400:
			pos=16;
			break;
		case ss480x800:
			pos=18;
			break;
		default:
			pos=11;
			break;
	}
	if (fullresetasked) {
		_stprintf(mes,_T("*** %s ***"),gettext(_T("_@M1757_")));
		RawWrite(hDC,mes,pos,1, RGB_DARKWHITE,WTMODE_NORMAL);
	} else {
#ifndef LKCOMPETITION
		_stprintf(mes,_T("Version %S.%S (%S)"),LKVERSION,LKRELEASE,__DATE__);
#else
		_stprintf(mes,_T("V%S.%S COMPETITION"),LKVERSION,LKRELEASE,__DATE__);
#endif
		RawWrite(hDC,mes,pos,1, RGB_DARKWHITE,WTMODE_NORMAL);
	}
  }
#endif

  if (RUN_MODE!=RUN_WELCOME) {

	// FillRect(hDC,&ScreenSizeR, LKBrush_Black); // REMOVE 

	TCHAR mes[100];
#ifndef LKCOMPETITION
	_stprintf(mes,_T("%S v%S.%S - %s"),LKFORK,LKVERSION,LKRELEASE,gettext(_T("_@M2054_")));
#else
	_stprintf(mes,_T("%SC v%S.%S - %s"),LKFORK,LKVERSION,LKRELEASE,gettext(_T("_@M2054_")));
#endif
	RawWrite(hDC,mes,1,1, RGB_LIGHTGREY,WTMODE_NORMAL);

	unsigned long freeram = CheckFreeRam()/1024;
	TCHAR buffer[MAX_PATH];
	LocalPath(buffer);
	unsigned long freestorage = FindFreeSpace(buffer);
	_stprintf(mes,_T("free ram %.1ldM  storage %.1ldM"), freeram/1024,freestorage/1024);
	RawWrite(hDC,mes,3,0, RGB_LIGHTGREY,WTMODE_NORMAL);

	if ( ScreenSize != ss320x240 && ScreenLandscape )
	RawWrite(hDC,_T("_______________________"),2,2, RGB_LIGHTGREY,WTMODE_NORMAL);

	if (fullresetasked) {
		_stprintf(mes,_T("%s"),gettext(_T("_@M1757_")));	// LK8000 PROFILES RESET
		RawWrite(hDC,mes,5,2, RGB_ICEWHITE, WTMODE_OUTLINED);
		_stprintf(mes,_T("%s"),gettext(_T("_@M1759_")));	// SELECTED IN SYSTEM
		RawWrite(hDC,mes,6,2, RGB_ICEWHITE, WTMODE_OUTLINED);
	} else {
		_stprintf(mes,_T("%s"),PilotName_Config);
		RawWrite(hDC,mes,4,2, RGB_ICEWHITE, WTMODE_OUTLINED);

		_stprintf(mes,_T("%s"),AircraftRego_Config);
		RawWrite(hDC,mes,5,2, RGB_AMBER, WTMODE_OUTLINED);

		_stprintf(mes,_T("%s"),AircraftType_Config);
		RawWrite(hDC,mes,6,2, RGB_AMBER, WTMODE_OUTLINED);

    LKASSERT(szPolarFile[0]);
		extern void LK_wsplitpath(const WCHAR* path, WCHAR* drv, WCHAR* dir, WCHAR* name, WCHAR* ext);
		LK_wsplitpath(szPolarFile, (WCHAR*) NULL, (WCHAR*) NULL, srcfile, (WCHAR*) NULL);

		_stprintf(mes,_T("%s %s"),gettext(_T("_@M528_")),srcfile);  // polar file
		RawWrite(hDC,mes,7,2, RGB_AMBER, WTMODE_OUTLINED);

    LKASSERT(startProfileFile[0]);
		LK_wsplitpath(startProfileFile, (WCHAR*) NULL, (WCHAR*) NULL, srcfile, (WCHAR*) NULL);
		_stprintf(mes,_T("%s: %s"),MsgToken(1746),srcfile);  
		RawWrite(hDC,mes,11,1, RGB_ICEWHITE, WTMODE_NORMAL);
	}


	// RawWrite(hDC,_T("_______________________"),8,2, RGB_LIGHTGREY,WTMODE_NORMAL); // REMOVE FOR THE 3.0

	return;
  }

}

static void OnCloseClicked(WindowControl * Sender){
	(void)Sender;

  if (EnableSoundModes) LKSound(_T("LK_SLIDE.WAV"));
  switch(RUN_MODE) {
	case RUN_DUALPROF:
		RUN_MODE=RUN_WELCOME;
		break;
  }
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
//  Removed 110605: we now run devInit on startup for all devices, and we dont want an immediate and useless reset.
//  LKForceComPortReset=true; 
  PortMonitorMessages=0;
  wf->SetModalResult(mrOK);
}
static void OnDUALPROFILEClicked(WindowControl * Sender){
	(void)Sender;
  RUN_MODE=RUN_DUALPROF;
  if (EnableSoundModes) LKSound(_T("LK_SLIDE.WAV"));
  wf->SetModalResult(mrOK);
}
static void OnEXITClicked(WindowControl * Sender){
	(void)Sender;
  RUN_MODE=RUN_EXIT;
  wf->SetModalResult(mrOK);
}
static void OnPROFILEClicked(WindowControl * Sender){
	(void)Sender;
  RUN_MODE=RUN_PROFILE;
  if (EnableSoundModes) LKSound(_T("LK_SLIDE.WAV"));
  wf->SetModalResult(mrOK);
}
static void OnAIRCRAFTClicked(WindowControl * Sender){
	(void)Sender;
  RUN_MODE=RUN_AIRCRAFT;
  if (EnableSoundModes) LKSound(_T("LK_SLIDE.WAV"));
  wf->SetModalResult(mrOK);
}
static void OnDEVICEClicked(WindowControl * Sender){
	(void)Sender;
  RUN_MODE=RUN_DEVICE;
  if (EnableSoundModes) LKSound(_T("LK_SLIDE.WAV"));
  wf->SetModalResult(mrOK);
}
static void OnPILOTClicked(WindowControl * Sender){
	(void)Sender;
  RUN_MODE=RUN_PILOT;
  if (EnableSoundModes) LKSound(_T("LK_SLIDE.WAV"));
  wf->SetModalResult(mrOK);
}

static CallBackTableEntry_t CallBackTable[]={
  DeclareCallBackEntry(OnSplashPaint),
  DeclareCallBackEntry(NULL)
};


// This function will be recalled as long it returns 1.
// Returning 0 will indicate proceed with startup
// Returning -1 will indicate exit program

short dlgStartupShowModal(void){
  WndProperty* wp;

  #if TESTBENCH
  StartupStore(TEXT(". Startup dialog, RUN_MODE=%d %s"),RUN_MODE,NEWLINE);
  #endif

  char filename[MAX_PATH];
  strcpy(filename,"");

  // FLY SIM PROFILE EXIT
  if (RUN_MODE==RUN_WELCOME) {
	if (!ScreenLandscape) {
		LocalPathS(filename, TEXT("dlgFlySim_L.xml"));
		wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_FLYSIM_L"));
	} else {
		LocalPathS(filename, TEXT("dlgFlySim.xml"));
		wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_FLYSIM"));
	}
	if (!wf) {
		return 0;
	}
  }

  //  PROFILE AIRCRAFT  CLOSE
  if (RUN_MODE==RUN_DUALPROF) {
	if (!ScreenLandscape) {
		LocalPathS(filename, TEXT("dlgDualProfile_L.xml"));
		wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_DUALPROFILE_L"));
	} else {
		LocalPathS(filename, TEXT("dlgDualProfile.xml"));
		wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_DUALPROFILE"));
	}
	if (!wf) return 0;
  }

  // CHOOSE PROFILE
  if (RUN_MODE==RUN_PROFILE || RUN_MODE==RUN_AIRCRAFT || RUN_MODE==RUN_PILOT || RUN_MODE==RUN_DEVICE) {
	if (!ScreenLandscape) {
		LocalPathS(filename, TEXT("dlgStartup_L.xml"));
		wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_STARTUP_L"));
	} else {
		LocalPathS(filename, TEXT("dlgStartup.xml"));
		wf = dlgLoadFromXML(CallBackTable, filename, hWndMainWindow, TEXT("IDR_XML_STARTUP"));
	}
	if (!wf) return 0;
  }

  wSplash = (WndOwnerDrawFrame*)wf->FindByName(TEXT("frmSplash")); 
  wSplash->SetWidth(ScreenSizeX);


  int  PROFWIDTH=0, PROFACCEPTWIDTH=0, PROFHEIGHT=0, PROFSEPARATOR=0;

  if (RUN_MODE==RUN_WELCOME) {
	((WndButton *)wf->FindByName(TEXT("cmdFLY"))) ->SetOnClickNotify(OnFLYClicked);
	((WndButton *)wf->FindByName(TEXT("cmdSIM"))) ->SetOnClickNotify(OnSIMClicked);
	((WndButton *)wf->FindByName(TEXT("cmdDUALPROFILE"))) ->SetOnClickNotify(OnDUALPROFILEClicked);
	((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetOnClickNotify(OnEXITClicked);
	if (ScreenLandscape) {
		
		PROFWIDTH=(ScreenSizeX-IBLSCALE(320))/3; 


		switch(ScreenSize) {
			case ss800x480:
			case ss400x240:
				((WndButton *)wf->FindByName(TEXT("cmdFLY"))) ->SetWidth(IBLSCALE(110));
				((WndButton *)wf->FindByName(TEXT("cmdSIM"))) ->SetWidth(IBLSCALE(110));
				((WndButton *)wf->FindByName(TEXT("cmdSIM"))) ->SetLeft(IBLSCALE(208)+PROFWIDTH*3);
				((WndButton *)wf->FindByName(TEXT("cmdDUALPROFILE"))) ->SetLeft(IBLSCALE(88)+PROFWIDTH);
				((WndButton *)wf->FindByName(TEXT("cmdDUALPROFILE"))) ->SetWidth(IBLSCALE(92)+PROFWIDTH/6);
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
				((WndButton *)wf->FindByName(TEXT("cmdDUALPROFILE"))) ->SetLeft(IBLSCALE(88)+PROFWIDTH);
				((WndButton *)wf->FindByName(TEXT("cmdDUALPROFILE"))) ->SetWidth(IBLSCALE(99)+PROFWIDTH/6);
				((WndButton *)wf->FindByName(TEXT("cmdDUALPROFILE"))) ->SetHeight(IBLSCALE(38));
				((WndButton *)wf->FindByName(TEXT("cmdDUALPROFILE"))) ->SetTop(IBLSCALE(197));
				((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetLeft(IBLSCALE(161)+PROFWIDTH*2);
				((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetWidth(IBLSCALE(65)+PROFWIDTH/5);
				((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetHeight(IBLSCALE(38));
				((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetTop(IBLSCALE(197));
				break;
			case ss640x480:
			case ss320x240:
				((WndButton *)wf->FindByName(TEXT("cmdDUALPROFILE"))) ->SetLeft(IBLSCALE(93)+PROFWIDTH);
				((WndButton *)wf->FindByName(TEXT("cmdDUALPROFILE"))) ->SetWidth(IBLSCALE(73)+PROFWIDTH/6);
				((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetLeft(IBLSCALE(166)+PROFWIDTH*2);
				((WndButton *)wf->FindByName(TEXT("cmdEXIT"))) ->SetWidth(IBLSCALE(60)+PROFWIDTH/5);
				((WndButton *)wf->FindByName(TEXT("cmdSIM"))) ->SetLeft(IBLSCALE(228)+PROFWIDTH*3);
				((WndButton *)wf->FindByName(TEXT("cmdSIM"))) ->SetWidth(IBLSCALE(88));
				break;
			default:
				((WndButton *)wf->FindByName(TEXT("cmdDUALPROFILE"))) ->SetLeft(IBLSCALE(93)+PROFWIDTH);
				((WndButton *)wf->FindByName(TEXT("cmdDUALPROFILE"))) ->SetWidth(IBLSCALE(73)+PROFWIDTH/6);
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

  if (RUN_MODE==RUN_DUALPROF) {
	((WndButton *)wf->FindByName(TEXT("cmdAIRCRAFT"))) ->SetOnClickNotify(OnAIRCRAFTClicked);
	((WndButton *)wf->FindByName(TEXT("cmdPROFILE"))) ->SetOnClickNotify(OnPROFILEClicked);
	((WndButton *)wf->FindByName(TEXT("cmdDEVICE"))) ->SetOnClickNotify(OnDEVICEClicked);
	((WndButton *)wf->FindByName(TEXT("cmdPILOT"))) ->SetOnClickNotify(OnPILOTClicked);
	((WndButton *)wf->FindByName(TEXT("cmdCLOSE"))) ->SetOnClickNotify(OnCloseClicked);

	unsigned int lx;
	unsigned int w;
	unsigned int SPACEBORDER;

	if (ScreenLandscape) {

		PROFWIDTH=(ScreenSizeX-IBLSCALE(320))/3;

		SPACEBORDER=NIBLSCALE(2);

		w= ( ScreenSizeX - (SPACEBORDER*6) ) /5;

		((WndButton *)wf->FindByName(TEXT("cmdAIRCRAFT"))) ->SetWidth(w);
		((WndButton *)wf->FindByName(TEXT("cmdPROFILE"))) ->SetWidth(w);
		((WndButton *)wf->FindByName(TEXT("cmdDEVICE"))) ->SetWidth(w);
		((WndButton *)wf->FindByName(TEXT("cmdPILOT"))) ->SetWidth(w);
		((WndButton *)wf->FindByName(TEXT("cmdCLOSE"))) ->SetWidth(w);

		lx=SPACEBORDER-1; // count from 0
		((WndButton *)wf->FindByName(TEXT("cmdAIRCRAFT"))) ->SetLeft(lx);
		lx+=w+SPACEBORDER;
		((WndButton *)wf->FindByName(TEXT("cmdPROFILE"))) ->SetLeft(lx);
		lx+=w+SPACEBORDER;
		((WndButton *)wf->FindByName(TEXT("cmdDEVICE"))) ->SetLeft(lx);
		lx+=w+SPACEBORDER;
		((WndButton *)wf->FindByName(TEXT("cmdPILOT"))) ->SetLeft(lx);
		lx+=w+SPACEBORDER;
		((WndButton *)wf->FindByName(TEXT("cmdCLOSE"))) ->SetLeft(lx);

	} else {
		SPACEBORDER=NIBLSCALE(2);
		w= ( ScreenSizeX - (SPACEBORDER*3) ) /2;

		((WndButton *)wf->FindByName(TEXT("cmdAIRCRAFT"))) ->SetWidth(w);
		((WndButton *)wf->FindByName(TEXT("cmdPROFILE"))) ->SetWidth(w);
		lx=SPACEBORDER-1; // count from 0
		((WndButton *)wf->FindByName(TEXT("cmdAIRCRAFT"))) ->SetLeft(lx);
		lx+=w+SPACEBORDER;
		((WndButton *)wf->FindByName(TEXT("cmdPROFILE"))) ->SetLeft(lx);

		w= ( ScreenSizeX - (SPACEBORDER*4) ) /3;
		((WndButton *)wf->FindByName(TEXT("cmdDEVICE"))) ->SetWidth(w);
		((WndButton *)wf->FindByName(TEXT("cmdPILOT"))) ->SetWidth(w);
		((WndButton *)wf->FindByName(TEXT("cmdCLOSE"))) ->SetWidth(w);
		lx=SPACEBORDER-1; // count from 0
		((WndButton *)wf->FindByName(TEXT("cmdDEVICE"))) ->SetLeft(lx);
		lx+=w+SPACEBORDER;
		((WndButton *)wf->FindByName(TEXT("cmdPILOT"))) ->SetLeft(lx);
		lx+=w+SPACEBORDER;
		((WndButton *)wf->FindByName(TEXT("cmdCLOSE"))) ->SetLeft(lx);

		// We could greatly simplify these, now that we use dynamic scaling here.
		PROFWIDTH=IBLSCALE(236);
		PROFACCEPTWIDTH=NIBLSCALE(45);
		PROFHEIGHT=NIBLSCALE(25);
		PROFSEPARATOR=NIBLSCALE(2);
	}
  }



  if (RUN_MODE==RUN_PROFILE || RUN_MODE==RUN_AIRCRAFT || RUN_MODE==RUN_PILOT || RUN_MODE==RUN_DEVICE) {
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

  TCHAR temp[MAX_PATH];

  wf->SetHeight(ScreenSizeY);
  wf->SetWidth(ScreenSizeX);

  //
  // File selection shared  by PROFILEs choices
  //
  wp = ((WndProperty *)wf->FindByName(TEXT("prpProfile")));
  if (wp) {
    DataFieldFileReader* dfe;
    dfe = (DataFieldFileReader*)wp->GetDataField();

    if (RUN_MODE==RUN_PROFILE) {
	    _stprintf(temp,_T("*%S"),LKS_PRF); 
	    dfe->ScanDirectoryTop(_T(LKD_CONF),temp); 
	    dfe->addFile(gettext(_T("_@M1741_")),_T("PROFILE_RESET")); 
	    dfe->Lookup(startProfileFile);
    } 
    if (RUN_MODE==RUN_AIRCRAFT) {
	    _stprintf(temp,_T("*%S"),LKS_AIRCRAFT); 
	    dfe->ScanDirectoryTop(_T(LKD_CONF),temp); 
	    dfe->Lookup(startAircraftFile);
    }
    if (RUN_MODE==RUN_DEVICE) {
	    _stprintf(temp,_T("*%S"),LKS_DEVICE); 
	    dfe->ScanDirectoryTop(_T(LKD_CONF),temp); 
	    dfe->Lookup(startDeviceFile);
    }
    if (RUN_MODE==RUN_PILOT) {
	    _stprintf(temp,_T("*%S"),LKS_PILOT); 
	    dfe->ScanDirectoryTop(_T(LKD_CONF),temp); 
	    dfe->Lookup(startPilotFile);
    }

    // Set the black stripe in size with ACCEPT button
    wp->SetHeight(PROFHEIGHT);
    wp->SetWidth(PROFWIDTH);

    // Set rescaled offset for positioning the ACCEPT
    if (ScreenLandscape)
    	wp->SetLeft(((ScreenSizeX-PROFWIDTH-PROFSEPARATOR-PROFACCEPTWIDTH)/2)-NIBLSCALE(2));
    else
    	wp->SetLeft(0);

    wp->RefreshDisplay();
  }

  extern bool CheckFilesystemWritable(void);
  if (!CheckFilesystemWritable()) {
	MessageBoxX(hWndMainWindow, _T("LK8000 CANNOT WRITE IN MEMORY CARD!\nCARD IS LOCKED, OR DAMAGED, OR FULL."), _T("CRITICAL PROBLEM"), MB_OK|MB_ICONQUESTION);
	RUN_MODE=RUN_EXIT;
	Shutdown();
	goto _exit;
  }

  if  (!CheckRootDir()) {
	TCHAR mydir[MAX_PATH];
	TCHAR mes[MAX_PATH];

	LocalPath(mydir,_T(""));
	_stprintf(mes,_T("%s"),mydir);
	MessageBoxX(hWndMainWindow, _T("NO LK8000 DIRECTORY\nCheck Installation!"), _T("FATAL ERROR 000"), MB_OK|MB_ICONQUESTION);
	MessageBoxX(hWndMainWindow, mes, _T("NO LK8000 DIRECTORY"), MB_OK|MB_ICONQUESTION, true);
	RUN_MODE=RUN_EXIT;
	Shutdown();
	goto _exit;
  }

  if  (!CheckDataDir()) {
	TCHAR mydir[MAX_PATH];
	TCHAR mes[MAX_PATH];

	LocalPath(mydir,_T(LKD_SYSTEM));
	_stprintf(mes,_T("%s"),mydir);
	MessageBoxX(hWndMainWindow, _T("NO SYSTEM DIRECTORY\nCheck Installation!"), _T("FATAL ERROR 001"), MB_OK|MB_ICONQUESTION);
	MessageBoxX(hWndMainWindow, mes, _T("NO SYSTEM DIRECTORY"), MB_OK|MB_ICONQUESTION, true);
	RUN_MODE=RUN_EXIT;
	Shutdown();
	goto _exit;
  }

  if  (!CheckLanguageDir()) {
	TCHAR mydir[MAX_PATH];
	TCHAR mes[MAX_PATH];
	StartupStore(_T("... CHECK LANGUAGE DIRECTORY FAILED!%s"),NEWLINE);

	LocalPath(mydir,_T(LKD_LANGUAGE));
	_stprintf(mes,_T("%s"),mydir);
	MessageBoxX(hWndMainWindow, _T("LANGUAGE DIRECTORY CHECK FAIL\nCheck Language Install"), _T("FATAL ERROR 002"), MB_OK|MB_ICONQUESTION);
	MessageBoxX(hWndMainWindow, mes, _T("NO LANGUAGE DIRECTORY"), MB_OK|MB_ICONQUESTION, true);
	RUN_MODE=RUN_EXIT;
	Shutdown();
	goto _exit;
  }
  if  (!CheckLanguageEngMsg()) {
	TCHAR mydir[MAX_PATH];
	TCHAR mes[MAX_PATH];
	StartupStore(_T("... CHECK LANGUAGE ENG_MSG FAILED!%s"),NEWLINE);
	LocalPath(mydir,_T(LKD_LANGUAGE));
	_stprintf(mes,_T("%s/ENG_MSG.TXT"),mydir);
	MessageBoxX(hWndMainWindow, _T("ENG_MSG.TXT MISSING in LANGUAGE\nCheck Language Install"), _T("FATAL ERROR 012"), MB_OK|MB_ICONQUESTION);
	MessageBoxX(hWndMainWindow, mes, _T("MISSING FILE!"), MB_OK|MB_ICONQUESTION, true);
	RUN_MODE=RUN_EXIT;
	Shutdown();
	goto _exit;
  }
  if  (!CheckSystemDefaultMenu()) {
	TCHAR mydir[MAX_PATH];
	TCHAR mes[MAX_PATH];
	StartupStore(_T("... CHECK SYSTEM DEFAULT_MENU.TXT FAILED!%s"),NEWLINE);
	LocalPath(mydir,_T(LKD_SYSTEM));
	_stprintf(mes,_T("%s/DEFAULT_MENU.TXT"),mydir);
	MessageBoxX(hWndMainWindow, _T("DEFAULT_MENU.TXT MISSING in SYSTEM\nCheck System Install"), _T("FATAL ERROR 022"), MB_OK|MB_ICONQUESTION);
	MessageBoxX(hWndMainWindow, mes, _T("MISSING FILE!"), MB_OK|MB_ICONQUESTION, true);
	RUN_MODE=RUN_EXIT;
	Shutdown();
	goto _exit;
  }

  if  (!CheckSystemBitmaps()) {
	TCHAR mydir[MAX_PATH];
	TCHAR mes[MAX_PATH];
	StartupStore(_T("... CHECK SYSTEM _BITMAPSH FAILED!%s"),NEWLINE);
	LocalPath(mydir,_T(LKD_BITMAPS));
	_stprintf(mes,_T("%s/_BITMAPSH"),mydir);
	MessageBoxX(hWndMainWindow, _T("_BITMAPSH MISSING in SYSTEM Bitmaps\nCheck System Install"), _T("FATAL ERROR 032"), MB_OK|MB_ICONQUESTION);
	MessageBoxX(hWndMainWindow, mes, _T("MISSING FILE!"), MB_OK|MB_ICONQUESTION, true);
	RUN_MODE=RUN_EXIT;
	Shutdown();
	goto _exit;
  }

  if  (!CheckPolarsDir()) {
	TCHAR mydir[MAX_PATH];
	TCHAR mes[MAX_PATH];
	StartupStore(_T("... CHECK POLARS DIRECTORY FAILED!%s"),NEWLINE);

	LocalPath(mydir,_T(LKD_POLARS));
	_stprintf(mes,_T("%s"),mydir);
	MessageBoxX(hWndMainWindow, _T("NO POLARS DIRECTORY\nCheck Install"), _T("FATAL ERROR 003"), MB_OK|MB_ICONQUESTION);
	MessageBoxX(hWndMainWindow, mes, _T("NO POLARS DIRECTORY"), MB_OK|MB_ICONQUESTION, true);
	RUN_MODE=RUN_EXIT;
	Shutdown();
	goto _exit;
  }


  // Standby for a system request to close the application during this phase.
  wf->SetTimerNotify(OnTimerNotify);
  #if 1
  srand(GetTickCount());
  #endif

  wf->ShowModal();
  if (RUN_MODE==RUN_SHUTDOWN) goto _exit;

  wp = (WndProperty*)wf->FindByName(TEXT("prpProfile"));
  if (wp) {
	DataFieldFileReader* dfe;
	dfe = (DataFieldFileReader*)wp->GetDataField();

	if (RUN_MODE==RUN_PROFILE) {
		if (_tcslen(dfe->GetPathFile())>0) {
			if (_tcscmp(dfe->GetPathFile(),startProfileFile) ) { // if they are not the same
				_tcscpy(startProfileFile,dfe->GetPathFile());
				if (_tcscmp(startProfileFile,_T("PROFILE_RESET"))==0) {
					#if TESTBENCH
					StartupStore(_T("... Selected FULL RESET virtual profile\n"));
					#endif
					if (MessageBoxX(NULL, gettext(TEXT("_@M1758_")), 
						gettext(TEXT("_@M1757_")), MB_OK|MB_ICONQUESTION));
					fullresetasked=true;
				} else {
					#if TESTBENCH
					StartupStore(_T("... Selected new profile, preloading..\n"));
					#endif
					LKProfileLoad(startProfileFile);
					fullresetasked=false;
				}
			}
		}
	}
	if (RUN_MODE==RUN_AIRCRAFT) {
		if (_tcslen(dfe->GetPathFile())>0) {
			if (_tcscmp(dfe->GetPathFile(),startAircraftFile) ) { // if they are not the same
				_tcscpy(startAircraftFile,dfe->GetPathFile());
				#if TESTBENCH
				StartupStore(_T("... Selected new aircraft, preloading..\n"));
				#endif
				LKProfileLoad(startAircraftFile);
			}
		}
	}
	if (RUN_MODE==RUN_DEVICE) {
		if (_tcslen(dfe->GetPathFile())>0) {
			if (_tcscmp(dfe->GetPathFile(),startDeviceFile) ) { // if they are not the same
				_tcscpy(startDeviceFile,dfe->GetPathFile());
				#if TESTBENCH
				StartupStore(_T("... Selected new device, preloading..\n"));
				#endif
				LKProfileLoad(startDeviceFile);
			}
		}
	}
	if (RUN_MODE==RUN_PILOT) {
		if (_tcslen(dfe->GetPathFile())>0) {
			if (_tcscmp(dfe->GetPathFile(),startPilotFile) ) { // if they are not the same
				_tcscpy(startPilotFile,dfe->GetPathFile());
				#if TESTBENCH
				StartupStore(_T("... Selected new pilot, preloading..\n"));
				#endif
				LKProfileLoad(startPilotFile);
			}
		}
	}
	RUN_MODE=RUN_DUALPROF;
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


_exit:
  if (wf!=NULL) {
	delete wf;
	wf = NULL;
  }

  if (RUN_MODE==RUN_FLY || RUN_MODE==RUN_SIM) {
	if (EnableSoundModes) LKSound(_T("LK_SLIDE.WAV"));
	return 0;	// do not repeat dialog
  }

  if (RUN_MODE==RUN_EXIT || RUN_MODE==RUN_SHUTDOWN)
	return -1;	// terminate
  else
	return 1;	// repeat dialog

}

