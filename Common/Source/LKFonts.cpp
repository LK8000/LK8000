/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "ScreenGeometry.h"

extern void ApplyClearType(LOGFONT *logfont);
extern void ApplyFontSize(LOGFONT *logfont);

// Called after InitLKScreen, normally
void InitLKFonts()
{
  #if TESTBENCH
  StartupStore(_T(". InitLKFonts%s"),NEWLINE); // 091213
  #endif

  LOGFONT logfontTarget;	// StatisticsWindow
  LOGFONT logfontBig;		// InfoWindow
  LOGFONT logfontTitle;		// MapWindow
  LOGFONT logfontTitleNavbox;
  LOGFONT logfontMap;		// MapWindow compatible, safe to user changed with edit fonts
  LOGFONT logfontValue;		// StatisticsWindow
  LOGFONT logfontUnit;		// TitleSmallWindow
  LOGFONT logfontSmall;
  LOGFONT logfontMedium;
  LOGFONT logfontInfoBig;
  LOGFONT logfontInfoBigItalic;
  LOGFONT logfontInfoNormal;
  LOGFONT logfontInfoSmall;
  LOGFONT logfontPanelBig;
  LOGFONT logfontPanelMedium;
  LOGFONT logfontPanelSmall;
  LOGFONT logfontPanelUnit;


  memset ((char *)&logfontTarget, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontBig, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontTitle, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontTitleNavbox, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontMap, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontValue, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontUnit, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontSmall, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontMedium, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontInfoBig, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontInfoBigItalic, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontInfoNormal, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontInfoSmall, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontPanelBig, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontPanelMedium, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontPanelSmall, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontPanelUnit, 0, sizeof (LOGFONT) );


  //
  // PRESET RESOLUTIONS FOR LK8000
  // Like for Fonts.cpp, we have embedded resolutions fine tuned, and we rescale upon them.
  // We use screen aspect (we call it also geometry) to detect the best match if the 
  // resolution is not among these.
  //

  // remember: changing values here may need cosmetic changes in InfoBoxLayout where


  //
  // LANDSCAPE ORIENTATION
  // 
  if (ScreenSize==(ScreenSize_t)ss800x480 ||
      (ScreenSize==0 && ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_53) )
  {
	splitter=6;
	propGetFontSettingsFromString(TEXT("72,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);	// 64 600
	propGetFontSettingsFromString(TEXT("41,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);	// 40 600
	propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);

 	propGetFontSettingsFromString(TEXT("36,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
 	propGetFontSettingsFromString(TEXT("34,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
 	propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap); // was 36
	// propGetFontSettingsFromString(TEXT("50,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);	// 46 600
	propGetFontSettingsFromString(TEXT("54,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget); // 100914
	propGetFontSettingsFromString(TEXT("48,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
	propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);

	propGetFontSettingsFromString(TEXT("40,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig); 
	propGetFontSettingsFromString(TEXT("40,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic); 
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal); 
	propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);

	propGetFontSettingsFromString(TEXT("64,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);  
	propGetFontSettingsFromString(TEXT("48,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium); 
	propGetFontSettingsFromString(TEXT("32,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall); 
	propGetFontSettingsFromString(TEXT("22,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 

  } 
  else if (ScreenSize==(ScreenSize_t)ss400x240)
  {
	splitter=6;
	propGetFontSettingsFromString(TEXT("36,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig); // v2.2
	propGetFontSettingsFromString(TEXT("38,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig); 
	propGetFontSettingsFromString(TEXT("21,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);	// 40 600
	propGetFontSettingsFromString(TEXT("9,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);

 	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
 	propGetFontSettingsFromString(TEXT("17,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
 	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap); // was 36
	//propGetFontSettingsFromString(TEXT("25,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);  // v2.2
	propGetFontSettingsFromString(TEXT("28,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget); // 110704
	propGetFontSettingsFromString(TEXT("24,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
	propGetFontSettingsFromString(TEXT("9,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);

	// 20 is ok for 8 rows in nearest pages - 110704 bigger fonts on smaller screens
	//propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);        // v2.2
	//propGetFontSettingsFromString(TEXT("20,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);  // v2.2
	propGetFontSettingsFromString(TEXT("25,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("25,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic); 
	propGetFontSettingsFromString(TEXT("17,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal); 
	propGetFontSettingsFromString(TEXT("15,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);

	propGetFontSettingsFromString(TEXT("32,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);  
	propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium); 
	propGetFontSettingsFromString(TEXT("16,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall); 
	propGetFontSettingsFromString(TEXT("11,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 

  } 
  else if ( (ScreenSize==(ScreenSize_t)ss480x272) ||
       (ScreenSize==0 && ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_169) )
  {
	if (ScreenSizeX==854)
		splitter=6;
	else
		splitter=5;
	propGetFontSettingsFromString(TEXT("48,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	#if 0
	if (splitter==6) {
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("24,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	} else {
	#endif
		propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("32,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	// }
	propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig); 
	propGetFontSettingsFromString(TEXT("28,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic); 
	propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("38,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig); 
	propGetFontSettingsFromString(TEXT("26,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 

  }
  else if (ScreenSize==(ScreenSize_t)ss720x408) { // WQVGA  e.g. MIO

	splitter=5;
	propGetFontSettingsFromString(TEXT("72,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	propGetFontSettingsFromString(TEXT("39,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("15,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
	propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
	propGetFontSettingsFromString(TEXT("48,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
	propGetFontSettingsFromString(TEXT("45,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
	propGetFontSettingsFromString(TEXT("21,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);

	propGetFontSettingsFromString(TEXT("42,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig); 
	propGetFontSettingsFromString(TEXT("42,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic); 
	propGetFontSettingsFromString(TEXT("33,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("57,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig); 
	propGetFontSettingsFromString(TEXT("39,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("27,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("21,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
  }
  else if ( ScreenSize==(ScreenSize_t)ss480x234 ||
       (ScreenSize==0 && ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_21) )
  {

	splitter=5;
	#if (WINDOWSPC>0)
	propGetFontSettingsFromString(TEXT("38,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	#else
	propGetFontSettingsFromString(TEXT("38,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontBig);
	#endif
	propGetFontSettingsFromString(TEXT("24,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	#if 0
	if (splitter==6) {
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("20,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	} else {
	#endif
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
	propGetFontSettingsFromString(TEXT("28,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
	propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	// }
	propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("28,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	#if (WINDOWSPC>0)
	propGetFontSettingsFromString(TEXT("34,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
	#else
	propGetFontSettingsFromString(TEXT("34,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontPanelBig);
	#endif
	propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("16,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
  }
  else if (ScreenSize==(ScreenSize_t)ss320x240) 
  {
	// Units are too big, tahoma is not enough. Units are disabled by default.
	// This is also used by 640x480 devices actually, if SE_VGA is not disabled

	splitter=5;
	propGetFontSettingsFromString(TEXT("34,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	propGetFontSettingsFromString(TEXT("16,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	#if 0
	if (splitter==6) {
		propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("20,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("16,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);

	} else {
	#endif
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("24,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("22,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	//}
	propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("20,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
	propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
  } 
  else if (ScreenSize==(ScreenSize_t)ss640x480 ||
       (ScreenSize==0 && ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_43) )
  {
	splitter=5;
	#if (WINDOWSPC>0)
	propGetFontSettingsFromString(TEXT("64,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	#else
	propGetFontSettingsFromString(TEXT("64,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontBig);
	#endif
	propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	#if 0
	if (splitter==6) {
		propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("46,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue); 
		propGetFontSettingsFromString(TEXT("16,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);

		} else {		
		#endif
		propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("46,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("44,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("16,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	// }
	propGetFontSettingsFromString(TEXT("38,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("38,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("22,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("56,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
	propGetFontSettingsFromString(TEXT("44,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("28,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("20,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
  }
  else if (ScreenSize==(ScreenSize_t)ss896x672)
  {
	splitter=6;
	#if (WINDOWSPC>0)
	propGetFontSettingsFromString(TEXT("89,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	#else
	propGetFontSettingsFromString(TEXT("89,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontBig);
	#endif
	propGetFontSettingsFromString(TEXT("44,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("20,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	if (splitter==6) {
		propGetFontSettingsFromString(TEXT("38,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("38,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("38,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("62,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("44,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue); 
		propGetFontSettingsFromString(TEXT("22,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);

	} 
	#if 0
	else {		
		propGetFontSettingsFromString(TEXT("44,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("44,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("44,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("64,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("61,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("22,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	}
	#endif
	propGetFontSettingsFromString(TEXT("53,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("53,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("47,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("30,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("78,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
	propGetFontSettingsFromString(TEXT("61,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("39,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("28,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
  }
	//
        // PORTRAIT ORIENTATION
        //
  else if (ScreenSize==(ScreenSize_t)ss240x320) 
  {
	splitter=3;
		propGetFontSettingsFromString(TEXT("30,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontBig);
		propGetFontSettingsFromString(TEXT("16,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
		propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);

	if (splitter==3) {
		// Splitter = 3 on two rows
		propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("22,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("23,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("9,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	}
	// ----------- unused
	#if 0
	if (splitter==6) {
		propGetFontSettingsFromString(TEXT("9,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("9,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("9,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("20,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("11,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("6,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	}
	if (splitter==5) {
		// very small, only a sample of what can be seen under landscape mode
		propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("22,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("17,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("6,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	} 
	else {
		// Splitter 4
		propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("22,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		#if (WINDOWSPC>0)
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		#else
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), &logfontValue);
		#endif
		propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	}
	#endif
	// -------- end unused
	propGetFontSettingsFromString(TEXT("17,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("17,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("15,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("22,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontPanelBig);
	propGetFontSettingsFromString(TEXT("17,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);  // 10 101005
  }
  else if (ScreenSize==(ScreenSize_t)ss272x480 ||
    (ScreenSize==0 && !ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_169) )
  {
	splitter=3;
	propGetFontSettingsFromString(TEXT("36,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		propGetFontSettingsFromString(TEXT("20,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
		propGetFontSettingsFromString(TEXT("10,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	if (splitter==3) {
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("28,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), &logfontValue);
		propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	}
	#if 0
	if (splitter==5) {
		// very small, only a sample of what can be seen under landscape mode
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("26,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	} 
	else {
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("26,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("20,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), &logfontValue);
		propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	}
	#endif
	propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("18,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("28,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
	propGetFontSettingsFromString(TEXT("18,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
  }
  else if (ScreenSize==(ScreenSize_t)ss480x640 ||
       (ScreenSize==0 && !ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_43) )
  {

	splitter=3;
	#if (WINDOWSPC>0)
	propGetFontSettingsFromString(TEXT("60,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	#else
	propGetFontSettingsFromString(TEXT("60,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontBig);
	#endif
	propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);

	if (splitter==3) { 
		/* ----------------------------
		// Splitter = 6 , all-in-a-line  unused being too small,  otherwise ok
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("44,2,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("44,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("40,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
		   ----------------------------- */
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("46,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget); // 44 101005
		propGetFontSettingsFromString(TEXT("46,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	} 
	#if 0
	else {	
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("44,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget); // 600
		propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	}
	#endif
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig); 
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("30,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	#if (WINDOWSPC>0)
	propGetFontSettingsFromString(TEXT("44,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);  // 101004: 49 800
	#else
	propGetFontSettingsFromString(TEXT("44,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontPanelBig); 
	#endif
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("16,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);  // 101004 18
  }
  else if (ScreenSize==(ScreenSize_t)ss480x800 ||
       (ScreenSize==0 && !ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_53) )
  {
	splitter=3;
	#if (WINDOWSPC>0)
	propGetFontSettingsFromString(TEXT("62,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	#else
	propGetFontSettingsFromString(TEXT("62,0,0,0,800,0,0,0,0,0,0,3,2,TahomaBD"), &logfontBig);
	#endif
	propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);

	if (splitter==3) {
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("46,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("46,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	}
	#if 0
	if (splitter==6) { // unused
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("44,2,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);

	}
	else {		
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("44,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget); // 600
		propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
	}
	#endif
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig); 
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("30,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	#if (WINDOWSPC>0)
	propGetFontSettingsFromString(TEXT("48,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);  // 101004 49 800
	#else
	propGetFontSettingsFromString(TEXT("48,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), &logfontPanelBig); 
	#endif
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("20,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
  }
  //
  // ELSE WE DID NOT FIND A VALID CUSTOM RESOLUTION OR A VALID SCREEN GEOMETRY FOR THIS ORIENTATION!
  // We use 16:9 480x272 settings, but no warranty!
  //
  else {
        StartupStore(_T(". >> (LKFonts) Unknown unsupported screen geometry or resolution%s"),NEWLINE);
	if (ScreenLandscape) {
		splitter=5;
		propGetFontSettingsFromString(TEXT("48,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
		propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
		propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("32,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontValue);
		propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
		propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig); 
		propGetFontSettingsFromString(TEXT("28,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic); 
		propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
		propGetFontSettingsFromString(TEXT("38,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig); 
		propGetFontSettingsFromString(TEXT("26,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
		propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit); 
	} else {
		splitter=3;
		propGetFontSettingsFromString(TEXT("36,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		propGetFontSettingsFromString(TEXT("20,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
		propGetFontSettingsFromString(TEXT("10,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleNavbox);
		propGetFontSettingsFromString(TEXT("28,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,TahomaBD"), &logfontValue);
		propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontUnit);
		propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
		propGetFontSettingsFromString(TEXT("18,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
		propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
		propGetFontSettingsFromString(TEXT("28,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
		propGetFontSettingsFromString(TEXT("18,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	}
  }

  //
  // If our choice was based upon geometry criteria, and not on a custom supported resolution,
  // we must ask for dynamic resize. So we mark Orientation flag to 1 .
  // We use this flag to autogenerate fonts for unknown resolutions.
  // We do the same in Fonts.cpp
  if (ScreenSize==0) {
      #if TESTBENCH
      StartupStore(_T("... (LKFonts) Forcing font resize%s"),NEWLINE);
      #endif
      logfontBig.lfOrientation = 1;
      logfontMedium.lfOrientation = 1;
      logfontSmall.lfOrientation = 1;
      logfontTitle.lfOrientation = 1;
      logfontMap.lfOrientation = 1;
      logfontTitleNavbox.lfOrientation = 1;
      logfontTarget.lfOrientation = 1;
      logfontValue.lfOrientation = 1;
      logfontUnit.lfOrientation = 1;
      logfontInfoBig.lfOrientation = 1;
      logfontInfoBigItalic.lfOrientation = 1;
      logfontInfoNormal.lfOrientation = 1;
      logfontInfoSmall.lfOrientation = 1;
      logfontPanelBig.lfOrientation = 1;
      logfontPanelMedium.lfOrientation = 1;
      logfontPanelSmall.lfOrientation = 1;

      #define TIGHTOFFSET 0 // previously 4, but doubtly correct
      if (ScreenLandscape) {
          BottomSize=(int) (((double)logfontTitle.lfHeight + logfontValue.lfHeight - TIGHTOFFSET)*Screen0Ratio);
      } else {
          BottomSize=(int) ((((double)logfontTitle.lfHeight + logfontValue.lfHeight)*2 - TIGHTOFFSET)*Screen0Ratio);
      }
  }


  ApplyClearType(&logfontTarget);
  ApplyFontSize(&logfontTarget);
  ApplyClearType(&logfontBig);
  ApplyFontSize(&logfontBig);
  ApplyClearType(&logfontValue);
  ApplyFontSize(&logfontValue);
  ApplyClearType(&logfontTitle);
  ApplyFontSize(&logfontTitle);
  ApplyClearType(&logfontMap);
  ApplyFontSize(&logfontMap);
  ApplyClearType(&logfontTitleNavbox);
  ApplyFontSize(&logfontTitleNavbox);
  ApplyClearType(&logfontUnit);
  ApplyFontSize(&logfontUnit);
  ApplyClearType(&logfontMedium);
  ApplyFontSize(&logfontMedium);
  ApplyClearType(&logfontSmall);
  ApplyFontSize(&logfontSmall);
  ApplyClearType(&logfontInfoBig);
  ApplyFontSize(&logfontInfoBig);
  ApplyClearType(&logfontInfoBigItalic);
  ApplyFontSize(&logfontInfoBigItalic);
  ApplyClearType(&logfontInfoNormal);
  ApplyFontSize(&logfontInfoNormal);
  ApplyClearType(&logfontInfoSmall);
  ApplyFontSize(&logfontInfoSmall);
  ApplyClearType(&logfontPanelBig);
  ApplyFontSize(&logfontPanelBig);
  ApplyClearType(&logfontPanelMedium);
  ApplyFontSize(&logfontPanelMedium);
  ApplyClearType(&logfontPanelSmall);
  ApplyFontSize(&logfontPanelSmall);
  ApplyClearType(&logfontPanelUnit);
  ApplyFontSize(&logfontPanelUnit);

  LK8UnitFont.Create(&logfontUnit);
  LK8TitleFont.Create(&logfontTitle);
  LK8MapFont.Create(&logfontMap);
  LK8TitleNavboxFont.Create(&logfontTitleNavbox);
  LK8ValueFont.Create(&logfontValue);
  LK8TargetFont.Create(&logfontTarget); 
  LK8BigFont.Create(&logfontBig);
  LK8MediumFont.Create(&logfontMedium);
  LK8SmallFont.Create(&logfontSmall);
  LK8InfoBigFont.Create(&logfontInfoBig);
  LK8InfoBigItalicFont.Create(&logfontInfoBigItalic);
  LK8InfoNormalFont.Create(&logfontInfoNormal);
  LK8InfoSmallFont.Create(&logfontInfoSmall);
  LK8PanelBigFont.Create(&logfontPanelBig);
  LK8PanelMediumFont.Create(&logfontPanelMedium);
  LK8PanelSmallFont.Create(&logfontPanelSmall);
  LK8PanelUnitFont.Create(&logfontPanelUnit);


}


