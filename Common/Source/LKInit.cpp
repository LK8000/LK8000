/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "StdAfx.h"
#include <stdio.h>
#include "externs.h"

#ifdef PNA
#include "Modeltype.h"
#include "LKHolux.h"
#include "LKRoyaltek3200.h"
#endif

using std::min;
using std::max;

extern void ResetNearestTopology();

// InitLKScreen can be called anytime, and should be called upon screen changed from portrait to landscape,
// or windows size is changed for any reason. We dont support dynamic resize of windows, though, because each
// resolution has its own tuned settings. This is thought for real devices, not for PC emulations.
// Attention: after InitLKScreen, also InitLKFonts should be called. 
void InitLKScreen() {

#if (WINDOWSPC>0)
  int iWidth=SCREENWIDTH;
  int iHeight=SCREENHEIGHT;
#else
  int iWidth=GetSystemMetrics(SM_CXSCREEN);
  int iHeight=GetSystemMetrics(SM_CYSCREEN);
#endif

  ScreenSizeX=iWidth;
  ScreenSizeY=iHeight;
  ScreenSizeR.top=0;
  ScreenSizeR.bottom=iHeight-1;
  ScreenSizeR.left=0;
  ScreenSizeR.right=iWidth-1;

  int maxsize=0;
  int minsize=0;
  maxsize = max(ScreenSizeR.right-ScreenSizeR.left+1,ScreenSizeR.bottom-ScreenSizeR.top+1);
  minsize = min(ScreenSizeR.right-ScreenSizeR.left+1,ScreenSizeR.bottom-ScreenSizeR.top+1);

  ScreenDScale = max(1.0,minsize/240.0); // always start w/ shortest dimension

  if (maxsize == minsize) 
  {
    ScreenDScale *= 240.0 / 320.0;
  }

  ScreenScale = (int)ScreenDScale;

  #if (WINDOWSPC>0)
  if (maxsize==720) {
        ScreenScale=2; // force rescaling with Stretch
  }
  #endif

  if ( ((double)ScreenScale) == ScreenDScale)
	ScreenIntScale = true;
  else
	ScreenIntScale = false;

  int i;
  if ( ScreenIntScale ) {
        for (i=0; i<=MAXIBLSCALE; i++) LKIBLSCALE[i]=(int)(i*ScreenScale);
  } else {
        for (i=0; i<=MAXIBLSCALE;i++) LKIBLSCALE[i]=(int)(i*ScreenDScale);
  }

  // StartupStore(_T("...... ScreenScale=%d ScreenDScale=%.3f ScreenIntScale=%d\n"),ScreenScale,ScreenDScale,ScreenIntScale);

  ScreenSize=0;

  if (iWidth == 240 && iHeight == 320) ScreenSize=(ScreenSize_t)ss240x320; // QVGA      portrait
  if (iWidth == 234 && iHeight == 320) ScreenSize=(ScreenSize_t)ss240x320; // use the same config of 240x320
  if (iWidth == 272 && iHeight == 480) ScreenSize=(ScreenSize_t)ss272x480;
  if (iWidth == 240 && iHeight == 400) ScreenSize=(ScreenSize_t)ss240x320; //           portrait

  if (iWidth == 480 && iHeight == 640) ScreenSize=(ScreenSize_t)ss480x640; //  VGA
  if (iWidth == 640 && iHeight == 480) ScreenSize=(ScreenSize_t)ss640x480; //   VGA
  if (iWidth == 320 && iHeight == 240) ScreenSize=(ScreenSize_t)ss320x240; //  QVGA
  if (iWidth == 320 && iHeight == 234) ScreenSize=(ScreenSize_t)ss320x240; //  QVGA
  if (iWidth == 720 && iHeight == 408) ScreenSize=(ScreenSize_t)ss720x408;
  if (iWidth == 480 && iHeight == 800) ScreenSize=(ScreenSize_t)ss480x800;
  if (iWidth == 400 && iHeight == 240) ScreenSize=(ScreenSize_t)ss400x240; // landscape
  if (iWidth == 480 && iHeight == 272) ScreenSize=(ScreenSize_t)ss480x272; // WQVGA     landscape
  if (iWidth == 480 && iHeight == 234) ScreenSize=(ScreenSize_t)ss480x234; //   iGo
  if (iWidth == 800 && iHeight == 480) ScreenSize=(ScreenSize_t)ss800x480; //  WVGA
  if (iWidth == 896 && iHeight == 672) ScreenSize=(ScreenSize_t)ss896x672; //  PC version only

  TCHAR tbuf[80];
  if (ScreenSize==0) {
        wsprintf(tbuf,_T(". InitLKScreen: ++++++ ERROR UNKNOWN RESOLUTION %dx%d !%s"),iWidth,iHeight,NEWLINE); // 091119
        StartupStore(tbuf);
  } else {
        wsprintf(tbuf,_T(". InitLKScreen: %dx%d%s"),iWidth,iHeight,NEWLINE); // 091213
        StartupStore(tbuf);
  }

  if (ScreenSize > (ScreenSize_t)sslandscape) 
	ScreenLandscape=true;
  else
	ScreenLandscape=false;

  // By default, h=v=size/6 and here we set it better
  switch (ScreenSize) { 
	case (ScreenSize_t)ss800x480:
		GestureSize=50;
		LKVarioSize=50;
		// dscale=480/240=2  800/dscale=400 -(70+2+2)=  326 x dscale = 652
		LKwdlgConfig=652;
		break;
	case (ScreenSize_t)ss400x240:
		GestureSize=50;
		LKVarioSize=25;
		// dscale=240/240=1  400/dscale=400 -(70+2+2)=  326 x dscale = 326
		LKwdlgConfig=326;
		break;
	case (ScreenSize_t)ss640x480:
		GestureSize=50;
		LKVarioSize=40;
		// dscale=480/240=2  640/dscale=320 -(70+2+2)=  246 x dscale = 492
		LKwdlgConfig=492;
		break;
	case (ScreenSize_t)ss896x672:
		GestureSize=50;
		LKVarioSize=56;
		// dscale=672/240=2.8  896/dscale=320 -(70+2+2)=  246 x dscale = 689
		LKwdlgConfig=689;
		break;
	case (ScreenSize_t)ss480x272:
		GestureSize=50;
		LKVarioSize=30;
		// dscale=272/240=1.133  480/dscale=424 -(70+2+2)=  350 x dscale = 397
		LKwdlgConfig=395;
		break;
	case (ScreenSize_t)ss720x408:
		GestureSize=50;
		LKVarioSize=45;
		// dscale=408/240=1.133  720/dscale=423 -(70+2+2)=  350 x dscale = 594
		LKwdlgConfig=594;
		break;
	case (ScreenSize_t)ss480x234:
		GestureSize=50;
		LKVarioSize=30;
		// dscale=234/240=0.975  480/dscale=492 -(70+2+2)=  418 x dscale = 407
		LKwdlgConfig=405;
		break;
	case (ScreenSize_t)ss320x240:
		GestureSize=50;
		LKVarioSize=20;
		// dscale=240/240=1  320/dscale=320 -(70+2+2)=  246 x dscale = 246
		// but 246 is too long..
		LKwdlgConfig=244;
		break;
	// PORTRAIT MODES
	case (ScreenSize_t)ss480x640:
		GestureSize=50;
		LKVarioSize=30;
		break;
	case (ScreenSize_t)ss480x800:
		GestureSize=50;
		LKVarioSize=30;
		// dscale=240/240=1  400/dscale=400 -(70+2+2)=  326 x dscale = 326
		LKwdlgConfig=324;
		break;
	case (ScreenSize_t)ss240x320:
		GestureSize=50;
		LKVarioSize=13;
		break;
	default:
		GestureSize=50;
		LKVarioSize=30;
		break;
  }
}


