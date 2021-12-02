/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
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
#include "LoadSplash.h"
#include "Asset.hpp"

//
// Called by LKDrawLook8000, this is what happens when we change mapspace mode, advancing through types.
// We paint infopages, nearest, tri, etc.etc.
// Normally there is plenty of cpu available because the map is not even calculated.
// This is why we bring to the Draw thread, in the nearest pages case, also calculations.
//
void MapWindow::DrawMapSpace(LKSurface& Surface,  const RECT& rc) {

  BrushReference hB;

  TextInBoxMode_t TextDisplayMode = {};
  TCHAR Buffer[LKSIZEBUFFERLARGE*2];

  if (!IsDithered()) {
    if (MapSpaceMode == MSM_WELCOME) {
      if (INVERTCOLORS)
        hB = LKBrush_Petrol;
      else
        hB = LKBrush_Mlight;
    } else {
      if (INVERTCOLORS)
        hB = LKBrush_Mdark;
      else
        hB = LKBrush_Mlight;

    }
  } else {
    if (INVERTCOLORS)
      hB = LKBrush_Black;
    else
      hB = LKBrush_White;
  }

  const auto oldfont = Surface.SelectObject(LKINFOFONT); // save font
  if (MapSpaceMode==MSM_WELCOME) {
      LKBitmap WelcomeBitmap = LoadSplash(_T("LKPROFILE"));
      if(WelcomeBitmap) {
          DrawSplash(Surface, rc, WelcomeBitmap);
      }
  } else {
      Surface.FillRect(&rc, hB);
  }

  // Paint borders in green, but only in nearest pages and welcome
  if (MapSpaceMode == MSM_WELCOME || (!IsMultiMap() && MapSpaceMode != MSM_MAP)) {
    LKColor color = INVERTCOLORS ? RGB_GREEN : RGB_DARKGREEN;;
    if (IsDithered()) {
      color = INVERTCOLORS ? RGB_WHITE : RGB_BLACK;
    }
    LKPen BorderPen(PEN_SOLID, ScreenThinSize, color);
    auto OldPen = Surface.SelectObject(BorderPen);
    auto OldBrush = Surface.SelectObject(LK_HOLLOW_BRUSH);

    Surface.Rectangle(rc.left, rc.top, rc.right, rc.bottom - BottomSize);

    Surface.SelectObject(OldPen);
    Surface.SelectObject(OldBrush);
  }


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
		if (!DrawInfo.NAVWarning) {
		   static double firsttime=DrawInfo.Time;
		   // delayed automatic exit from welcome mode
		   if ( DrawInfo.Time > (firsttime+3.0) ) {
                      SetModeType(LKMODE_MAP,MP_MOVING);
                      LKevent=LKEVENT_NONE;
                      LKSound(_T("LK_BEEP1.WAV"));
                      RefreshMap();
                      break;
                   }
                }
		if(GlobalModelType==ModelType::LX_MINI_MAP)
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
    TextInBox(Surface, &rc, Buffer, (rc.right+rc.left)/2, NIBLSCALE(50) , &TextDisplayMode, false);
    break;
  }
  Surface.SelectObject(oldfont);
}
