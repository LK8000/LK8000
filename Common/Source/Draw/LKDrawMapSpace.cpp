/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKInterface.h"
#include "LKObjects.h"
#include "RGB.h"
#include "DoInits.h"
#include "Modeltype.h"
#include "Sideview.h"
#include "Multimap.h"
#include "Sound/Sound.h"

extern void LoadSplash(LKSurface& Surface, const TCHAR *splashfile);

//
// Called by LKDrawLook8000, this is what happens when we change mapspace mode, advancing through types.
// We paint infopages, nearest, tri, etc.etc.
// Normally there is plenty of cpu available because the map is not even calculated.
// This is why we bring to the Draw thread, in the nearest pages case, also calculations.
//
void MapWindow::DrawMapSpace(LKSurface& Surface,  const RECT& rc ) {

  BrushReference hB;

  TextInBoxMode_t TextDisplayMode = {0};
  TCHAR Buffer[LKSIZEBUFFERLARGE*2];
#ifdef DRAWLKSTATUS
  bool dodrawlkstatus=false;
#endif

  static POINT p[10];

  if (MapSpaceMode==MSM_WELCOME) {
	if (INVERTCOLORS)
		hB=LKBrush_Petrol;
	  else
		hB=LKBrush_Mlight;
  } else {
	if (INVERTCOLORS)
	  hB=LKBrush_Mdark;
	else
	  hB=LKBrush_Mlight;

  }
  const auto oldfont = Surface.SelectObject(LKINFOFONT); // save font

  if (MapSpaceMode!=MSM_WELCOME) Surface.FillRect(&rc, hB);

  if (DoInit[MDI_DRAWMAPSPACE]) {
	p[0].x=0; p[0].y=rc.bottom-BottomSize-NIBLSCALE(2); p[1].x=rc.right-1; p[1].y=p[0].y;
	p[2].x=0; p[2].y=0; p[3].x=rc.right-1; p[3].y=0; // 091230 right-1
	p[4].x=0; p[4].y=0; p[5].x=0; p[5].y=rc.bottom-BottomSize-NIBLSCALE(2);
	p[6].x=rc.right-1; p[6].y=0; p[7].x=rc.right-1; p[7].y=rc.bottom-BottomSize-NIBLSCALE(2); // 091230 right-1

//	p[8].x=0; p[8].y=rc.bottom-BottomSize-NIBLSCALE(2); p[9].x=rc.right; p[9].y=p[8].y;

/*
StartupStore(_T("DOINIT DRAWMAPSPACE 21=%d=%d 22=%d=%d 23=%d=%d 24=%d=%d 31=%d=%d 32=%d=%d\n"),
ConfIP[LKMODE_WP][0],ConfIP21,
ConfIP[LKMODE_WP][1],ConfIP22,
ConfIP[LKMODE_WP][2],ConfIP23,
ConfIP[LKMODE_WP][3],ConfIP24,
ConfIP[LKMODE_NAV][0],ConfIP31,
ConfIP[LKMODE_NAV][1],ConfIP32);
*/

	if (MapSpaceMode==MSM_WELCOME) LoadSplash(Surface,_T("LKPROFILE"));
	DoInit[MDI_DRAWMAPSPACE]=false; 
  }

  // Paint borders in green, but only in nearest pages and welcome
  if (MapSpaceMode==MSM_WELCOME || (!IsMultiMap() && MapSpaceMode!=MSM_MAP) )
  {
	  if (INVERTCOLORS) {
		Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p[2], p[3], RGB_GREEN, rc);
		Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p[4], p[5], RGB_GREEN, rc);
		Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p[6], p[7], RGB_GREEN, rc);
		Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p[0], p[1], RGB_GREEN, rc);
	  } else {
		Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p[2], p[3], RGB_DARKGREEN, rc);
		Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p[4], p[5], RGB_DARKGREEN, rc);
		Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p[6], p[7], RGB_DARKGREEN, rc);
		Surface.DrawLine(PEN_SOLID, NIBLSCALE(1), p[0], p[1], RGB_DARKGREEN, rc);
	  }
  }


