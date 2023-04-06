/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "ScreenGeometry.h"

extern void ApplyClearType(LOGFONT *logfont);
extern void ApplyFontSize(LOGFONT *logfont);
extern void ApplyCustomResize(LOGFONT *logfont, short change);

extern void InitializeOneFont (LKFont& theFont, LOGFONT logfont);

//
// IF WE ADD A FONT, WE MUST ADD ALSO THE NEW DEINIT FOR IT , REMEMBER
//
void Init_Fonts_2(void)
{
  #if TESTBENCH
  StartupStore(_T(". Init_Fonts_2%s"),NEWLINE);
  #endif

  LOGFONT logfontTarget;	// StatisticsWindow
  LOGFONT logfontBig;		// InfoWindow
  LOGFONT logfontTitle;		// MapWindow
  LOGFONT logfontGenericVar01;
  LOGFONT logfontGenericVar02;
  LOGFONT logfontMap;		// MapWindow compatible, safe to user changed with edit fonts
  LOGFONT logfontSmall;
  LOGFONT logfontMedium;
  LOGFONT logfontInfoBig;	// infopages x.x  only
  LOGFONT logfontInfoBigItalic; // infopages x.x  only
  LOGFONT logfontInfoBig2L;	// infopages x.x  only
  LOGFONT logfontInfoBigItalic2L; // infopages x.x  only
  LOGFONT logfontInfoNearest;   // infopages header "APTS 1.1 Distance .."
  LOGFONT logfontInfoNormal;    // multimaps GA
  LOGFONT logfontInfoSmall;     // welcome  visualGlide  HSI
  LOGFONT logfontPanelBig;
  LOGFONT logfontPanelMedium;   // infopages header topleft number "2.x" and HSI
  LOGFONT logfontPanelSmall;
  LOGFONT logfontPanelUnit;     // used for FAI Sector scaling

  // BottomBar ONLY
  //
  LOGFONT logfontBottomBarTitle;
  LOGFONT logfontBottomBarValue;
  LOGFONT logfontBottomBarUnit;

  // Overlays ONLY
  //
  LOGFONT logfontOverlayBig;
  LOGFONT logfontOverlayMedium; // LK8OverlayGatesFont
  LOGFONT logfontOverlaySmall;
  LOGFONT logfontOverlayTitle;  // LK8OverlayMcModeFont
  LOGFONT logfontOverlayTarget;

  // VisualGlide (MM4)  ONLY
  //
  LOGFONT logfontVisualTop;
  LOGFONT logfontVisualBot;

  memset ((char *)&logfontTarget, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontBig, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontTitle, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontGenericVar01, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontMap, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontBottomBarValue, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontBottomBarUnit, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontSmall, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontMedium, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontInfoBig, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontInfoBigItalic, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontInfoBig2L, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontInfoBigItalic2L, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontInfoNearest, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontInfoNormal, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontInfoSmall, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontPanelBig, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontPanelMedium, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontPanelSmall, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontPanelUnit, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontBottomBarTitle, 0, sizeof (LOGFONT) );

  memset ((char *)&logfontOverlayBig, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontOverlayMedium, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontOverlaySmall, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontOverlayTitle, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontOverlayTarget, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontVisualTop, 0, sizeof (LOGFONT) );
  memset ((char *)&logfontVisualBot, 0, sizeof (LOGFONT) );

  //
  // PRESET RESOLUTIONS FOR LK8000
  // Like for Fonts.cpp, we have embedded resolutions fine tuned, and we rescale upon them.
  // We use screen aspect (we call it also geometry) to detect the best match if the
  // resolution is not among these.
  //

  //
  // LANDSCAPE ORIENTATION
  //
  if (ScreenSize==(ScreenSize_t)ss800x480 ||
      (ScreenSize==0 && ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_53) )
  {
	propGetFontSettingsFromString(TEXT("72,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);	// 64 600
	propGetFontSettingsFromString(TEXT("72,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayBig);
	propGetFontSettingsFromString(TEXT("41,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);	// 40 600
	propGetFontSettingsFromString(TEXT("41,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayMedium);
	propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlaySmall);

	propGetFontSettingsFromString(TEXT("36,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	propGetFontSettingsFromString(TEXT("36,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTitle);
	propGetFontSettingsFromString(TEXT("34,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar01);
	propGetFontSettingsFromString(TEXT("34,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarTitle);
	propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap); // was 36
	propGetFontSettingsFromString(TEXT("54,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget); // 100914
	propGetFontSettingsFromString(TEXT("54,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTarget); // 100914
	propGetFontSettingsFromString(TEXT("48,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarValue);
	propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarUnit);
	propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar02);

	propGetFontSettingsFromString(TEXT("40,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("40,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNearest);
	propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);

	propGetFontSettingsFromString(TEXT("64,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
	propGetFontSettingsFromString(TEXT("48,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("32,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("22,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);

	logfontInfoBig2L=logfontInfoBig;
	logfontInfoBigItalic2L=logfontInfoBigItalic;
  }
  else if (ScreenSize==(ScreenSize_t)ss800x600)
  {
	propGetFontSettingsFromString(TEXT("80,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	propGetFontSettingsFromString(TEXT("80,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayBig);
	propGetFontSettingsFromString(TEXT("40,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("40,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayMedium);
	propGetFontSettingsFromString(TEXT("17,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	propGetFontSettingsFromString(TEXT("17,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlaySmall);
	propGetFontSettingsFromString(TEXT("40,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	propGetFontSettingsFromString(TEXT("40,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTitle);
	propGetFontSettingsFromString(TEXT("40,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
	propGetFontSettingsFromString(TEXT("40,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar01);
	propGetFontSettingsFromString(TEXT("36,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarTitle);
	propGetFontSettingsFromString(TEXT("46,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
	propGetFontSettingsFromString(TEXT("46,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTarget);
	propGetFontSettingsFromString(TEXT("54,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarValue);
	propGetFontSettingsFromString(TEXT("20,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarUnit);
	propGetFontSettingsFromString(TEXT("20,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar02);
	propGetFontSettingsFromString(TEXT("47,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("47,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("42,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("42,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNearest);
	propGetFontSettingsFromString(TEXT("27,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("70,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
	propGetFontSettingsFromString(TEXT("55,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("35,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("25,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);

	logfontInfoBig2L=logfontInfoBig;
	logfontInfoBigItalic2L=logfontInfoBigItalic;
  }
  else if (ScreenSize==(ScreenSize_t)ss400x240)
  {
	propGetFontSettingsFromString(TEXT("38,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	propGetFontSettingsFromString(TEXT("38,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayBig);
	propGetFontSettingsFromString(TEXT("21,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);	// 40 600
	propGetFontSettingsFromString(TEXT("21,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayMedium);	// 40 600
	propGetFontSettingsFromString(TEXT("9,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	propGetFontSettingsFromString(TEXT("9,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlaySmall);

	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTitle);
	propGetFontSettingsFromString(TEXT("17,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar01);
	propGetFontSettingsFromString(TEXT("17,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarTitle);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap); // was 36
	propGetFontSettingsFromString(TEXT("28,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget); // 110704
	propGetFontSettingsFromString(TEXT("28,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTarget); // 110704
	propGetFontSettingsFromString(TEXT("24,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarValue);
	propGetFontSettingsFromString(TEXT("9,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarUnit);
	propGetFontSettingsFromString(TEXT("9,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar02);

	propGetFontSettingsFromString(TEXT("25,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("25,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("17,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("17,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNearest);
	propGetFontSettingsFromString(TEXT("15,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);

	propGetFontSettingsFromString(TEXT("32,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
	propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("16,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("11,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);

	logfontInfoBig2L=logfontInfoBig;
	logfontInfoBigItalic2L=logfontInfoBigItalic;

  }
  else if ( (ScreenSize==(ScreenSize_t)ss480x272) ||
       (ScreenSize==0 && ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_169) )
  {
	propGetFontSettingsFromString(TEXT("48,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	propGetFontSettingsFromString(TEXT("44,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayBig);
	propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayMedium);
	propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlaySmall);
	propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTitle);
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar01);
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarTitle);
	propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
	propGetFontSettingsFromString(TEXT("32,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
	propGetFontSettingsFromString(TEXT("30,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTarget);
	propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarValue);
	propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarUnit);
	propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar02);
	propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("28,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("36,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
	propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNearest);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);

	logfontInfoBig2L=logfontInfoBig;
	logfontInfoBigItalic2L=logfontInfoBigItalic;
  }
  else if ( ScreenSize==(ScreenSize_t)ss480x234 ||
       (ScreenSize==0 && ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_21) )
  {

	propGetFontSettingsFromString(TEXT("38,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	propGetFontSettingsFromString(TEXT("38,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayBig);
	propGetFontSettingsFromString(TEXT("24,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("24,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayMedium);
	propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlaySmall);
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTitle);
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar01);
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarTitle);
	propGetFontSettingsFromString(TEXT("28,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
	propGetFontSettingsFromString(TEXT("28,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTarget);
	propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarValue);
	propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarUnit);
	propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar02);
	propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("28,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNearest);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("34,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
	propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("16,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);

	logfontInfoBig2L=logfontInfoBig;
	logfontInfoBigItalic2L=logfontInfoBigItalic;
  }
  else if (ScreenSize==(ScreenSize_t)ss320x240)
  {
	// Units are too big, tahoma is not enough. Units are disabled by default.
	// This is also used by 640x480 devices actually, if SE_VGA is not disabled

	propGetFontSettingsFromString(TEXT("34,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	propGetFontSettingsFromString(TEXT("34,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayBig);
	propGetFontSettingsFromString(TEXT("16,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("16,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayMedium);
	propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlaySmall);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTitle);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar01);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarTitle);
	propGetFontSettingsFromString(TEXT("24,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
	propGetFontSettingsFromString(TEXT("24,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTarget);
	propGetFontSettingsFromString(TEXT("22,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarValue);
	propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarUnit);
	propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar02);
	propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("20,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNearest);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
	propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);

	logfontInfoBig2L=logfontInfoBig;
	logfontInfoBigItalic2L=logfontInfoBigItalic;
  }
  else if (ScreenSize==(ScreenSize_t)ss640x480 ||
       (ScreenSize==0 && ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_43) )
  {
	propGetFontSettingsFromString(TEXT("64,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	propGetFontSettingsFromString(TEXT("64,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayBig);
	propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayMedium);
	propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlaySmall);
	propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTitle);
	propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
	propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar01);
	propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarTitle);
	propGetFontSettingsFromString(TEXT("46,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
	propGetFontSettingsFromString(TEXT("46,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTarget);
	propGetFontSettingsFromString(TEXT("44,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarValue);
	propGetFontSettingsFromString(TEXT("16,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarUnit);
	propGetFontSettingsFromString(TEXT("16,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar02);
	propGetFontSettingsFromString(TEXT("38,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("38,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNearest);
	propGetFontSettingsFromString(TEXT("22,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("56,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
	propGetFontSettingsFromString(TEXT("44,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("28,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("20,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);

	logfontInfoBig2L=logfontInfoBig;
	logfontInfoBigItalic2L=logfontInfoBigItalic;
  }

  //
  // PORTRAIT ORIENTATION
  //

  else if (ScreenSize==(ScreenSize_t)ss240x320)
  {
	propGetFontSettingsFromString(TEXT("30,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	propGetFontSettingsFromString(TEXT("30,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayBig);
	propGetFontSettingsFromString(TEXT("16,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("16,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayMedium);
	propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlaySmall);

	propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTitle);
	propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
	propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar01);
	propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarTitle);
	propGetFontSettingsFromString(TEXT("22,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
	propGetFontSettingsFromString(TEXT("22,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTarget);
	propGetFontSettingsFromString(TEXT("23,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarValue);
	propGetFontSettingsFromString(TEXT("9,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarUnit);
	propGetFontSettingsFromString(TEXT("9,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar02);
	propGetFontSettingsFromString(TEXT("17,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("17,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("17,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig2L);
	propGetFontSettingsFromString(TEXT("17,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic2L);
	propGetFontSettingsFromString(TEXT("15,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("15,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNearest);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("22,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
	propGetFontSettingsFromString(TEXT("17,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("8,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);  // 10 101005
  }
  else if (ScreenSize==(ScreenSize_t)ss272x480 ||
    (ScreenSize==0 && !ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_169) )
  {
	propGetFontSettingsFromString(TEXT("36,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	propGetFontSettingsFromString(TEXT("36,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayBig);
	propGetFontSettingsFromString(TEXT("20,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("20,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayMedium);
	propGetFontSettingsFromString(TEXT("10,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	propGetFontSettingsFromString(TEXT("10,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlaySmall);
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTitle);
	propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
	propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar01);
	propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarTitle);
	propGetFontSettingsFromString(TEXT("28,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
	propGetFontSettingsFromString(TEXT("28,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTarget);
	propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarValue);
	propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarUnit);
	propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar02);
	propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("22,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig2L);
	propGetFontSettingsFromString(TEXT("28,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic2L);
	propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNearest);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("28,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
	propGetFontSettingsFromString(TEXT("18,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);
  }
  else if (ScreenSize==(ScreenSize_t)ss480x640 ||
       (ScreenSize==0 && !ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_43) )
  {

	propGetFontSettingsFromString(TEXT("60,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	propGetFontSettingsFromString(TEXT("60,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayBig);
	propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayMedium);
	propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlaySmall);

	propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTitle);
	propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
	propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar01);
	propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarTitle);
	propGetFontSettingsFromString(TEXT("46,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget); // 44 101005
	propGetFontSettingsFromString(TEXT("46,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTarget); // 44 101005
	propGetFontSettingsFromString(TEXT("46,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarValue);
	propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarUnit);
	propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar02);
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig2L);
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic2L);
	propGetFontSettingsFromString(TEXT("30,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("30,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNearest);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("44,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
	propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("16,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);  // 101004 18
  }
  else if (ScreenSize==(ScreenSize_t)ss600x800)
  {

	propGetFontSettingsFromString(TEXT("75,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	propGetFontSettingsFromString(TEXT("75,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayBig);
	propGetFontSettingsFromString(TEXT("40,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("40,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayMedium);
	propGetFontSettingsFromString(TEXT("15,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	propGetFontSettingsFromString(TEXT("15,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlaySmall);
	propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTitle);
	propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
	propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar01);
	propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarTitle);
	propGetFontSettingsFromString(TEXT("62,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarValue);

	propGetFontSettingsFromString(TEXT("57,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget); // 44 101005
	propGetFontSettingsFromString(TEXT("57,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTarget); // 44 101005
	propGetFontSettingsFromString(TEXT("22,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarUnit);
	propGetFontSettingsFromString(TEXT("22,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar02);
	propGetFontSettingsFromString(TEXT("43,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("43,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("43,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig2L);
	propGetFontSettingsFromString(TEXT("43,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic2L);
	propGetFontSettingsFromString(TEXT("37,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("37,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNearest);
	propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("58,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
	propGetFontSettingsFromString(TEXT("42,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
	propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
	propGetFontSettingsFromString(TEXT("20,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);  // 101004 18
  }
  else if (ScreenSize==(ScreenSize_t)ss480x800 ||
       (ScreenSize==0 && !ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_53) )
  {
	propGetFontSettingsFromString(TEXT("62,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
	propGetFontSettingsFromString(TEXT("62,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayBig);
	propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
	propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayMedium);
	propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
	propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlaySmall);

	propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
	propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTitle);
	propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
	propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar01);
	propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarTitle);
	propGetFontSettingsFromString(TEXT("46,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
	propGetFontSettingsFromString(TEXT("46,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTarget);
	propGetFontSettingsFromString(TEXT("46,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarValue);
	propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarUnit);
	propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar02);
	propGetFontSettingsFromString(TEXT("40,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
	propGetFontSettingsFromString(TEXT("40,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
	propGetFontSettingsFromString(TEXT("40,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig2L);
	propGetFontSettingsFromString(TEXT("40,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic2L);
	propGetFontSettingsFromString(TEXT("30,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
	propGetFontSettingsFromString(TEXT("30,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNearest);
	propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
	propGetFontSettingsFromString(TEXT("48,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
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
		propGetFontSettingsFromString(TEXT("48,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		propGetFontSettingsFromString(TEXT("48,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayBig);
		propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
		propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayMedium);
		propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
		propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlaySmall);
		propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTitle);
		propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar01);
		propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarTitle);
		propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("32,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("32,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTarget);
		propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarValue);
		propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarUnit);
		propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar02);
		propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
		propGetFontSettingsFromString(TEXT("28,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
		propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig2L);
		propGetFontSettingsFromString(TEXT("28,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic2L);
		propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
		propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNearest);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
		propGetFontSettingsFromString(TEXT("38,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
		propGetFontSettingsFromString(TEXT("26,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
		propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);
	} else {
		propGetFontSettingsFromString(TEXT("36,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontBig);
		propGetFontSettingsFromString(TEXT("36,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayBig);
		propGetFontSettingsFromString(TEXT("20,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMedium);
		propGetFontSettingsFromString(TEXT("20,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayMedium);
		propGetFontSettingsFromString(TEXT("10,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), &logfontSmall);
		propGetFontSettingsFromString(TEXT("10,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlaySmall);
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitle);
		propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTitle);
		propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMap);
		propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar01);
		propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarTitle);
		propGetFontSettingsFromString(TEXT("28,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontTarget);
		propGetFontSettingsFromString(TEXT("28,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontOverlayTarget);
		propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarValue);
		propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontBottomBarUnit);
		propGetFontSettingsFromString(TEXT("10,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar02);
		propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig);
		propGetFontSettingsFromString(TEXT("18,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic);
		propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBig2L);
		propGetFontSettingsFromString(TEXT("18,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), &logfontInfoBigItalic2L);
		propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNormal);
		propGetFontSettingsFromString(TEXT("18,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoNearest);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontInfoSmall);
		propGetFontSettingsFromString(TEXT("28,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelBig);
		propGetFontSettingsFromString(TEXT("18,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelMedium);
		propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelSmall);
		propGetFontSettingsFromString(TEXT("14,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontPanelUnit);
	}
  }


  #if TESTBENCH
  if (ScreenSize==0) StartupStore(_T("... (LKFonts) Forcing font resize%s"),NEWLINE);
  #endif

  // BUILD UP ALL-RES AUTOMATIC FONTS
  //


  // VISUAL GLIDE
  int calcsize= ScreenSizeX/(ScreenLandscape?30:22);
  // since applyfontsize will multiply, and we dont want to, we first divide to compensate.
  if (ScreenSize==0) calcsize/= Screen0Ratio;
  if (calcsize<10) calcsize=10;

  propGetFontSettingsFromString(TEXT("1,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontVisualTop);
  propGetFontSettingsFromString(TEXT("1,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontVisualBot);
  logfontVisualBot.lfHeight = calcsize;

  // Do not use demibold with tiny chars
  if (calcsize<14) {
      logfontVisualTop.lfHeight = calcsize+1;
      logfontVisualTop.lfWeight=400;
  } else {
      logfontVisualTop.lfHeight = calcsize;
      logfontVisualTop.lfWeight=600;
  }

  // CREATE STANDARD FONTS
  //
  InitializeOneFont(LK8TitleFont, logfontTitle);
  InitializeOneFont(LK8MapFont, logfontMap);
  InitializeOneFont(LK8GenericVar01Font, logfontGenericVar01);
  InitializeOneFont(LK8GenericVar02Font, logfontGenericVar02);
  InitializeOneFont(LK8TargetFont, logfontTarget);
  InitializeOneFont(LK8BigFont, logfontBig);
  InitializeOneFont(LK8MediumFont, logfontMedium);
  InitializeOneFont(LK8SmallFont, logfontSmall);
  InitializeOneFont(LK8InfoNormalFont, logfontInfoNormal);
  InitializeOneFont(LK8InfoNearestFont, logfontInfoNearest);
  InitializeOneFont(LK8InfoSmallFont, logfontInfoSmall);
  InitializeOneFont(LK8PanelBigFont, logfontPanelBig);
  InitializeOneFont(LK8PanelMediumFont, logfontPanelMedium);
  InitializeOneFont(LK8PanelSmallFont, logfontPanelSmall);
  InitializeOneFont(LK8PanelUnitFont, logfontPanelUnit);

  // CREATE CUSTOM FONTS
  //
  ApplyCustomResize(&logfontBottomBarTitle,FontBottomBar);
  InitializeOneFont(LK8BottomBarTitleFont, logfontBottomBarTitle);
  ApplyCustomResize(&logfontBottomBarValue,FontBottomBar);
  InitializeOneFont(LK8BottomBarValueFont, logfontBottomBarValue);
  ApplyCustomResize(&logfontBottomBarUnit,FontBottomBar);
  InitializeOneFont(LK8BottomBarUnitFont, logfontBottomBarUnit);
  ApplyCustomResize(&logfontInfoBig,FontInfopage1L);
  InitializeOneFont(LK8InfoBigFont, logfontInfoBig);
  ApplyCustomResize(&logfontInfoBigItalic,FontInfopage1L);
  InitializeOneFont(LK8InfoBigItalicFont, logfontInfoBigItalic);
  ApplyCustomResize(&logfontInfoBig2L,FontInfopage1L);
  InitializeOneFont(LK8InfoBig2LFont, logfontInfoBig2L);
  ApplyCustomResize(&logfontInfoBigItalic2L,FontInfopage1L);
  InitializeOneFont(LK8InfoBigItalic2LFont, logfontInfoBigItalic2L);


  ApplyCustomResize(&logfontOverlayBig,FontOverlayBig);
  InitializeOneFont(LK8OverlayBigFont, logfontOverlayBig);
  ApplyCustomResize(&logfontOverlayTitle,FontOverlayBig); // McMode follows Values
  InitializeOneFont(LK8OverlayMcModeFont, logfontOverlayTitle);
  ApplyCustomResize(&logfontOverlayTarget,FontOverlayMedium); // notice:also MapScale is scaled accordingly
  InitializeOneFont(LK8OverlayMediumFont, logfontOverlayTarget);

  InitializeOneFont(LK8OverlayGatesFont, logfontOverlayMedium);
  InitializeOneFont(LK8OverlaySmallFont, logfontOverlaySmall);

  ApplyCustomResize(&logfontVisualTop,FontVisualGlide);
  ApplyCustomResize(&logfontVisualBot,FontVisualGlide);
  InitializeOneFont(LK8VisualTopFont, logfontVisualTop);
  InitializeOneFont(LK8VisualBotFont, logfontVisualBot);


  //
  // CALCULATE BOTTOMSIZE HEIGHT
  //
  // This is relative to the code in LKDrawBottomBar.
  // We need to calculate it here because it is used by other draw functions.
  //

  LKWindowSurface windowSurface(*main_window);
  LKBitmapSurface tmpSurface(windowSurface, 1, 1);

  const auto oldFont = tmpSurface.SelectObject(LK8BottomBarTitleFont);
  const int syTitle = tmpSurface.GetTextHeight(_T("M"));;

  tmpSurface.SelectObject(LK8BottomBarValueFont);
  const int syValue = tmpSurface.GetTextHeight(_T("5"));;

  if (ScreenLandscape)
  	BottomSize = (syTitle + syValue) + 2 * NIBLSCALE(1);
  else
  	BottomSize = 2 * (syTitle + syValue) + 3 * NIBLSCALE(1);

  BottomSize += NIBLSCALE(1);
  tmpSurface.SelectObject(oldFont);
  tmpSurface.Release();


}
