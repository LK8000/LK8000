/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Utils2.cpp,v 8.31 2010/12/13 00:55:29 root Exp root $
*/

#include "StdAfx.h"
#include <stdio.h>
#include "options.h"
#include "externs.h"
#include "lk8000.h"
#include "McReady.h"
#include "RGB.h"
#include "Modeltype.h"

using std::min;
using std::max;


// colorcode is taken from a 5 bit AsInt union
void MapWindow::TextColor(HDC hDC, short colorcode) {

	switch (colorcode) {
	case TEXTBLACK: 
		if (BlackScreen) // 091109
		  SetTextColor(hDC,RGB_WHITE);  // black 
		else
		  SetTextColor(hDC,RGB_BLACK);  // black 
	  break;
	case TEXTWHITE: 
		if (BlackScreen) // 091109
		  SetTextColor(hDC,RGB_LIGHTYELLOW);  // white
		else
		  SetTextColor(hDC,RGB_WHITE);  // white
	  break;
	case TEXTGREEN: 
	  SetTextColor(hDC,RGB_GREEN);  // green
	  break;
	case TEXTRED:
	  SetTextColor(hDC,RGB_RED);  // red
	  break;
	case TEXTBLUE:
	  SetTextColor(hDC,RGB_BLUE);  // blue
	  break;
	case TEXTYELLOW:
	  SetTextColor(hDC,RGB_YELLOW);  // yellow
	  break;
	case TEXTCYAN:
	  SetTextColor(hDC,RGB_CYAN);  // cyan
	  break;
	case TEXTMAGENTA:
	  SetTextColor(hDC,RGB_MAGENTA);  // magenta
	  break;
	case TEXTLIGHTGREY: 
	  SetTextColor(hDC,RGB_LIGHTGREY);  // light grey
	  break;
	case TEXTGREY: 
	  SetTextColor(hDC,RGB_GREY);  // grey
	  break;
	case TEXTLIGHTGREEN:
	  SetTextColor(hDC,RGB_LIGHTGREEN);  //  light green
	  break;
	case TEXTLIGHTRED:
	  SetTextColor(hDC,RGB_LIGHTRED);  // light red
	  break;
	case TEXTLIGHTYELLOW:
	  SetTextColor(hDC,RGB_LIGHTYELLOW);  // light yellow
	  break;
	case TEXTLIGHTCYAN:
	  SetTextColor(hDC,RGB_LIGHTCYAN);  // light cyan
	  break;
	case TEXTORANGE:
	  SetTextColor(hDC,RGB_ORANGE);  // orange
	  break;
	case TEXTLIGHTORANGE:
	  SetTextColor(hDC,RGB_LIGHTORANGE);  // light orange
	  break;
	case TEXTLIGHTBLUE:
	  SetTextColor(hDC,RGB_LIGHTBLUE);  // light blue
	  break;
	default:
	  SetTextColor(hDC,RGB_MAGENTA);  // magenta so we know it's wrong: nobody use magenta..
	  break;
	}

}



// Get the infobox type from configuration, selecting position i
// From 1-8 auxiliaries
//     0-16 dynamic page
//
int GetInfoboxType(int i) {

	int retval = 0;
	if (i<1||i>16) return LK_ERROR;

	// it really starts from 0
	if (i<=8)
		retval = (InfoType[i-1] >> 24) & 0xff; // auxiliary
	else {
		switch ( MapWindow::mode.Fly() ) {
			case MapWindow::Mode::MODE_FLY_CRUISE:
				retval = (InfoType[i-9] >> 8) & 0xff;
				break;
			case MapWindow::Mode::MODE_FLY_FINAL_GLIDE:
				retval = (InfoType[i-9] >> 16) & 0xff;
				break;
			case MapWindow::Mode::MODE_FLY_CIRCLING:
				retval = (InfoType[i-9]) & 0xff; 
				break;
			default:
				// impossible case, show twice auxiliaries
				retval = (InfoType[i-9] >> 24) & 0xff;
				break;
		}
	}

	return min(NumDataOptions-1,retval);
}

// Returns the LKProcess index value for configured infobox (0-8) for dmCruise, dmFinalGlide, Auxiliary, dmCircling
// The function name is really stupid...
// dmMode is an enum, we simply use for commodity
int GetInfoboxIndex(int i, MapWindow::Mode::TModeFly dmMode) {
	int retval = 0;
	if (i<0||i>8) return LK_ERROR;

	switch(dmMode) {
		case MapWindow::Mode::MODE_FLY_CRUISE:
			retval = (InfoType[i-1] >> 8) & 0xff;
			break;
		case MapWindow::Mode::MODE_FLY_FINAL_GLIDE:
			retval = (InfoType[i-1] >> 16) & 0xff;
			break;
		case MapWindow::Mode::MODE_FLY_CIRCLING:
			retval = (InfoType[i-1]) & 0xff; 
			break;
		default:
			// default is auxiliary
			retval = (InfoType[i-1] >> 24) & 0xff; 
			break;
	}
	return min(NumDataOptions-1,retval);
}


