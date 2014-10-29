/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "LKMapWindow.h"
#include "LKObjects.h"
#include "RGB.h"
#include "Sideview.h"
#include "Multimap.h"
#include "LKInterface.h"
#include "McReady.h"

extern TCHAR Sideview_szNearAS[];


//
// Top-Left Header for multimaps
// some choices to choose one from 
//
void MapWindow::DrawMultimap_Topleft(LKSurface& Surface, const RECT& rci)
{

  TCHAR topleft_txt[80];
  bool noaction=false;
  static unsigned short counter=0;
  #define COUNT_VERBOSE	1	// How many seconds we display the page indicator

  if (LKevent==LKEVENT_NEWRUN) counter=0;

  switch(MapSpaceMode)
  {
	case MSM_MAPTRK:
		if (counter<COUNT_VERBOSE)
			_stprintf(topleft_txt, TEXT("1 %s"),MsgToken(1287)); // Heading
		else
			//_stprintf(topleft_txt, TEXT("1"));
			noaction=true;

		break;

	case MSM_MAPWPT:
		if (counter<COUNT_VERBOSE)
			_stprintf(topleft_txt, TEXT("2 %s"),MsgToken(1289)); // Next WP
		else
			//_stprintf(topleft_txt, TEXT("2"));
			noaction=true;

		break;

	case MSM_MAPASP:
		if (counter<COUNT_VERBOSE)
			_stprintf(topleft_txt, TEXT("3 %s"),MsgToken(1292)); // Nearest airspace
		else
			//_stprintf(topleft_txt, TEXT("3"));
			noaction=true;

		break;

	case MSM_MAPRADAR:
		if (counter<COUNT_VERBOSE)
			_stprintf(topleft_txt, TEXT("4 FLARM Radar"));
		else
			//_stprintf(topleft_txt, TEXT("4"));
			noaction=true;

		break;
	default:
		noaction=true;
		break;
  } 

  if (noaction) return;

  LKFont oldFont = Surface.SelectObject(LK8TargetFont);
  LKBrush oldBrush= Surface.SelectObject(LKBrush_Mdark);
  LKPen oldPen = Surface.SelectObject(LK_WHITE_PEN);

  MapWindow::LKWriteText(Surface, topleft_txt, LEFTLIMITER, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_LEFT, RGB_SBLACK, true);

  if (counter<255) counter++;

  Surface.SelectObject(oldBrush);
  Surface.SelectObject(oldPen);
  Surface.SelectObject(oldFont);
}


#define MMCOLOR_ENABLED_FLIP	RGB_LIGHTGREEN
#define MMCOLOR_ENABLED_FLOP	RGB_GREEN
#define MMCOLOR_DISABLED RGB_AMBER

void MapWindow::DrawMultimap_Topright(LKSurface& Surface, const RECT& rci) {

  TCHAR topright_txt[80];
  bool noaction=false;
  LKColor wcolor;
  static bool flip= true;

  flip = !flip;

  switch(MapSpaceMode)
  {
	case MSM_MAPTRK:
	case MSM_MAPWPT:
		//
		// If there is no topview, there is also no ACTIVE choice!
		//
		if (Current_Multimap_TopRect.bottom==0) return;
		_stprintf(topright_txt, MsgToken(2231));
			wcolor=MMCOLOR_DISABLED;
		break;

	case MSM_MAPASP:
		_stprintf(topright_txt, MsgToken(1293));
		if(SonarWarning) {
			if (flip)
				wcolor=MMCOLOR_ENABLED_FLIP;
			else
				wcolor=MMCOLOR_ENABLED_FLOP;
		} else
			wcolor=MMCOLOR_DISABLED;
		break;

	case MSM_MAPRADAR:
		noaction=true;

		break;
	default:
		noaction=true;
		break;
  } 

  if (noaction) return;

  LKFont  oldFont = Surface.SelectObject(MapWindowFont);
  LKBrush oldBrush= Surface.SelectObject(LKBrush_Mdark);
  LKPen oldPen = Surface.SelectObject(LK_WHITE_PEN);

  LKWriteText(Surface, topright_txt, rci.right-RIGHTLIMITER, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_RIGHT, wcolor, true);

  Surface.SelectObject(oldBrush);
  Surface.SelectObject(oldPen);
  Surface.SelectObject(oldFont);
}



