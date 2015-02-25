/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "LKProfiles.h"
#include "ScreenGeometry.h"

#ifndef DEFAULT_QUALITY
#define DEFAULT_QUALITY 0
#endif

#ifndef NONANTIALIASED_QUALITY
#define NONANTIALIASED_QUALITY 3
#endif

#ifndef ANTIALIASED_QUALITY
#define ANTIALIASED_QUALITY 4
#endif

#define CLEARTYPE_COMPAT_QUALITY 	6
int GetFontRenderer() { 

  switch(FontRenderer) {
	case 0:
		return CLEARTYPE_COMPAT_QUALITY;
		break;
	case 1:
		return ANTIALIASED_QUALITY;
		break;
	case 2:
		return DEFAULT_QUALITY;
		break;
	case 3:
		return NONANTIALIASED_QUALITY;
		break;
	default:
		return CLEARTYPE_COMPAT_QUALITY;
		break;
  }
}


//
// We rescale the font size assuming if it was marked as rescalable
//
void ApplyFontSize(LOGFONT *logfont) {

  if (logfont->lfOrientation == 0) return;
  logfont->lfOrientation = 0;
  logfont->lfHeight = (int)((double)logfont->lfHeight * Screen0Ratio);

}

void ApplyClearType(LOGFONT *logfont) {

  // this has to be checked on PPC and old 2002 CE devices: using ANTIALIASED quality could be better
  // 110120  .. and in fact on ppc2002 no cleartype available
  logfont->lfQuality = GetFontRenderer();
}


bool IsNullLogFont(LOGFONT logfont) {
  bool bRetVal=false;

  LOGFONT LogFontBlank;
  memset ((char *)&LogFontBlank, 0, sizeof (LOGFONT));

  if ( memcmp(&logfont, &LogFontBlank, sizeof(LOGFONT)) == 0) {
    bRetVal=true;
  }
  return bRetVal;
}


void InitializeOneFont (LKFont& theFont, 
                               const char FontRegKey[] , 
                               LOGFONT autoLogFont, 
                               LOGFONT * LogFontUsed)
{
  theFont.Release();

  LOGFONT logfont;
  memset ((char *)&logfont, 0, sizeof (LOGFONT));

  if (UseCustomFonts) {
    propGetFontSettings(FontRegKey, &logfont);
    if (!IsNullLogFont(logfont)) {
      theFont.Create(&logfont);
      if (LogFontUsed != NULL) {
        *LogFontUsed = logfont; // RLD save for custom font GUI
      }
    }
  }

  if (!IsNullLogFont(autoLogFont)) {
      ApplyClearType(&autoLogFont);
      ApplyFontSize(&autoLogFont);
      theFont.Create(&autoLogFont);
      if (LogFontUsed != NULL) {
          *LogFontUsed = autoLogFont; // RLD save for custom font GUI
      }
  }
}


