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
extern TCHAR Sideview_szNearAS[];



//
// Top-Left Header for multimaps
// some choices to choose one from 
//
void MapWindow::DrawMultimap_Topleft(const HDC hdc, const RECT rci)
{

  TCHAR topleft_txt[10];
  // HFONT oldFont = (HFONT)SelectObject(hdc, LK8TargetFont);

  HFONT  oldFont = (HFONT) SelectObject(hdc, LK8MediumFont);
  HBRUSH oldBrush= (HBRUSH)SelectObject(hdc,LKBrush_Mdark);
  HPEN     oldPen= (HPEN)  SelectObject(hdc, GetStockObject(WHITE_PEN));

  switch(MapSpaceMode)
  {
	case MSM_MAPTRK:
		_stprintf(topleft_txt, TEXT(" M1 "));

		break;

	case MSM_MAPWPT:
		_stprintf(topleft_txt, TEXT(" M2 "));

		break;

	case MSM_MAPASP:
		_stprintf(topleft_txt, TEXT(" M3"));

		break;

	case MSM_MAPRADAR:
		_stprintf(topleft_txt, TEXT(" M4"));

		break;
	default:
		break;
  } 

  MapWindow::LKWriteText(hdc, topleft_txt, LEFTLIMITER, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_LEFT, RGB_BLACK, true);

  SelectObject(hdc,oldBrush);
  SelectObject(hdc,oldPen);
  SelectObject(hdc,oldFont);
}



void MapWindow::DrawMultimap_DynaLabel(const HDC hdc, const RECT rci)
{

  HBRUSH oldBrush;
  HFONT oldFont;

  if (INVERTCOLORS)
        oldBrush=(HBRUSH)SelectObject(hdc,LKBrush_Petrol);
  else
        oldBrush=(HBRUSH)SelectObject(hdc,LKBrush_LightCyan);

  extern double fSplitFact;
  SIZE textSize;
  int midsplit=(long)((double)(rci.bottom-rci.top)*fSplitFact);

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

        SelectObject(hdc, MapWindowFont);
        if(Sonar_IsEnabled)
                LKWriteText(hdc, MsgToken(1293),  (rci.right), rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_RIGHT, RGB_GREEN, true);
        else
                LKWriteText(hdc, MsgToken(1293),  (rci.right), rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_RIGHT, RGB_AMBER, true);
  }

  SelectObject(hdc,oldBrush);
  SelectObject(hdc,oldFont);

}


