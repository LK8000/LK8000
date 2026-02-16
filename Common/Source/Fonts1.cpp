/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/
#include "externs.h"
#include "LKProfiles.h"
#include "ScreenGeometry.h"
#include "utils/tokenizer.h"

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

extern void Init_Fonts_1(void);
extern void Init_Fonts_2(void);

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
// If our choice was based upon geometry criteria, and not on a custom supported resolution,
// we must ask for dynamic resize. In this case we rescale the font size.
// We use this approach to autogenerate fonts for unknown resolutions.
//
void ApplyFontSize(LOGFONT *logfont) {

  if (ScreenSize == 0) {
      logfont->lfHeight *= Screen0Ratio;
  }

#ifdef USE_FREETYPE
  if (logfont->lfWeight == 600) {
    logfont->lfHeight += 1000;
  }
#endif
}

void ApplyCustomResize(LOGFONT *logfont, short change) {
  if (change != MAXFONTRESIZE) {
    int f = change - MAXFONTRESIZE;
    logfont->lfHeight += (int)logfont->lfHeight * f / 16;
  }
}


void ApplyClearType(LOGFONT *logfont) {

  // this has to be checked on PPC and old 2002 CE devices: using ANTIALIASED quality could be better
  // 110120  .. and in fact on ppc2002 no cleartype available
  logfont->lfQuality = GetFontRenderer();
}


void InitializeOneFont (LKFont& theFont, LOGFONT logfont) {

  theFont.Release();

  ApplyClearType(&logfont);
  ApplyFontSize(&logfont);
  theFont.Create(&logfont);
}



void propGetFontSettingsFromString(const TCHAR *Buffer1, LOGFONT* lplf)
{
#define propGetFontSettingsMAX_SIZE 128
  TCHAR Buffer[propGetFontSettingsMAX_SIZE+1]; // RLD need a buffer (not sz) for strtok_r w/ gcc optimized ARM920

  TCHAR *pToken;
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

  lk::tokenizer<TCHAR> tok(Buffer);

  if ((pToken = tok.Next({_T(',')})) == NULL) return;
  lfTmp.lfHeight = _tcstol(pToken, NULL, 10);
  if ((pToken = tok.Next({_T(',')})) == NULL) return;
  lfTmp.lfWidth = _tcstol(pToken, NULL, 10);
  if ((pToken = tok.Next({_T(',')})) == NULL) return;
  lfTmp.lfEscapement = _tcstol(pToken, NULL, 10);
  if ((pToken = tok.Next({_T(',')})) == NULL) return;
  lfTmp.lfOrientation = _tcstol(pToken, NULL, 10);
  if ((pToken = tok.Next({_T(',')})) == NULL) return;
  lfTmp.lfWeight = _tcstol(pToken, NULL, 10);
  if ((pToken = tok.Next({_T(',')})) == NULL) return;
  lfTmp.lfItalic = (unsigned char)_tcstol(pToken, NULL, 10);
  if ((pToken = tok.Next({_T(',')})) == NULL) return;
  lfTmp.lfUnderline = (unsigned char)_tcstol(pToken, NULL, 10);
  if ((pToken = tok.Next({_T(',')})) == NULL) return;
  lfTmp.lfStrikeOut = (unsigned char)_tcstol(pToken, NULL, 10);
  if ((pToken = tok.Next({_T(',')})) == NULL) return;
  lfTmp.lfCharSet = (unsigned char)_tcstol(pToken, NULL, 10);
  if ((pToken = tok.Next({_T(',')})) == NULL) return;
  lfTmp.lfOutPrecision = (unsigned char)_tcstol(pToken, NULL, 10);
  if ((pToken = tok.Next({_T(',')})) == NULL) return;
  lfTmp.lfClipPrecision = (unsigned char)_tcstol(pToken, NULL, 10);

  // FIXFONTS possible multiplier for FT_Set_Pixel in place of Set_Char_Size
  // double mulp=(8.7 / 10.0)*100;
  // lfTmp.lfHeight = (int)((lfTmp.lfHeight*mulp)/100.0);

  // DEFAULT_QUALITY			   0
  // RASTER_FONTTYPE			   0x0001
  // DRAFT_QUALITY			     1
  // NONANTIALIASED_QUALITY  3
  // ANTIALIASED_QUALITY     4
  // CLEARTYPE_QUALITY       5
  // CLEARTYPE_COMPAT_QUALITY 6

  if ((pToken = tok.Next({_T(',')})) == NULL) return;
  ApplyClearType(&lfTmp);

  if ((pToken = tok.Next({_T(',')})) == NULL) return;
  lfTmp.lfPitchAndFamily = (unsigned char)_tcstol(pToken, NULL, 10);

  if ((pToken = tok.Next({_T(',')})) == NULL) return;

  lk::strcpy(lfTmp.lfFaceName, pToken);

  memcpy((void *)lplf, (void *)&lfTmp, sizeof (LOGFONT));

  return;
}