double GetMacCready(int wpindex, short wpmode)
{
	if (WayPointCalc[wpindex].IsLandable) {
		if (MACCREADY>GlidePolar::SafetyMacCready) 
			return MACCREADY;
		else
			return GlidePolar::SafetyMacCready;
	}
	return MACCREADY;

}


void SetOverColorRef() {
  switch(OverColor) {
	case OcWhite:
		OverColorRef=RGB_WHITE;
		break;
	case OcBlack:
		OverColorRef=RGB_SBLACK;
		break;
	case OcBlue:
		OverColorRef=RGB_DARKBLUE;
		break;
	case OcGreen:
		OverColorRef=RGB_GREEN;
		break;
	case OcYellow:
		OverColorRef=RGB_YELLOW;
		break;
	case OcCyan:
		OverColorRef=RGB_CYAN;
		break;
	case OcOrange:
		OverColorRef=RGB_ORANGE;
		break;
	case OcGrey:
		OverColorRef=RGB_GREY;
		break;
	case OcDarkGrey:
		OverColorRef=RGB_DARKGREY;
		break;
	case OcDarkWhite:
		OverColorRef=RGB_DARKWHITE;
		break;
	case OcAmber:
		OverColorRef=RGB_AMBER;
		break;
	case OcLightGreen:
		OverColorRef=RGB_LIGHTGREEN;
		break;
	case OcPetrol:
		OverColorRef=RGB_PETROL;
		break;
	default:
		OverColorRef=RGB_MAGENTA;
		break;
  }
}


#ifdef PNA
void CreateRecursiveDirectory(TCHAR *fullpath)
{
  TCHAR tmpbuf[MAX_PATH];
  TCHAR *p;
  TCHAR *lastslash;
  bool found;
  
  if ( _tcslen(fullpath) <10 || _tcslen(fullpath)>=MAX_PATH) {
	StartupStore(_T("... FontPath too short or too long, cannot create folders%s"),NEWLINE);
	return;
  }

  if (*fullpath != '\\' ) {
	StartupStore(TEXT("... FontPath <%s> has no leading backslash, cannot create folders on a relative path.%s"),fullpath,NEWLINE);
	return;
  }

  lastslash=tmpbuf;

  do {
	// we copy the full path in tmpbuf as a working copy 
	_tcscpy(tmpbuf,fullpath);
	found=false;
	// we are looking after a slash. like in /Disk/
	// special case: a simple / remaining which we ignore, because either it is the first and only (like in \)
	// or it is a trailing slash with a null following
	if (*(lastslash+1)=='\0') {
		break;
	}
	
	// no eol, so lets look for another slash, starting from the char after last
	for (p=lastslash+1; *p != '\0'; p++) {
		if ( *p == '\\' ) {
			*p='\0';
			found=true;
			lastslash=p;
			break;
		}
	}
	if (_tcscmp(tmpbuf,_T("\\Windows"))==0) {
		continue;
	}
	CreateDirectory(tmpbuf, NULL);
  } while (found);
			
  return;
}
#endif


bool CheckClubVersion() {
  TCHAR srcfile[MAX_PATH];
  LocalPath(srcfile, _T("CLUB"));
  if (  GetFileAttributes(srcfile) == 0xffffffff ) return false;
  return true;
}

void ClubForbiddenMsg() {
  MessageBoxX(hWndMapWindow, 
	// LKTOKEN  _@M503_ = "Operation forbidden on CLUB devices" 
	gettext(TEXT("_@M503_")),
	_T("CLUB DEVICE"), 
	MB_OK|MB_ICONEXCLAMATION);
        return;
}


// Are we using lockmode? What is the current status?
bool LockMode(const short lmode) {

  switch(lmode) {
	case 0:		// query availability of LockMode
		return true;
		break;

	case 1:		// query lock/unlock status
		return LockModeStatus;
		break;

	case 2:		// invert lock status
		LockModeStatus = !LockModeStatus;
		return LockModeStatus;
		break;

	case 3:		// query button is usable or not
		if (ISPARAGLIDER)
			// Positive if not flying
			return CALCULATED_INFO.Flying==TRUE?false:true;
		else return true;
		break;

	case 9:		// Check if we can unlock the screen
		if (ISPARAGLIDER) {
			// Automatic unlock
			if (CALCULATED_INFO.Flying == TRUE) {
				if ( (GPS_INFO.Time - CALCULATED_INFO.TakeOffTime)>10) {
					LockModeStatus=false;
				}
			}
		}
		return LockModeStatus;
		break;

	default:
		return false;
		break;
  }

  return 0;

}



