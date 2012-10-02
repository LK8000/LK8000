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
#include "DoInits.h"
#include "Sideview.h"
#include "Message.h"
#include "LKInterface.h"
#include "InputEvents.h"


extern int XstartScreen, YstartScreen;
//extern long VKtime;
extern int Sideview_asp_heading_task;
extern AirSpaceSideViewSTRUCT Sideview_pHandeled[MAX_NO_SIDE_AS];

extern long  iSonarLevel;
extern bool Sonar_IsEnabled;
extern AirSpaceSonarLevelStruct sSonarLevel[];
extern TCHAR Sideview_szNearAS[];
extern double fZOOMScale;
AirSpaceSonarLevelStruct sSonarLevel[10] = {
    /* horizontal sonar levels */
    /* Dist , Delay *0.5s, V/H,      soundfile */
    {  150,     3,         true, TEXT("LK_SONAR_H1.WAV")},
    {  330,     3,         true, TEXT("LK_SONAR_H2.WAV")},
    {  500,     5,         true, TEXT("LK_SONAR_H3.WAV")},
    {  650,     5,         true, TEXT("LK_SONAR_H4.WAV")},
    {  850,     7,         true, TEXT("LK_SONAR_H5.WAV")},
    /* vertical sonar levels */
    {  30 ,     3,         false, TEXT("LK_SONAR_H1.WAV")},
    {  50 ,     3,         false, TEXT("LK_SONAR_H2.WAV")},
    {  70,      5,         false, TEXT("LK_SONAR_H3.WAV")},
    {  90,      5,         false, TEXT("LK_SONAR_H4.WAV")},
    {  110,     7,         false, TEXT("LK_SONAR_H5.WAV")}
   };


int SonarNotify(void)
{
static unsigned long lSonarCnt = 0;

   lSonarCnt++;

   if(Sideview_asp_heading_task== 2)
   {
     if((iSonarLevel >=0) && (iSonarLevel < 10))
      if( lSonarCnt > (unsigned)sSonarLevel[iSonarLevel].iSoundDelay)
		{
		  lSonarCnt = 0;
                  // StartupStore(_T("... level=%d PLAY <%s>\n"),iSonarLevel,&sSonarLevel[iSonarLevel].szSoundFilename);
		  LKSound((TCHAR*) &(sSonarLevel[iSonarLevel].szSoundFilename));
		}
    }
  return 0;
}


void MapWindow::LKDrawMultimap_Asp(HDC hdc, const RECT rc)
{


RECT rci = rc;
rci.bottom -= BottomSize;
  if (DoInit[MDI_MAPASP]) {
	// init statics here and then clear init to false
	DoInit[MDI_MAPASP]=false;
  }


  switch(LKevent) {
	//
	// USABLE EVENTS
	// 



	case LKEVENT_TOPLEFT:
		InputEvents::setMode(_T("MMCONF"));
		break;
	break;

	case LKEVENT_TOPRIGHT:
	  if(Sideview_asp_heading_task== 2)
	  {
	    Sonar_IsEnabled = !Sonar_IsEnabled;
	    if (EnableSoundModes) {
	    	if (Sonar_IsEnabled)
			LKSound(TEXT("LK_TONEUP.WAV"));
		else
			LKSound(TEXT("LK_TONEDOWN.WAV"));
	    }
	  }
	break;

	default:
		// THIS SHOULD NEVER HAPPEN, but always CHECK FOR IT!
		break;
  }



  RenderAirspace( hdc,   rci);

  TCHAR topleft_txt[10];
  TCHAR topcenter_txt[80];
  switch(GetSideviewPage())
  {
	case 0:
		// No need to print "Heading". It is obvious.
		// _stprintf(topcenter_txt, TEXT("%s"), MsgToken(1290));
		_tcscpy(topcenter_txt,_T("HEADING"));
		_stprintf(topleft_txt, TEXT("M1: TRK"));
		break;

	case 1:
		if (GetOvertargetIndex()>=0)
		{
			TCHAR szOvtname[80];
			GetOvertargetName(szOvtname);
			_stprintf(topcenter_txt, TEXT("%s"), szOvtname);
			_stprintf(topleft_txt, TEXT("M2: WPT"));
		}
		else
		{
			_stprintf(topcenter_txt, TEXT("---"));
		}
		break;

	case 2:
		_stprintf(topcenter_txt, TEXT("%s"), Sideview_szNearAS );
		_stprintf(topleft_txt, TEXT("M3: ASP"));
		break;
	default:
		break;
  } 



  //SetBkMode(hdc, OPAQUE);


  HFONT hfOld = (HFONT)SelectObject(hdc, MapWindowFont);
  LKWriteText(hdc, topleft_txt, LEFTLIMITER, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_LEFT, RGB_WHITE, true);


  //SelectObject(hdc, LK8InfoSmallFont);
  //LKWriteText(hdc, topcenter_txt, rci.right/3, rci.top+TOPLIMITER , 0, WTMODE_NORMAL, WTALIGN_LEFT, RGB_BLACK, true);

  if (INVERTCOLORS)
	SelectObject(hdc,LKBrush_Petrol);
  else
	SelectObject(hdc,LKBrush_LightCyan);

  extern double fSplitFact;
  SIZE textSize;
  int midsplit=(long)((double)(rci.bottom-rci.top)*fSplitFact);
  SelectObject(hdc, LK8UnitFont);
  GetTextExtentPoint(hdc, _T("Y"), 1, &textSize);
  // move the label on top view when the topview window is big enough
  if (fSplitFact >0.5)
	midsplit-=textSize.cy;
  if (fSplitFact <0.5)
	midsplit+=textSize.cy;

  MapWindow::LKWriteBoxedText(hdc,topcenter_txt, rc.right/3, midsplit, 0, WTALIGN_CENTER);


  if(Sideview_asp_heading_task== 2)
  {
	SelectObject(hdc, MapWindowFont);
	if(Sonar_IsEnabled)
	    LKWriteText(hdc, MsgToken(1293),  (rc.right)-40, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_RIGHT, RGB_GREEN, true);
	  else
 	    LKWriteText(hdc, MsgToken(1293),  (rc.right)-40, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_RIGHT, RGB_AMBER, true); 
  }



  //SetBkMode(hdc, TRANSPARENT);

  SonarNotify();
  SelectObject(hdc, hfOld);
  LKevent=LKEVENT_NONE;



}