void MapWindow::DrawMultimap_DynaLabel(LKSurface& Surface, const RECT& rci)
{
#if 0
  if (!IsMultimapOverlays()) return;

  LKBrush oldBrush;
  LKFont oldFont;

  if (INVERTCOLORS)
        oldBrush=Surface.SelectObject(LKBrush_Ndark);
  else
        oldBrush=Surface.SelectObject(LKBrush_LightGrey);

  extern double fSplitFact;
  SIZE textSize;
  int midsplit=(long)((double)(rci.bottom-rci.top)*fSplitFact);	 // this is ok
  //int midsplit=Current_Multimap_TopRect.bottom;	// this SHOULD be ok, but in M3 the TopRect is updated 1s late

  //oldFont=(HFONT)Surface.SelectObject(LK8UnitFont);
  oldFont=(HFONT)Surface.SelectObject(LK8PanelSmallFont);

  GetTextExtentPoint(hdc, _T("Y"), 1, &textSize);
  // move the label on top view when the topview window is big enough
  if (fSplitFact >0.5)
        midsplit-=textSize.cy;
  if (fSplitFact <0.5)
        midsplit+=textSize.cy;


  if(MapSpaceMode==MSM_MAPASP)
  {
        TCHAR topcenter_txt[80];
        _stprintf(topcenter_txt, TEXT("%s"), Sideview_szNearAS );

	LKPen oldpen=(HPEN)SelectObject(hdc,LKPen_White_N1);

        MapWindow::LKWriteBoxedText(hdc,&MapRect,topcenter_txt, rci.right/3, midsplit, 0, WTALIGN_CENTER, RGB_WHITE, RGB_BLACK);

	SelectObject(hdc,oldpen);
	goto _return;
  }

#if 0
  if(MapSpaceMode==MSM_MAPWPT || MapSpaceMode==MSM_MAPTRK)
  {
        TCHAR topcenter_txt[80];
	int index=GetOvertargetIndex();
	if (index<0) goto _return;

	TCHAR wpname[NAME_SIZE+1];
	TCHAR BufferValue[10];
	TCHAR BufferUnit[10];

	//
	// Waypoint name, bearing difference and distance
	//
	LK_tcsncpy(wpname, WayPointList[index].Name, 8);
	LKFormatBrgDiff(index, false, BufferValue, BufferUnit);
        _stprintf(topcenter_txt, TEXT("%s   %s   "),wpname,BufferValue);
	LKFormatDist(index, false, BufferValue, BufferUnit);
	_tcscat(topcenter_txt,BufferValue);
	_tcscat(topcenter_txt,BufferUnit);

	LKPen oldpen=(HPEN)SelectObject(hdc,LKPen_White_N1);
        MapWindow::LKWriteBoxedText(hdc,&MapRect,topcenter_txt, rci.right/4+NIBLSCALE(4), midsplit, 0, WTALIGN_CENTER, RGB_WHITE, RGB_BLACK);

	//
	// Efficiency required and Arrival altitude
	//
	_tcscpy(topcenter_txt,_T(""));
	LKFormatGR(index, false, BufferValue, BufferUnit);
	_tcscat(topcenter_txt,BufferValue);
	_tcscat(topcenter_txt,_T("   "));
	LKFormatAltDiff(index, false, BufferValue, BufferUnit);
	_tcscat(topcenter_txt,BufferValue);
	_tcscat(topcenter_txt,BufferUnit);

	if (WayPointCalc[index].AltArriv[AltArrivMode]<0) {
		SelectObject(hdc,LKPen_Red_N1);
	} else {
		// Reachable is not available for virtual wps, to be investigated..
		// So we dont paint green or red for them, when arrival is positive, because
		// we dont know if there are obstacles.
		if (index>RESWP_END) {
			if (WayPointList[index].Reachable )
				SelectObject(hdc,LKPen_Green_N1);
			else
				SelectObject(hdc,LKPen_Red_N1);
		}
	}

	extern bool IsSafetyMacCreadyInUse(int val);
        if (IsSafetyMacCreadyInUse(index)) {
                _stprintf(BufferValue,_T("   @%.1f"),GlidePolar::SafetyMacCready*LIFTMODIFY);
        } else {
                _stprintf(BufferValue,_T("   @%.1f"),MACCREADY);
	}
// TODO: topcenter never reset here?
	_tcscat(topcenter_txt,BufferValue);

        MapWindow::LKWriteBoxedText(hdc,&MapRect,topcenter_txt, rci.right-rci.right/4, midsplit, 0, WTALIGN_CENTER, RGB_WHITE, RGB_BLACK);

	SelectObject(hdc,oldpen);
	//goto _return;
  }
#endif // experimental "eyes style" overlays

_return:
  SelectObject(hdc,oldBrush);
  SelectObject(hdc,oldFont);
#endif // NOT IN NEWMULTIMAPS
}

//
// Simple thin black line separator between side and topview, if needed
//
void MapWindow::DrawMultimap_SideTopSeparator(LKSurface& Surface, const RECT& rci) {
  if ( DrawRect.bottom>0 && Current_Multimap_SizeY<SIZE4 ) {
	POINT line[2];
	line[0].x = rci.left;
	line[1].x = rci.right;
	line[0].y=DrawRect.bottom;
	line[1].y=DrawRect.bottom;
        Surface.DrawLine(PEN_SOLID, 1, line[0], line[1], RGB_BLACK,rci);
  }
}

