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
  HPEN oldPen;
  HBRUSH oldBrush;
  HFONT oldFont;

//oldFont = (HFONT)SelectObject(hdc, MapWindowFont);
//oldFont = (HFONT)SelectObject(hdc, LK8InfoSmallFont);

  switch(GetSideviewPage())
  {
	case IM_HEADING:
		_stprintf(topleft_txt, TEXT(" 1 TRK"));

		oldFont = (HFONT)SelectObject(hdc, LK8ValueFont);
		oldBrush=(HBRUSH)SelectObject(hdc,LKBrush_Mdark);
		oldPen=(HPEN) SelectObject(hdc, GetStockObject(WHITE_PEN));
		MapWindow::LKWriteText(hdc, topleft_txt, LEFTLIMITER, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_LEFT, RGB_BLACK, true);
		break;

	case IM_NEXT_WP:
		_stprintf(topleft_txt, TEXT(" 2 WPT"));

		oldFont = (HFONT)SelectObject(hdc, LK8MediumFont);
		oldBrush=(HBRUSH)SelectObject(hdc,LKBrush_Mdark);
		oldPen=(HPEN) SelectObject(hdc, GetStockObject(WHITE_PEN));
		MapWindow::LKWriteText(hdc, topleft_txt, LEFTLIMITER, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_LEFT, RGB_BLACK, true);
		break;

	case IM_NEAR_AS:
		_stprintf(topleft_txt, TEXT(" 3"));

		oldFont = (HFONT)SelectObject(hdc, LK8TargetFont);
		oldBrush=(HBRUSH)SelectObject(hdc,LKBrush_Mdark);
		oldPen=(HPEN) SelectObject(hdc, GetStockObject(WHITE_PEN));
		MapWindow::LKWriteText(hdc, topleft_txt, LEFTLIMITER, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_LEFT, RGB_BLACK, true);
		break;
	default:
		break;
  } 

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

  if(GetSideviewPage()== IM_NEXT_WP)
  {
        TCHAR topcenter_txt[80];
        _stprintf(topcenter_txt, TEXT("BADRAGAZ  61.2Km  >>63  +1234m"));
        SelectObject(hdc,LKBrush_Green);
        MapWindow::LKWriteBoxedText(hdc,&MapRect,topcenter_txt, rci.right/3, midsplit, 0, WTALIGN_CENTER, RGB_BLACK, RGB_BLACK);
  }


  SelectObject(hdc,oldBrush);
  SelectObject(hdc,oldFont);

}


