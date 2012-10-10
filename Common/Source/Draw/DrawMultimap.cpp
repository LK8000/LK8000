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

extern bool Sonar_IsEnabled;
extern bool ActiveMap_IsEnabled;
extern TCHAR Sideview_szNearAS[];
extern RECT Sideview_TopRect_InUse;



//
// Top-Left Header for multimaps
// some choices to choose one from 
//
void MapWindow::DrawMultimap_Topleft(const HDC hdc, const RECT rci)
{

  TCHAR topleft_txt[10];
  bool noaction=false;
  static unsigned short counter=0;
  #define COUNT_VERBOSE	5	// 5 seconds, normally

  if (LKevent==LKEVENT_NEWRUN) counter=0;

  switch(MapSpaceMode)
  {
	case MSM_MAPTRK:
		if (counter<COUNT_VERBOSE)
			_stprintf(topleft_txt, TEXT("1 TRK"));
		else
			_stprintf(topleft_txt, TEXT("1"));

		break;

	case MSM_MAPWPT:
		if (counter<COUNT_VERBOSE)
			_stprintf(topleft_txt, TEXT("2 WPT"));
		else
			_stprintf(topleft_txt, TEXT("2"));

		break;

	case MSM_MAPASP:
		if (counter<COUNT_VERBOSE)
			_stprintf(topleft_txt, TEXT("3 ASP"));
		else
			_stprintf(topleft_txt, TEXT("3"));

		break;

	case MSM_MAPRADAR:
		if (counter<COUNT_VERBOSE)
			_stprintf(topleft_txt, TEXT("4 RDR"));
		else
			_stprintf(topleft_txt, TEXT("4"));

		break;
	default:
		noaction=true;
		break;
  } 

  if (noaction) return;

  HFONT oldFont = (HFONT)SelectObject(hdc, LK8TargetFont);
 // HFONT  oldFont = (HFONT) SelectObject(hdc, LK8MediumFont);
  HBRUSH oldBrush= (HBRUSH)SelectObject(hdc,LKBrush_Mdark);
  HPEN     oldPen= (HPEN)  SelectObject(hdc, GetStockObject(WHITE_PEN));

  MapWindow::LKWriteText(hdc, topleft_txt, LEFTLIMITER, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_LEFT, RGB_SBLACK, true);

  if (counter<255) counter++;

  SelectObject(hdc,oldBrush);
  SelectObject(hdc,oldPen);
  SelectObject(hdc,oldFont);
}


#define MMCOLOR_ENABLED_FLIP	RGB_LIGHTGREEN
#define MMCOLOR_ENABLED_FLOP	RGB_GREEN
#define MMCOLOR_DISABLED RGB_GREY

void MapWindow::DrawMultimap_Topright(const HDC hdc, const RECT rci) {

  TCHAR topright_txt[10];
  bool noaction=false;
  COLORREF wcolor;
  static bool flip= true;

  flip = !flip;

  switch(MapSpaceMode)
  {
	case MSM_MAPTRK:
	case MSM_MAPWPT:
		_stprintf(topright_txt, MsgToken(2231));
		if(ActiveMap_IsEnabled) {
			if (flip)
				wcolor=MMCOLOR_ENABLED_FLIP;
			else
				wcolor=MMCOLOR_ENABLED_FLOP;
		} else
			wcolor=MMCOLOR_DISABLED;
		break;

	case MSM_MAPASP:
		_stprintf(topright_txt, MsgToken(1293));
		if(Sonar_IsEnabled) {
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

  HFONT  oldFont = (HFONT) SelectObject(hdc, MapWindowFont);
  HBRUSH oldBrush= (HBRUSH)SelectObject(hdc,LKBrush_Mdark);
  HPEN   oldPen= (HPEN)  SelectObject(hdc, GetStockObject(WHITE_PEN));

  LKWriteText(hdc, topright_txt, rci.right-RIGHTLIMITER, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_RIGHT, wcolor, true);

  SelectObject(hdc,oldBrush);
  SelectObject(hdc,oldPen);
  SelectObject(hdc,oldFont);
}



void MapWindow::DrawMultimap_DynaLabel(const HDC hdc, const RECT rci)
{

  HBRUSH oldBrush;
  HFONT oldFont;

  if (INVERTCOLORS)
        oldBrush=(HBRUSH)SelectObject(hdc,LKBrush_Ndark);
  else
        oldBrush=(HBRUSH)SelectObject(hdc,LKBrush_LightGrey);

  extern double fSplitFact;
  SIZE textSize;
  int midsplit=(long)((double)(rci.bottom-rci.top)*fSplitFact);	 // this is ok
  //int midsplit=Sideview_TopRect_InUse.bottom;	// this SHOULD be ok, but in M3 the TopRect is updated 1s late

  oldFont=(HFONT)SelectObject(hdc, LK8UnitFont);

  GetTextExtentPoint(hdc, _T("Y"), 1, &textSize);
  // move the label on top view when the topview window is big enough
  if (fSplitFact >0.5)
        midsplit-=textSize.cy;
  if (fSplitFact <0.5)
        midsplit+=textSize.cy;


  if(GetSideviewPage()== IM_NEAR_AS)
  {
        TCHAR topcenter_txt[80];
        _stprintf(topcenter_txt, TEXT("%s"), Sideview_szNearAS );

        MapWindow::LKWriteBoxedText(hdc,&MapRect,topcenter_txt, rci.right/3, midsplit, 0, WTALIGN_CENTER, RGB_WHITE, RGB_BLACK);
  }

  SelectObject(hdc,oldBrush);
  SelectObject(hdc,oldFont);

}