void InitLKFonts() {
  Init_Fonts_1();
  Init_Fonts_2();
}

//
// Historically we had two font configurations. For simplicity, we keep the two setups
// still separated, since they are very long. The only drawback is that the deinit is in common.
//
void Init_Fonts_1() {
  TestLog(_T(". Init_Fonts_1"));

  LOGFONT logfontTitleWindowLogFont;
  LOGFONT logfontMapWindowLogFont;
  LOGFONT logfontMapWindowBoldLogFont;
  LOGFONT logfontButtonFont;
  LOGFONT logfontCDIWindowLogFont;
  LOGFONT logfontGenericVar03Font;
  LOGFONT logfontMapWaypointFont;
  LOGFONT logfontMapWaypointBoldFont;
  LOGFONT logfontMapTopologyFont;
  LOGFONT logfontMapScaleFont;
  LOGFONT logfontCustom1Font;

  memset ((char *)&logfontTitleWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&logfontMapWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&logfontMapWindowBoldLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&logfontButtonFont, 0, sizeof (LOGFONT));
  memset ((char *)&logfontCDIWindowLogFont, 0, sizeof (LOGFONT));
  memset ((char *)&logfontGenericVar03Font, 0, sizeof (LOGFONT));
  memset ((char *)&logfontMapWaypointFont, 0, sizeof (LOGFONT));
  memset ((char *)&logfontMapWaypointBoldFont, 0, sizeof (LOGFONT));
  memset ((char *)&logfontMapTopologyFont, 0, sizeof (LOGFONT));
  memset ((char *)&logfontMapScaleFont, 0, sizeof (LOGFONT));
  memset ((char *)&logfontCustom1Font, 0, sizeof (LOGFONT));



/* Work in progress... following not accurate
 * TitleWindowFont	= Font=0 in dialogs, used also in dlStartup rawwrite. Easily removable.
 * CDIWindowFont	= Font=3 and Font=4 in dialogs
 * LK8GenericVar03Font		= Stats, map labels
 * MapWindowFont	=
 * MapWindowBoldFont	= menu buttons, waypoint selection, messages, etc.


   ------------------------------------------------------------------------------------
    FOLLOWING FONTS ARE USER RESIZABLE. DO NOT USE - RESERVED FOR THEIR OWN PAGES
   ------------------------------------------------------------------------------------

   MapWaypoint                map waypoint names
   MapWaypointBold            map waypoint names
   MapTopology                map topology names
   MapScaleFont               map scale and ibox indicators 3.5km AUX9 .. overlay
   LK8InfoBigFont             nearest pages, values
   LK8InfoBigItalicFont          "
   LK8InfoBig2LFont           nearest pages, values for UseTwoLines
   LK8InfoBigItalic2LFont        "

   LK8BottomBarTitleFont      the bottom bar
   LK8BottomBarValueFont
   LK8BottomBarUnitFont

   LK8OverlayBigFont          map overlays
   LK8OverlayMediumtFont
   LK8OverlaySmallFont
   LK8OverlayGatesFont
   LK8OverlayMcModeFont

   LK8VisualTop               visualglide
   LK8VisualBot

   Custom1Font	              available, not yet used


 */

  //
  // LANDSCAPE
  //
   // If you set a font here for a specific resolution, no automatic font generation is used.
  if ( (ScreenSize==(ScreenSize_t)ss480x272) ||
       (ScreenSize==0 && ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_169) )
  { // WQVGA  e.g. MIO
    propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), &logfontTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,800,0,0,0,0,0,0,4,2,Tahoma"), &logfontCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("16,0,0,0,600,0,0,0,0,0,0,4,2,Tahoma"), &logfontGenericVar03Font);
    propGetFontSettingsFromString(TEXT("16,0,0,0,600,0,0,0,0,0,0,4,2,Tahoma"), &logfontMapTopologyFont);
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), &logfontCustom1Font);
    propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), &logfontMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("19,0,0,0,600,0,0,0,0,0,0,6,2,Tahoma"), &logfontMapWaypointBoldFont);
    propGetFontSettingsFromString(TEXT("19,0,0,0,400,0,0,0,0,0,0,6,2,Tahoma"), &logfontMapWindowBoldLogFont);
    propGetFontSettingsFromString(TEXT("21,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), &logfontMapWaypointFont);
    propGetFontSettingsFromString(TEXT("20,0,0,0,600,0,0,0,0,0,0,4,2,Tahoma"), &logfontMapScaleFont);
  }
  else if ( ScreenSize==(ScreenSize_t)ss480x234 ||
       (ScreenSize==0 && ScreenGeometry==SCREEN_GEOMETRY_21) )
  { // e.g. Messada 2440
    propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("16,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar03Font);
    propGetFontSettingsFromString(TEXT("16,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), &logfontMapTopologyFont);
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontCustom1Font);
    propGetFontSettingsFromString(TEXT("20,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapScaleFont);
    propGetFontSettingsFromString(TEXT("20,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("20,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointBoldFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss800x480 ||
       (ScreenSize==0 && ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_53) )
  {
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar03Font);
    propGetFontSettingsFromString(TEXT("26,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapTopologyFont);
    propGetFontSettingsFromString(TEXT("48,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontCustom1Font);
    propGetFontSettingsFromString(TEXT("30,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapScaleFont);
    propGetFontSettingsFromString(TEXT("30,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("30,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointBoldFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss800x600 )
  {
    propGetFontSettingsFromString(TEXT("24,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("30,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar03Font);
    propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapTopologyFont);
    propGetFontSettingsFromString(TEXT("25,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontCustom1Font);
    propGetFontSettingsFromString(TEXT("30,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapScaleFont);
    propGetFontSettingsFromString(TEXT("40,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("34,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointBoldFont);
    propGetFontSettingsFromString(TEXT("35,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss400x240) {
    propGetFontSettingsFromString(TEXT("10,0,0,0,200,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar03Font);
    propGetFontSettingsFromString(TEXT("15,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapTopologyFont);
    propGetFontSettingsFromString(TEXT("24,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontCustom1Font);
    propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapScaleFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointBoldFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss640x480 ||
       (ScreenSize==0 && ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_43) )
  {
    propGetFontSettingsFromString(TEXT("19,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("24,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("26,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar03Font);
    propGetFontSettingsFromString(TEXT("26,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontMapTopologyFont);
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontCustom1Font);
    propGetFontSettingsFromString(TEXT("32,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapScaleFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointFont);
    propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointBoldFont);
    propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss320x240) {
    propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("14,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("14,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar03Font);
    propGetFontSettingsFromString(TEXT("14,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapTopologyFont);
    propGetFontSettingsFromString(TEXT("10,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontCustom1Font);
    propGetFontSettingsFromString(TEXT("17,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapScaleFont);
    propGetFontSettingsFromString(TEXT("17,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("17,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointBoldFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowBoldLogFont);
  }
  //
  // PORTRAIT MODE
  //
  else if (ScreenSize==(ScreenSize_t)ss240x320) {
    propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("12,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("13,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar03Font);
    propGetFontSettingsFromString(TEXT("13,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapTopologyFont);
    propGetFontSettingsFromString(TEXT("10,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontCustom1Font);
    propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapScaleFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointFont);
    propGetFontSettingsFromString(TEXT("16,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointBoldFont);
    propGetFontSettingsFromString(TEXT("16,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss272x480 ||
       (ScreenSize==0 && !ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_169) )
  {
    propGetFontSettingsFromString(TEXT("12,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("15,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar03Font);
    propGetFontSettingsFromString(TEXT("15,0,0,0,600,1,0,0,0,0,0,3,2,Tahoma"), &logfontMapTopologyFont);
    propGetFontSettingsFromString(TEXT("10,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontCustom1Font);
    propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapScaleFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,600,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointBoldFont);
    propGetFontSettingsFromString(TEXT("18,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss480x640 ||
       (ScreenSize==0 && !ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_43) )
  {
    propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("26,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("23,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar03Font);
    propGetFontSettingsFromString(TEXT("23,0,0,0,100,1,0,0,0,0,0,3,2,Tahoma"), &logfontMapTopologyFont);
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontCustom1Font);
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapScaleFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointFont);
    propGetFontSettingsFromString(TEXT("28,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointBoldFont);
    propGetFontSettingsFromString(TEXT("28,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss480x800 ||
       (ScreenSize==0 && !ScreenLandscape && ScreenGeometry==SCREEN_GEOMETRY_53) )
  {
    propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("26,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("23,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar03Font);
    propGetFontSettingsFromString(TEXT("23,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapTopologyFont);
    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontCustom1Font);
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapScaleFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointFont);
    propGetFontSettingsFromString(TEXT("30,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointBoldFont);
    propGetFontSettingsFromString(TEXT("30,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowBoldLogFont);
  }
  else if (ScreenSize==(ScreenSize_t)ss600x800 )
  {
    propGetFontSettingsFromString(TEXT("28,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleWindowLogFont);
    propGetFontSettingsFromString(TEXT("33,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontCDIWindowLogFont);
    propGetFontSettingsFromString(TEXT("29,0,0,0,100,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar03Font);
    propGetFontSettingsFromString(TEXT("26,0,0,0,400,1,0,0,0,0,0,3,2,Tahoma"), &logfontMapTopologyFont);
    propGetFontSettingsFromString(TEXT("25,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontCustom1Font);
    propGetFontSettingsFromString(TEXT("28,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapScaleFont);
    propGetFontSettingsFromString(TEXT("40,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowLogFont);
    propGetFontSettingsFromString(TEXT("32,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointFont);
    propGetFontSettingsFromString(TEXT("28,0,0,0,800,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointBoldFont);
    propGetFontSettingsFromString(TEXT("36,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowBoldLogFont);
  }
  //
  // ELSE WE DID NOT FIND A VALID CUSTOM RESOLUTION OR A VALID SCREEN GEOMETRY FOR THIS ORIENTATION!
  // We use 16:9 480x272 settings, but no warranty! No boldness.
  //
  else {
    StartupStore(_T(". >> (Fonts) Unknown unsupported screen geometry or resolution%s"),NEWLINE);
    if (ScreenLandscape) {
	    propGetFontSettingsFromString(TEXT("14,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), &logfontTitleWindowLogFont);
	    propGetFontSettingsFromString(TEXT("15,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), &logfontCDIWindowLogFont);
	    propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), &logfontGenericVar03Font);
	    propGetFontSettingsFromString(TEXT("16,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), &logfontMapTopologyFont);
	    propGetFontSettingsFromString(TEXT("20,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), &logfontCustom1Font);
	    propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), &logfontMapScaleFont);
	    propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), &logfontMapWindowLogFont);
	    propGetFontSettingsFromString(TEXT("22,0,0,0,400,0,0,0,0,0,0,4,2,Tahoma"), &logfontMapWaypointFont);
	    propGetFontSettingsFromString(TEXT("19,0,0,0,400,0,0,0,0,0,0,6,2,Tahoma"), &logfontMapWaypointBoldFont);
	    propGetFontSettingsFromString(TEXT("19,0,0,0,400,0,0,0,0,0,0,6,2,Tahoma"), &logfontMapWindowBoldLogFont);
    } else {
	    propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontTitleWindowLogFont);
	    propGetFontSettingsFromString(TEXT("12,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontCDIWindowLogFont);
	    propGetFontSettingsFromString(TEXT("15,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontGenericVar03Font);
	    propGetFontSettingsFromString(TEXT("15,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapTopologyFont);
	    propGetFontSettingsFromString(TEXT("10,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontCustom1Font);
	    propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapScaleFont);
	    propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowLogFont);
	    propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointFont);
	    propGetFontSettingsFromString(TEXT("18,0,0,0,400,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWaypointBoldFont);
      propGetFontSettingsFromString(TEXT("15,0,0,0,500,0,0,0,0,0,0,3,2,Tahoma"), &logfontMapWindowBoldLogFont);
    }
  }


  // CREATE STANDARD FONTS
  //
  logfontButtonFont = logfontMapWindowBoldLogFont;
  if (logfontButtonFont.lfHeight > 0)
    logfontButtonFont.lfHeight += 2;
  else
    logfontButtonFont.lfHeight -= 2;

  InitializeOneFont (TitleWindowFont, logfontTitleWindowLogFont);
  InitializeOneFont (CDIWindowFont, logfontCDIWindowLogFont);
  InitializeOneFont (LK8GenericVar03Font, logfontGenericVar03Font);
  InitializeOneFont (MapWindowFont, logfontMapWindowLogFont);
  InitializeOneFont (MapWindowBoldFont, logfontMapWindowBoldLogFont);
  InitializeOneFont (LK8ButtonFont, logfontButtonFont);

  // CREATE CUSTOM FONTS
  //
  ApplyCustomResize(&logfontMapWaypointFont,FontMapWaypoint);
  InitializeOneFont (MapWaypointFont, logfontMapWaypointFont);
  ApplyCustomResize(&logfontMapWaypointBoldFont,FontMapWaypoint);
  InitializeOneFont (MapWaypointBoldFont, logfontMapWaypointBoldFont);
  ApplyCustomResize(&logfontMapTopologyFont,FontMapTopology);
  InitializeOneFont (MapTopologyFont, logfontMapTopologyFont);
  ApplyCustomResize(&logfontMapScaleFont,FontOverlayMedium); // follows OverlayMedium
  InitializeOneFont (MapScaleFont, logfontMapScaleFont);
  ApplyCustomResize(&logfontCustom1Font,FontCustom1);
  InitializeOneFont (Custom1Font, logfontCustom1Font);


  main_window->SetFont(MapWindowFont);
}



//
// REMINDER> DO NOT FORGET TO UPDATE THIS WHEN A FONT IS ADDED!!
//
void DeInitLKFonts() {

  TitleWindowFont.Release();
  MapWindowFont.Release();
  MapWindowBoldFont.Release();
  LK8ButtonFont.Release();
  CDIWindowFont.Release();
  LK8GenericVar03Font.Release();
  MapWaypointFont.Release();
  MapWaypointBoldFont.Release();
  MapTopologyFont.Release();
  MapScaleFont.Release();
  Custom1Font.Release();

  LK8TitleFont.Release();
  LK8MapFont.Release();
  LK8GenericVar01Font.Release();
  LK8GenericVar02Font.Release();
  LK8TargetFont.Release();
  LK8BigFont.Release();
  LK8MediumFont.Release();
  LK8SmallFont.Release();
  LK8InfoBigFont.Release();
  LK8InfoBigItalicFont.Release();
  LK8InfoBig2LFont.Release();
  LK8InfoBigItalic2LFont.Release();
  LK8InfoNormalFont.Release();
  LK8InfoNearestFont.Release();
  LK8InfoSmallFont.Release();
  LK8PanelBigFont.Release();
  LK8PanelMediumFont.Release();
  LK8PanelSmallFont.Release();
  LK8PanelUnitFont.Release();
  LK8BottomBarTitleFont.Release();
  LK8BottomBarValueFont.Release();
  LK8BottomBarUnitFont.Release();

  LK8OverlayBigFont.Release();
  LK8OverlayMediumFont.Release();
  LK8OverlaySmallFont.Release();
  LK8OverlayGatesFont.Release();
  LK8OverlayMcModeFont.Release();

  LK8VisualTopFont.Release();
  LK8VisualBotFont.Release();

}