void InitialiseFontsHardCoded(RECT rc,
                        LOGFONT * ptrhardTitleWindowLogFont,
                        LOGFONT * ptrhardMapWindowLogFont,
                        LOGFONT * ptrhardMapWindowBoldLogFont,
                        LOGFONT * ptrhardCDIWindowLogFont, // New
                        LOGFONT * ptrhardMapLabelLogFont,
                        LOGFONT * ptrhardStatisticsLogFont) {



  memset ((char *)ptrhardTitleWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrhardMapWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrhardMapWindowBoldLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrhardCDIWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrhardMapLabelLogFont, 0, sizeof (LOGFONT));
  memset ((char *)ptrhardStatisticsLogFont, 0, sizeof (LOGFONT));


/*
 * TitleWindowFont	= Font=0 in dialogs, used also in dlStartup rawwrite. Easily removable.
 * CDIWindowFont	= Font=3 and Font=4 in dialogs
 * StatisticsFont	= only in Statistics pages
 * MapLabelFont		= Stats, map labels
 * MapWindowFont	= text names on the map, zoom indicator
 * MapWindowBoldFont	= menu buttons, waypoint selection, messages, etc.
 *
 * Only MapWindow and MapLabel can be customized as of october 2011
 */

  //
  // LANDSCAPE
  //
   // If you set a font here for a specific resolution, no automatic font generation is used.
  if ( (ScreenSize==(ScreenSize_t)ss480x272) || 
       (ScreenSize==0 && ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_169) )
  { // WQVGA  e.g. MIO
    propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,400,0,0,0,0,0,0,4,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("16,0,0,0,600,0,0,0,0,0,0,4,2,Tahoma"), ptrhardMapLabelLogFont);
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("19,0,0,0,500,0,0,0,0,0,0,6,2,Tahoma"), ptrhardMapWindowBoldLogFont); 
  }
  else if (ScreenSize==(ScreenSize_t)ss720x408) { // WQVGA  e.g. MIO
    propGetFontSettingsFromString(TEXT("21,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("23,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("23,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont);
    propGetFontSettingsFromString(TEXT("30,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("33,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("30,0,0,0,700,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardMapWindowBoldLogFont);
  }
  else if ( ScreenSize==(ScreenSize_t)ss480x234 ||
       (ScreenSize==0 && ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_21) )
  { // e.g. Messada 2440
    propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("16,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); 
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("20,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss800x480 ||
       (ScreenSize==0 && ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_53) )
  {
    propGetFontSettingsFromString(TEXT("20,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); 
    propGetFontSettingsFromString(TEXT("48,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("30,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss400x240) {
    propGetFontSettingsFromString(TEXT("10,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont);
    propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss640x480 ||
       (ScreenSize==0 && ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_43) )
  { 
    propGetFontSettingsFromString(TEXT("19,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("26,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont);
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("28,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss896x672) { 
    propGetFontSettingsFromString(TEXT("25,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("33,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont);
    propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("44,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("39,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss320x240) { 
    propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("14,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont);
    propGetFontSettingsFromString(TEXT("10,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("17,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);
  } 
  // 
  // PORTRAIT MODE
  //
  else if (ScreenSize==(ScreenSize_t)ss240x320) { 
    propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("13,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); 
    propGetFontSettingsFromString(TEXT("10,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("16,0,0,0,500,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss272x480 ||
       (ScreenSize==0 && !ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_169) )
  {
    propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); 
    propGetFontSettingsFromString(TEXT("10,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss480x640 ||
       (ScreenSize==0 && !ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_43) )
  {
    propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("26,0,0,0,100,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("23,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont);
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont); 
    propGetFontSettingsFromString(TEXT("28,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont); 
  }
  else if (ScreenSize==(ScreenSize_t)ss480x800 ||
       (ScreenSize==0 && !ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_53) )
  {
    propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("26,0,0,0,100,0,0,0,0,0,0,3,2,TahomaBD"), ptrhardCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("23,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont);
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("30,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);
  }
  //
  // ELSE WE DID NOT FIND A VALID CUSTOM RESOLUTION OR A VALID SCREEN GEOMETRY FOR THIS ORIENTATION!
  // We use 16:9 480x272 settings, but no warranty!
  //
  else { 
    StartupStore(_T(". >> (Fonts) Unknown unsupported screen geometry or resolution%s"),NEWLINE);
    if (ScreenLandscape) {
	    propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), ptrhardTitleWindowLogFont);
	    propGetFontSettingsFromString(TEXT("15,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), ptrhardCDIWindowLogFont);
	    propGetFontSettingsFromString(TEXT("16,0,0,0,600,0,0,0,0,0,0,4,2,Tahoma"), ptrhardMapLabelLogFont);
	    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), ptrhardStatisticsLogFont);
	    propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), ptrhardMapWindowLogFont);
	    propGetFontSettingsFromString(TEXT("19,0,0,0,500,0,0,0,0,0,0,6,2,Tahoma"), ptrhardMapWindowBoldLogFont); 
    } else {
	    propGetFontSettingsFromString(TEXT("12,0,0,0,300,0,0,0,0,0,0,3,2,Tahoma"), ptrhardTitleWindowLogFont);
	    propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardCDIWindowLogFont);
	    propGetFontSettingsFromString(TEXT("15,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapLabelLogFont); 
	    propGetFontSettingsFromString(TEXT("10,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), ptrhardStatisticsLogFont);
	    propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowLogFont);
	    propGetFontSettingsFromString(TEXT("18,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), ptrhardMapWindowBoldLogFont);
    }
  }

  //
  // If our choice was based upon geometry criteria, and not on a custom supported resolution,
  // we must ask for dynamic resize. So we mark Orientation flag to 1 .
  // We use this flag to autogenerate fonts for unknown resolutions.
  // We do the same in LKFonts.
  //
  if (ScreenSize==0) {
      #if TESTBENCH
      StartupStore(_T("... (Fonts) Forcing font resize%s"),NEWLINE);
      #endif
      ptrhardTitleWindowLogFont->lfOrientation = 1;
      ptrhardCDIWindowLogFont->lfOrientation = 1;
      ptrhardMapLabelLogFont->lfOrientation = 1;
      ptrhardStatisticsLogFont->lfOrientation = 1;
      ptrhardMapWindowLogFont->lfOrientation = 1;
      ptrhardMapWindowBoldLogFont->lfOrientation = 1;
  }



}

void DeInitialiseFonts(void) {

  TitleWindowFont.Release();
  MapWindowFont.Release();
  MapWindowBoldFont.Release();
  CDIWindowFont.Release();
  MapLabelFont.Release();
  StatisticsFont.Release();

  LK8UnitFont.Release();
  LK8TitleFont.Release();
  LK8MapFont.Release();
  LK8TitleNavboxFont.Release();
  LK8ValueFont.Release();
  LK8TargetFont.Release();
  LK8BigFont.Release();
  LK8MediumFont.Release();
  LK8SmallFont.Release();
  LK8InfoBigFont.Release();
  LK8InfoBigItalicFont.Release();
  LK8InfoNormalFont.Release();
  LK8InfoSmallFont.Release();
  LK8PanelBigFont.Release();
  LK8PanelMediumFont.Release();
  LK8PanelSmallFont.Release();
  LK8PanelUnitFont.Release();
  
}

void InitialiseFonts(RECT rc)
{ //this routine must be called only at start/restart b/c there are many pointers to these fonts

  TitleWindowFont.Release();
  MapWindowFont.Release();
  MapWindowBoldFont.Release();
  CDIWindowFont.Release();
  MapLabelFont.Release();
  StatisticsFont.Release();

  LOGFONT hardTitleWindowLogFont;
  LOGFONT hardMapWindowLogFont;
  LOGFONT hardMapWindowBoldLogFont;
  LOGFONT hardCDIWindowLogFont; 
  LOGFONT hardMapLabelLogFont;
  LOGFONT hardStatisticsLogFont;

  memset ((char *)&hardTitleWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardMapWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardMapWindowBoldLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardCDIWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardMapLabelLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&hardStatisticsLogFont, 0, sizeof (LOGFONT));

  InitialiseFontsHardCoded(rc,
                        &hardTitleWindowLogFont,
                        &hardMapWindowLogFont,
                        &hardMapWindowBoldLogFont,
                        &hardCDIWindowLogFont, // New
                        &hardMapLabelLogFont,
                        &hardStatisticsLogFont);

  //
  // Merge the "hard" into the "auto" if one exists 
  //


  if (!IsNullLogFont(hardTitleWindowLogFont))
    autoTitleWindowLogFont = hardTitleWindowLogFont;

  if (!IsNullLogFont(hardMapWindowLogFont))
    autoMapWindowLogFont = hardMapWindowLogFont;


  if (!IsNullLogFont(hardMapWindowBoldLogFont))
    autoMapWindowBoldLogFont = hardMapWindowBoldLogFont;

  if (!IsNullLogFont(hardCDIWindowLogFont))
    autoCDIWindowLogFont = hardCDIWindowLogFont;

  if (!IsNullLogFont(hardMapLabelLogFont))
    autoMapLabelLogFont = hardMapLabelLogFont;

  if (!IsNullLogFont(hardStatisticsLogFont))
    autoStatisticsLogFont = hardStatisticsLogFont;


  InitializeOneFont (TitleWindowFont, 
                        NULL,
                        autoTitleWindowLogFont,
                        NULL);

  InitializeOneFont (CDIWindowFont, 
                        NULL,
                        autoCDIWindowLogFont,
                        NULL);

  InitializeOneFont (MapLabelFont, 
                        szRegistryFontMapLabelFont, 
                        autoMapLabelLogFont,
                        NULL);

  InitializeOneFont (StatisticsFont, 
                        NULL,
                        autoStatisticsLogFont,
                        NULL);

  InitializeOneFont (MapWindowFont, 
                        szRegistryFontMapWindowFont, 
                        autoMapWindowLogFont,
                        NULL);

  InitializeOneFont (MapWindowBoldFont, 
                        NULL,
                        autoMapWindowBoldLogFont,
                        NULL);

  MainWindow.SetFont(MapWindowFont);
}


void propGetFontSettingsFromString(const TCHAR *Buffer1, LOGFONT* lplf)
{
#define propGetFontSettingsMAX_SIZE 128
  TCHAR Buffer[propGetFontSettingsMAX_SIZE+1]; // RLD need a buffer (not sz) for strtok_r w/ gcc optimized ARM920

  TCHAR *pWClast, *pToken;
  LOGFONT lfTmp;
  LK_tcsncpy(Buffer, Buffer1, propGetFontSettingsMAX_SIZE);
    // FontDescription of format:
    // typical font entry
    // 26,0,0,0,700,1,0,0,0,0,0,4,2,<fontname>

    //FW_THIN   100
    //FW_NORMAL 400
    //FW_MEDIUM 500
    //FW_BOLD   700
    //FW_HEAVY  900

  LKASSERT(lplf != NULL);
  memset ((void *)&lfTmp, 0, sizeof (LOGFONT));

  if ((pToken = _tcstok_r(Buffer, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfHeight = _tcstol(pToken, NULL, 10);
  if ((pToken = _tcstok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfWidth = _tcstol(pToken, NULL, 10);
  if ((pToken = _tcstok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfEscapement = _tcstol(pToken, NULL, 10);
  if ((pToken = _tcstok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfOrientation = _tcstol(pToken, NULL, 10);
  if ((pToken = _tcstok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfWeight = _tcstol(pToken, NULL, 10);
  if ((pToken = _tcstok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfItalic = (unsigned char)_tcstol(pToken, NULL, 10);
  if ((pToken = _tcstok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfUnderline = (unsigned char)_tcstol(pToken, NULL, 10);
  if ((pToken = _tcstok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfStrikeOut = (unsigned char)_tcstol(pToken, NULL, 10);
  if ((pToken = _tcstok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfCharSet = (unsigned char)_tcstol(pToken, NULL, 10);
  if ((pToken = _tcstok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfOutPrecision = (unsigned char)_tcstol(pToken, NULL, 10);
  if ((pToken = _tcstok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfClipPrecision = (unsigned char)_tcstol(pToken, NULL, 10);

  // DEFAULT_QUALITY			   0
  // RASTER_FONTTYPE			   0x0001
  // DRAFT_QUALITY			     1
  // NONANTIALIASED_QUALITY  3
  // ANTIALIASED_QUALITY     4
  // CLEARTYPE_QUALITY       5
  // CLEARTYPE_COMPAT_QUALITY 6

  if ((pToken = _tcstok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  ApplyClearType(&lfTmp);

  if ((pToken = _tcstok_r(NULL, TEXT(","), &pWClast)) == NULL) return;
  lfTmp.lfPitchAndFamily = (unsigned char)_tcstol(pToken, NULL, 10);

  if ((pToken = _tcstok_r(NULL, TEXT(","), &pWClast)) == NULL) return;

  _tcscpy(lfTmp.lfFaceName, pToken);

  memcpy((void *)lplf, (void *)&lfTmp, sizeof (LOGFONT));

  return;
}


void propGetFontSettings(const char *Name, LOGFONT* lplf) {

 // Load custom font settings from profile only if relative to
 // configurable fonts, of course.
 if (Name==NULL || lplf==NULL) return;
 if ( !strcmp(Name,"MapLabelFont") ) {
	if (_tcslen(FontDesc_MapLabel)>0)
		propGetFontSettingsFromString(FontDesc_MapLabel, lplf);
 }
 if ( !strcmp(Name,"MapWindowFont") ) {
	if (_tcslen(FontDesc_MapWindow)>0)
		propGetFontSettingsFromString(FontDesc_MapWindow, lplf);
 }
}

