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


#define SIZE0 0
#define SIZE1 30
#define SIZE2 50
#define SIZE3 70

extern int XstartScreen, YstartScreen;
//extern long VKtime;
extern int Sideview_asp_heading_task;
extern AirSpaceSideViewSTRUCT Sideview_pHandeled[MAX_NO_SIDE_AS];
extern int   Sideview_iNoHandeldSpaces;
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
//#define TEXT_BOX
static int iSplit = SIZE1;
int k;
bool bFound = false;
RECT rci = rc;
rci.bottom -= BottomSize;
  if (DoInit[MDI_MAPASP]) {
	// init statics here and then clear init to false
	DoInit[MDI_MAPASP]=false;
  }

  // 
  // X,Y coordinates of last clicked point on screen
  // These coordinates are related to any point clicked, even for a page flip, for bottom bar etc.
  // In some cases, you will read old coordinates because for example after clicking in the center of 
  // bottom bar, the page changed out of multimap and entered nearest pages.  
  // 
  int X=XstartScreen;
  int Y=YstartScreen;

  //
  // Duration of key is inside long VKtime, in milliseconds.
  //

   //  LKWriteBoxedText(hdc, _T("MULTIMAP ASP EXAMPLE"), 1, 1 , 0, WTALIGN_LEFT);
  TCHAR ttext[100];
  
  switch(LKevent) {
	//
	// USABLE EVENTS
	// 

	case LKEVENT_NEWRUN:
		// CALLED ON ENTRY: when we select this page coming from another mapspace
		_tcscpy(ttext,_T("Event = NEW RUN"));
		fZOOMScale = 1.0;
		break;
	case LKEVENT_UP:
		// click on upper part of screen, excluding center
		_tcscpy(ttext,_T("Event = UP"));
		fZOOMScale /= ZOOMFACTOR;
	        if (EnableSoundModes)PlayResource(TEXT("IDR_WAV_CLICK"));
		break;
	case LKEVENT_DOWN:
		// click on lower part of screen,  excluding center
		_tcscpy(ttext,_T("Event = DOWN"));
		fZOOMScale *= ZOOMFACTOR;
	        if (EnableSoundModes)PlayResource(TEXT("IDR_WAV_CLICK"));
		break;
	case LKEVENT_TOPLEFT:
	  if (EnableSoundModes)PlayResource(TEXT("IDR_WAV_CLICK"));
	  IncSideviewPage();
	  fZOOMScale = 1.0;
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
	case LKEVENT_LONGCLICK:
		 for (k=0 ; k <= Sideview_iNoHandeldSpaces; k++)
		 {
		   if( Sideview_pHandeled[k].psAS != NULL)
		   {
			 if (PtInRect(X,Y,Sideview_pHandeled[k].rc ))
			 {
			   if (EnableSoundModes)PlayResource(TEXT("IDR_WAV_BTONE4"));
			   dlgAirspaceDetails(Sideview_pHandeled[k].psAS);       // dlgA
			   bFound = true;
			 }
		   }
		 }

		break;
	case LKEVENT_PAGEUP:
		if(iSplit == SIZE1) iSplit = SIZE0;
		if(iSplit == SIZE2) iSplit = SIZE1;
		if(iSplit == SIZE3) iSplit = SIZE2;
		break;
	case LKEVENT_PAGEDOWN:
		if(iSplit == SIZE2) iSplit = SIZE3;
		if(iSplit == SIZE1) iSplit = SIZE2;
		if(iSplit == SIZE0) iSplit = SIZE1;

		break;

	default:
		// THIS SHOULD NEVER HAPPEN, but always CHECK FOR IT!
		_tcscpy(ttext,_T("Event = unknown"));
		break;
  }
/*
  if(fZOOMScale > 10.0)
	 fZOOMScale = 10.0;
  if(fZOOMScale < 0.1)
	 fZOOMScale = 0.1;
*/
  static int oldSplit=SIZE1;
  if(oldSplit != iSplit)
  {
	oldSplit=iSplit;
	SetSplitScreenSize(oldSplit);
  }


   RenderAirspace( hdc,   rci);

   TCHAR szTxt[80];
   switch(GetSideviewPage())
   {
     case 0:
       //_stprintf(szTxt, TEXT("ASP.1 %s"), gettext(TEXT("_@M1290_"))); // _@M1290_ "HEADING"
       _stprintf(szTxt, TEXT("A.1 HEADING"));
     break;
     case 1:
       if (GetOvertargetIndex()>=0)
       {
	 TCHAR szOvtname[80];
         GetOvertargetName(szOvtname);
         _stprintf(szTxt, TEXT("A.2 %s"), szOvtname);
       }
       else
         _stprintf(szTxt, TEXT("A.2 %s"),  MsgToken(479));                    // "None"
     break;
     case 2:
       _stprintf(szTxt, TEXT("A.3 %s"), Sideview_szNearAS );       //_@M1291_ "Near AS"      //"Showing nearest airspace"
     break;
   }

   COLORREF rgbTextColor = RGB_DARKGREY ;
 //  if(INVERTCOLORS)
//     rgbTextColor =  RGB_LIGHTGREEN;

  //HFONT hfOld = (HFONT)SelectObject(hdc, LK8InfoSmallFont);
  HFONT hfOld = (HFONT)SelectObject(hdc, MapWindowFont);

 //   SetBkMode(hdc, OPAQUE);

//	LKWriteText(hDC, tbear, ncenterx,ncentery, 0, WTMODE_OUTLINED, WTALIGN_CENTER, RGB_LIGHTYELLOW, false);
  //LKWriteText(hdc, szTxt, LEFTLIMITER, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_LEFT, rgbTextColor, true);
  LKWriteText(hdc, szTxt, LEFTLIMITER, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_LEFT, RGB_WHITE, true);
	if(Sideview_asp_heading_task== 2)
	{
	  if(Sonar_IsEnabled)
	    //LKWriteText(hdc, gettext(_T("_@M1293_")),  (rc.right)-40, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_RIGHT, RGB_GREEN, true); // _@M1294_ "Sonar On"
	    LKWriteText(hdc, _T("SONAR"),  (rc.right)-40, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_RIGHT, RGB_GREEN, true);
	  else
 	    //LKWriteText(hdc, gettext(_T("_@M1294_")),  (rc.right)-40, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_RIGHT, RGB_WHITE, true); // _@M1294_ "Sonar Off"
 	    LKWriteText(hdc, _T("SONAR"),  (rc.right)-40, rci.top+TOPLIMITER , 0, WTMODE_OUTLINED, WTALIGN_RIGHT, RGB_AMBER, true); 
	}
//    SetBkMode(hdc, TRANSPARENT);
	SonarNotify();
    SelectObject(hdc, hfOld);




  // After using the event, WE MUST CLEAR IT, otherwise it will survive for next run.
  // This can be good for something, though, like automatic redo of last action.
  // You can also clear this event at the end of this function, to know during execution which was
  // the key pressed, but remember to clear it.
  LKevent=LKEVENT_NONE;



}