//
// This is called by lk8000.cpp on init, only once
//
void InitCustomHardware(void) {

  #ifdef PNA
  if (GlobalModelType == MODELTYPE_PNA_FUNTREK) {
	Init_GM130();
	// if (!DeviceIsGM130) return;
	// todo set to General devicetype if Init failed
  }
  if (GlobalModelType == MODELTYPE_PNA_ROYALTEK3200) {
	Init_Royaltek3200();
  }
  #endif

  return;
}

//
// This is called by lk8000.cpp on exit, only once
//
void DeInitCustomHardware(void) {

  #ifdef PNA
  if (DeviceIsGM130) DeInit_GM130();
  if (DeviceIsRoyaltek3200) DeInit_Royaltek3200();
  #endif

  return;
}



void UpdateConfBB(void) {

  ConfBB[0]=true; // thermal always on automatically
  ConfBB[1]=ConfBB1;
  ConfBB[2]=ConfBB2;
  ConfBB[3]=ConfBB3;
  ConfBB[4]=ConfBB4;
  ConfBB[5]=ConfBB5;
  ConfBB[6]=ConfBB6;
  ConfBB[7]=ConfBB7;
  ConfBB[8]=ConfBB8;
  ConfBB[9]=ConfBB9;

  if (ConfBB2==false && ConfBB3==false &&
      ConfBB4==false && ConfBB5==false &&
      ConfBB6==false && ConfBB7==false &&
      ConfBB8==false && ConfBB9==false)

		// we need at least one bottom bar stripe available (thermal apart)
		ConfBB[1]=true;

}

