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

extern void LoadSplash(HDC hDC, TCHAR *splashfile);
extern void LKDrawMultimap_Asp(HDC hdc,RECT rc);

//
// Called by LKDrawLook8000, this is what happens when there is no map to be drawn:
// we paint infopages, nearest, tri, etc.etc.
// Normally there is plenty of cpu available because the map is not even calculated.
// This is why we bring to the Draw thread, in the nearest pages case, also calculations.
//
void MapWindow::DrawMapSpace(HDC hdc,  RECT rc ) {

  HFONT oldfont;
  HBRUSH hB;

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
  oldfont = (HFONT)SelectObject(hdc, LKINFOFONT); // save font

  if (MapSpaceMode!=MSM_WELCOME) FillRect(hdc,&rc, hB); 

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

	if (MapSpaceMode==MSM_WELCOME) LoadSplash(hdc,_T("LKPROFILE"));
	DoInit[MDI_DRAWMAPSPACE]=false; 
  }

  // Paint borders in green, but not on white pages
  // Currently only RADAR is a whitepage
//  if (MapSpaceMode!=MSM_RADAR)
  {
	  if (INVERTCOLORS) {
		_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p[2], p[3], RGB_GREEN, rc);
		_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p[4], p[5], RGB_GREEN, rc);
		_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p[6], p[7], RGB_GREEN, rc);
		_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p[0], p[1], RGB_GREEN, rc);
	  } else {
		_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p[2], p[3], RGB_DARKGREEN, rc);
		_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p[4], p[5], RGB_DARKGREEN, rc);
		_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p[6], p[7], RGB_DARKGREEN, rc);
		_DrawLine(hdc, PS_SOLID, NIBLSCALE(1), p[0], p[1], RGB_DARKGREEN, rc);
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
RECT frc = rc;
  switch (MapSpaceMode) {
	case MSM_WELCOME:
#if (1)
		if (!DrawInfo.NAVWarning) { 
		static double firsttime=DrawInfo.Time;
		// delayed automatic exit from welcome mode
		if ( DrawInfo.Time > (firsttime+1.0) ) {
			SetModeType(LKMODE_MAP,MP_MOVING);
			LKevent=LKEVENT_NONE;
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

		DrawWelcome8000(hdc, rc);
		break;
	case MSM_MAPASP:
		LKDrawMultimap_Asp(hdc,rc);
		break;
	case MSM_MAPRADAR:
		LKDrawMultimap_Radar(hdc,rc);
		break;
	case MSM_MAPTEST:
		LKDrawMultimap_Test(hdc,rc);
		break;
	case MSM_LANDABLE:
	case MSM_NEARTPS:
	case MSM_AIRPORTS:
		DrawNearest(hdc, rc);
		break;
	case MSM_AIRSPACES:
		DrawAspNearest(hdc, rc);
		break;
	case MSM_COMMON:
	case MSM_RECENT:
		DrawCommon(hdc, rc);
		break;
	case MSM_MAP:
		break;
	case MSM_INFO_THERMAL:
	case MSM_INFO_CRUISE:
	case MSM_INFO_TASK:
	case MSM_INFO_AUX:
	case MSM_INFO_TRI:
	case MSM_INFO_TRF:
	case MSM_INFO_TARGET:
	case MSM_INFO_CONTEST:
		DrawInfoPage(hdc,rc, false);
		break;
	case MSM_TRAFFIC:
		DrawTraffic(hdc,rc);
		break;
	case MSM_THERMALS:
		DrawThermalHistory(hdc,rc);
		break;

  default:
    memset((void*)&TextDisplayMode, 0, sizeof(TextDisplayMode));
    TextDisplayMode.Color = RGB_WHITE;
    TextDisplayMode.NoSetFont = 1; 
    TextDisplayMode.AlligneCenter = 1;
    SelectObject(hdc, LK8TargetFont);
    _stprintf(Buffer,TEXT("MapSpaceMode=%d"),MapSpaceMode);
    TextInBox(hdc, Buffer, (rc.right-rc.left)/2, NIBLSCALE(50) , 0, &TextDisplayMode, false);
    break;
  }
#ifdef DRAWLKSTATUS
  // no need to clear dodrawlkstatus, it is already reset at each run
  if (dodrawlkstatus) DrawLKStatus(hdc, rc);
#endif
  SelectObject(hdc, oldfont); 
}