#ifdef DRAWLKSTATUS 
  if (LKevent==LKEVENT_NEWRUN) dodrawlkstatus=true;
#endif

  // We are entering mapspacemodes with no initial check on configured subpages.
  // Thus we need to ensure that the page is really available, or find the first valid.
  // However, this will prevent direct customkey access to pages!
  // Instead, we do it when we call next page from InfoPageChange
  // if (!ConfIP[ModeIndex][CURTYPE]) NextModeType();
  switch (MapSpaceMode) {
	case MSM_WELCOME:
#if 0
		SetModeType(LKMODE_MAP,MP_MOVING);
		RefreshMap();
		break;
#endif
#if (1)
		if (!DrawInfo.NAVWarning) { 
		static double firsttime=DrawInfo.Time;
		// delayed automatic exit from welcome mode
		if ( DrawInfo.Time > (firsttime+1.0) ) {
			SetModeType(LKMODE_MAP,MP_MOVING);
			LKevent=LKEVENT_NONE;
			LKSound(_T("LK_BEEP1.WAV"));
			RefreshMap();
			break;
		}
		}
#endif
		if(GlobalModelType==MODELTYPE_PNA_MINIMAP)
		{
			SetModeType(LKMODE_MAP,MP_MOVING);
			LKevent=LKEVENT_NONE;
			break;
		}

		DrawWelcome8000(Surface, rc);
		break;
	case MSM_MAPTRK:
		SetSideviewPage(IM_HEADING);
		LKDrawMultimap_Asp(Surface,rc);
		break;
	case MSM_MAPWPT:
		#if 0
		// If there is no destination, force jump to the map
		if (GetOvertargetIndex()<0) {
			SetModeType(LKMODE_MAP,MP_MOVING);
			LKevent=LKEVENT_NONE;
			break;
		}
		#endif
		SetSideviewPage(IM_NEXT_WP);
		LKDrawMultimap_Asp(Surface,rc);
		break;
	case MSM_MAPASP:
		SetSideviewPage(IM_NEAR_AS);
		LKDrawMultimap_Asp(Surface,rc);
		break;
	case MSM_MAPRADAR:
		LKDrawMultimap_Radar(Surface,rc);
		break;
	case MSM_VISUALGLIDE:
		SetSideviewPage(IM_VISUALGLIDE);
		LKDrawMultimap_Asp(Surface,rc);
		break;
	case MSM_MAPTEST:
		LKDrawMultimap_Test(Surface,rc);
		break;
	case MSM_LANDABLE:
	case MSM_NEARTPS:
	case MSM_AIRPORTS:
	case MSM_COMMON:
	case MSM_RECENT:
	case MSM_AIRSPACES:
	case MSM_THERMALS:
	case MSM_TRAFFIC:
		DrawNearest(Surface, rc);
		break;
	case MSM_MAP:
		break;
	case MSM_INFO_THERMAL:
	case MSM_INFO_CRUISE:
	case MSM_INFO_TASK:
	case MSM_INFO_AUX:
	case MSM_INFO_TRI:
	case MSM_INFO_HSI:
	case MSM_INFO_TRF:
	case MSM_INFO_TARGET:
	case MSM_INFO_CONTEST:
		DrawInfoPage(Surface,rc, false);
		break;

  default:
    memset((void*)&TextDisplayMode, 0, sizeof(TextDisplayMode));
    TextDisplayMode.Color = RGB_WHITE;
    TextDisplayMode.NoSetFont = 1; 
    TextDisplayMode.AlligneCenter = 1;
    Surface.SelectObject(LK8TargetFont);
    _stprintf(Buffer,TEXT("MapSpaceMode=%d"),MapSpaceMode);
    TextInBox(Surface, &rc, Buffer, (rc.right-rc.left)/2, NIBLSCALE(50) , 0, &TextDisplayMode, false);
    break;
  }
#ifdef DRAWLKSTATUS
  // no need to clear dodrawlkstatus, it is already reset at each run
  if (dodrawlkstatus) DrawLKStatus(hdc, rc);
#endif
  Surface.SelectObject(oldfont);
}