void UpdateConfIP(void) {

  // MAP MODE always available
  ConfIP[0][0]=true; 
  ConfIP[0][1]=true; 
  ConfMP[0]=true; // map mode

  // LKMODE_INFOMODE is 1
  ConfIP[1][0]=ConfIP11;
  ConfIP[1][1]=ConfIP12;
  ConfIP[1][2]=ConfIP13;
  ConfIP[1][3]=ConfIP14;
  ConfIP[1][4]=ConfIP15;
  ConfIP[1][5]=ConfIP16;

  // WPMODE
  ConfIP[2][0]=ConfIP21;
  ConfIP[2][1]=ConfIP22;
  ConfIP[2][2]=ConfIP23;
  ConfIP[2][3]=ConfIP24;

  // COMMONS
  ConfIP[3][0]=ConfIP31;
  ConfIP[3][1]=ConfIP32;
  ConfIP[3][2]=ConfIP33;

  // TRAFFIC always on if available
  ConfIP[4][0]=true;
  ConfIP[4][1]=true;
  ConfIP[4][2]=true;
  ConfMP[4]=true; // traffic mode

  // Check if we have INFOMODE
  if (ConfIP[1][0]==false && ConfIP[1][1]==false 
	&& ConfIP[1][2]==false && ConfIP[1][3]==false 
	&& ConfIP[1][4]==false && ConfIP[1][5]==false) {
	ConfMP[1]=false;
  } else
	ConfMP[1]=true;

  // Check if we have NEAREST pages
  if (ConfIP[2][0]==false && ConfIP[2][1]==false 
	&& ConfIP[2][2]==false && ConfIP[2][3]==false ) {
	ConfMP[2]=false;
  } else
	ConfMP[2]=true;

  // Check if we have COMMONS
  if (ConfIP[3][0]==false && ConfIP[3][1]==false && ConfIP[3][2]==false ) {
	ConfMP[3]=false;
  } else
	ConfMP[3]=true;

  /*
  // Verify that we have at least one menu
  if (ConfMP[1]==false && ConfMP[2]==false && ConfMP[3]==false ) {
	ConfIP[1][0]=true;
	ConfMP[1]=true;
  }
  */
  SetInitialModeTypes();

}

void SetInitialModeTypes(void) {

  // Update the initial values for each mapspace, keeping the first valid value. We search backwards.
  // INFOMODE 1  
  if (ConfIP[LKMODE_INFOMODE][IM_TRI]) ModeType[LKMODE_INFOMODE]=IM_TRI;
  if (ConfIP[LKMODE_INFOMODE][IM_CONTEST]) ModeType[LKMODE_INFOMODE]=IM_CONTEST;
  if (ConfIP[LKMODE_INFOMODE][IM_AUX]) ModeType[LKMODE_INFOMODE]=IM_AUX;
  if (ConfIP[LKMODE_INFOMODE][IM_TASK]) ModeType[LKMODE_INFOMODE]=IM_TASK;
  if (ConfIP[LKMODE_INFOMODE][IM_THERMAL]) ModeType[LKMODE_INFOMODE]=IM_THERMAL;
  if (ConfIP[LKMODE_INFOMODE][IM_CRUISE]) ModeType[LKMODE_INFOMODE]=IM_CRUISE;

  // WP NEAREST MODE 2  
  if (ConfIP[LKMODE_WP][WP_NEARTPS]) ModeType[LKMODE_WP]=WP_NEARTPS;
  if (ConfIP[LKMODE_WP][WP_LANDABLE]) ModeType[LKMODE_WP]=WP_LANDABLE;
  if (ConfIP[LKMODE_WP][WP_AIRPORTS]) ModeType[LKMODE_WP]=WP_AIRPORTS;

  // COMMONS MODE 3
  if (ConfIP[LKMODE_NAV][NV_HISTORY]) ModeType[LKMODE_WP]=NV_HISTORY;
  if (ConfIP[LKMODE_NAV][NV_COMMONS]) ModeType[LKMODE_WP]=NV_COMMONS;


}


// Requires restart if activated from config menu
void InitLK8000() 
{
        #if TESTBENCH
	StartupStore(_T(". Init LK8000%s"),NEWLINE);
        #endif
	LoadRecentList();

	InitModeTable();
	ResetNearestTopology();
}


// Conversion between submenus and global mapspace modes 
// Basic initialization of global variables and parameters.
//
void InitModeTable() {

	short i,j;
	#if TESTBENCH
	StartupStore(_T(". Init ModeTable for LK8000: "));
	#endif

	for (i=0; i<=LKMODE_TOP; i++)
		for (j=0; j<=MSM_TOP; j++)
			ModeTable[i][j]=INVALID_VALUE;


	// this table is for submenus, order is not important
	ModeTable[LKMODE_MAP][MP_WELCOME]	=	MSM_WELCOME;
	ModeTable[LKMODE_MAP][MP_MOVING]	=	MSM_MAP;

	ModeTable[LKMODE_WP][WP_AIRPORTS]	=	MSM_AIRPORTS;
	ModeTable[LKMODE_WP][WP_LANDABLE]	=	MSM_LANDABLE;
	ModeTable[LKMODE_WP][WP_NEARTPS]	=	MSM_NEARTPS;
	ModeTable[LKMODE_WP][WP_AIRSPACES]	=	MSM_AIRSPACES;

	ModeTable[LKMODE_INFOMODE][IM_CRUISE]	=	MSM_INFO_CRUISE;
	ModeTable[LKMODE_INFOMODE][IM_THERMAL]	=	MSM_INFO_THERMAL;
	ModeTable[LKMODE_INFOMODE][IM_TASK]	=	MSM_INFO_TASK;
	ModeTable[LKMODE_INFOMODE][IM_AUX]	=	MSM_INFO_AUX;
	ModeTable[LKMODE_INFOMODE][IM_CONTEST]	=	MSM_INFO_CONTEST;
	ModeTable[LKMODE_INFOMODE][IM_TRI]	=	MSM_INFO_TRI;

	ModeTable[LKMODE_NAV][NV_COMMONS]	=	MSM_COMMON;
	ModeTable[LKMODE_NAV][NV_HISTORY]	=	MSM_RECENT;
	ModeTable[LKMODE_NAV][NV_THERMALS]	=	MSM_THERMALS;

	ModeTable[LKMODE_TRF][TF_LIST]		=	MSM_TRAFFIC;
	ModeTable[LKMODE_TRF][IM_TRF]		=	MSM_INFO_TRF;
	ModeTable[LKMODE_TRF][IM_TARGET]	=	MSM_INFO_TARGET;

	// startup mode
	ModeIndex=LKMODE_MAP;
	// startup values for each mode. we shall update these defaults using current profile settings
	// for ConfIP real values. 
	ModeType[LKMODE_MAP]	=	MP_WELCOME;
	ModeType[LKMODE_INFOMODE]=	IM_CRUISE;
	ModeType[LKMODE_WP]	=	WP_AIRPORTS;
	ModeType[LKMODE_NAV]	=	NV_COMMONS;
	ModeType[LKMODE_TRF]	=	TF_LIST;

	ModeTableTop[LKMODE_MAP]=MP_TOP;
	ModeTableTop[LKMODE_WP]=WP_TOP;
	ModeTableTop[LKMODE_INFOMODE]=IM_TOP;
	ModeTableTop[LKMODE_NAV]=NV_TOP;
	ModeTableTop[LKMODE_TRF]=TF_TOP;

	// set all sorting type to distance (default) even for unconventional modes just to be sure
	for (i=0; i<=MSM_TOP; i++) SortedMode[i]=1;
	SortedMode[MSM_AIRSPACES]=2; // Airspaces have a different layout

	for (i=0; i<MAXNEAREST;i++) {
		SortedTurnpointIndex[i]=-1;
		SortedLandableIndex[i]=-1;
		SortedAirportIndex[i]=-1;
	}

	for (i=0; i<MAXCOMMON; i++) 
		CommonIndex[i]= -1;

	for (i=0; i<=MSM_TOP; i++) {
		SortBoxY[i]=0;
		for (j=0; j<=MAXSORTBOXES; j++) SortBoxX[i][j]=0;
	}

	SetInitialModeTypes();
	#if TESTBENCH
	StartupStore(_T("Ok%s"),NEWLINE);
	#endif
}


