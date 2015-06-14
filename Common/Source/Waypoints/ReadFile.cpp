/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "Dialogs.h"
#include "resource.h"


extern int globalFileNum;

static TCHAR nTemp2String[READLINE_LENGTH*2];



// returns -1 if error, or the WpFileType 
int ReadWayPointFile(ZZIP_FILE *fp, TCHAR *CurrentWpFileName)
{
  WAYPOINT new_waypoint {};
  DWORD fPos=0;
  int nLineNumber=0;
  short fileformat=LKW_DAT;

  CreateProgressDialog(gettext(TEXT("_@M903_"))); // Loading Waypoints File...

#ifdef WIN32
  // zzip_file_size only exist in private version of zziplib
  zzip_off_t fSize = zzip_file_size(fp);
  if (fSize <10) {
	StartupStore(_T("... ReadWayPointFile: waypoint file %s type=%d is empty%s"), CurrentWpFileName,fileformat,NEWLINE);
	return -1;
  }
#endif

  fileformat=GetWaypointFileFormatType(CurrentWpFileName);

  if (fileformat<0) {
	StartupStore(_T("... Unknown file format in waypoint file <%s\n"),CurrentWpFileName);
	// We do NOT return, because first we analyze the content.
  }


  memset(nTemp2String, 0, sizeof(nTemp2String)); // clear Temp Buffer

  // check file format
  bool fempty=true;
  int  slen=0; // 100204 WIP
  charset cs = charset::unknown;
  while ( ReadString(fp,READLINE_LENGTH,nTemp2String, cs) ) {
	slen=_tcslen(nTemp2String);
	if (slen<1) continue;
	if ( _tcsncmp(_T("G  WGS 84"),nTemp2String,9) == 0 ||
	   _tcsncmp(_T("G WGS 84"),nTemp2String,8) == 0 ||
	   // consider UCS header, 3 bytes in fact. This is a workaround.
	   _tcsncmp(_T("G  WGS 84"),&nTemp2String[3],9) == 0) {
		if ( !ReadString(fp,READLINE_LENGTH,nTemp2String,cs) ) {
			StartupStore(_T(". Waypoint file %d format: CompeGPS truncated, rejected%s"),globalFileNum+1,NEWLINE);
			return -1;
		}
		slen=_tcslen(nTemp2String);
		if (slen<1) {
			StartupStore(_T(". Waypoint file %d format: CompeGPS MISSING second U line, rejected%s"),globalFileNum+1,NEWLINE);
			return -1;
		}
		if ( (_tcsncmp(_T("U  0"),nTemp2String,4) == 0) ||
		     (_tcsncmp(_T("U 0"),nTemp2String,3) == 0)) {
			StartupStore(_T(". Waypoint file %d format: CompeGPS with UTM coordinates UNSUPPORTED%s"),globalFileNum+1,NEWLINE);
			return -1;
		}
		if ( _tcsncmp(_T("U  1"),nTemp2String,4) != 0 && 
		     _tcsncmp(_T("U 1"),nTemp2String,3) != 0 ) {
			StartupStore(_T(". Waypoint file %d format: CompeGPS unknown U field, rejected%s"),globalFileNum+1,NEWLINE);
			return -1;
		}
		
		StartupStore(_T(". Waypoint file %d format: CompeGPS, LatLon coordinates%s"),globalFileNum+1,NEWLINE);
		fempty=false;
		fileformat=LKW_COMPE;
		break;
	}
	if ( (_tcsncmp(_T("name,code,country"),nTemp2String,17) == 0) || 
		(_tcsncmp(_T("Title,Code,Country"),nTemp2String,18) == 0)  // 100314
	) {
		StartupStore(_T(". Waypoint file %d format: SeeYou%s"),globalFileNum+1,NEWLINE);
		fempty=false;
		fileformat=LKW_CUP;
		break;
	}

	if ( ( _tcsstr(nTemp2String, _T("OziExplorer Waypoint File")) == nTemp2String )||
			   // consider UCS header, 3 bytes in fact. This is a workaround.
			(_tcsstr(&nTemp2String[3], _T("OziExplorer Waypoint File")) == &nTemp2String[3]) ) {
		StartupStore(_T(". Waypoint file %d format: OziExplorer%s"),globalFileNum+1,NEWLINE);
		fempty=false;
		fileformat=LKW_OZI;
		break;
	}

	// consider also the case of empty file, when a waypoint if saved starting with numbering after
	// the virtual wps (including the 0);
	// Warning, using virtualdatheader 3 tcsncmp because virtuals are now more than 9.
	TCHAR virtualdatheader[5];
	_stprintf(virtualdatheader,_T("%d,"),RESWP_END+2);
	if ( _tcsncmp(_T("1,"),nTemp2String,2) == 0 ||
	  _tcsncmp(virtualdatheader,nTemp2String,3) == 0) {
		StartupStore(_T(". Waypoint file %d format: WinPilot%s"),globalFileNum+1,NEWLINE);
		fempty=false;
		fileformat=LKW_DAT;
		break;
	}
	// Otherwise we use the fileformat .xxx suffix. 
	// Why we did not do it since the beginning? Simply because we should not rely on .xxx suffix
	// because some formats like CompeGPS and OZI, for example, share the same .WPT suffix.
	// 
	if (fileformat<0) {
		StartupStore(_T(".. Unknown WP header, unknown format in <%s>%s"),nTemp2String,NEWLINE);
		// leaving fempty true, so no good file available
		break;
	} else {
		fempty=false;
		StartupStore(_T(".. Unknown WP header, using format %d.  Header: <%s>%s"),fileformat,nTemp2String,NEWLINE);
		break;
	}
  }
  if (fempty) {
	return -1;
  }

  // SetFilePointer(hFile,0,NULL,FILE_BEGIN);
  fPos = 0;

  // a real shame, too lazy to change into do while loop
  // Skip already read lines containing header, unless we are using DAT, which has no header
  if ( fileformat==LKW_DAT) goto goto_inloop; 

  memset(nTemp2String, 0, sizeof(nTemp2String)); // clear Temp Buffer

  while(ReadString(fp, READLINE_LENGTH, nTemp2String, cs)){
goto_inloop:    
	nLineNumber++;
	nTemp2String[READLINE_LENGTH]=_T('\0');
	nTemp2String[READLINE_LENGTH-1]=_T('\n');
	nTemp2String[READLINE_LENGTH-2]=_T('\r');
	fPos += _tcslen(nTemp2String);
   
	if (_tcsstr(nTemp2String, TEXT("**")) == nTemp2String) // Look For Comment
		continue;

	if (_tcsstr(nTemp2String, TEXT("*")) == nTemp2String)  // Look For SeeYou Comment
		continue;

	if (nTemp2String[0] == '\0')
		continue;

	new_waypoint.Details = NULL; 
	new_waypoint.Comment = NULL; 

	if ( fileformat == LKW_DAT || fileformat== LKW_XCW ) {
		if (ParseDAT(nTemp2String, &new_waypoint)) {

			if ( (_tcscmp(new_waypoint.Name, gettext(TEXT(RESWP_TAKEOFF_NAME)))==0) && (new_waypoint.Number==RESWP_ID)) {
				StartupStore(_T("... FOUND TAKEOFF (%s) INSIDE WAYPOINTS FILE%s"), gettext(TEXT(RESWP_TAKEOFF_NAME)), NEWLINE);
				memcpy(&WayPointList[RESWP_TAKEOFF],&new_waypoint,sizeof(WAYPOINT));
				continue;
			}

			if (WaypointInTerrainRange(&new_waypoint)) { 
                if(!AddWaypoint(std::move(new_waypoint))) {
					return -1; // failed to allocate
				}
			}	
	  	}
	}
	if ( fileformat == LKW_CUP ) {
		if ( _tcsncmp(_T("-----Related Tasks"),nTemp2String,18)==0) {
			break;
		}
		if (ParseCUPWayPointString(nTemp2String, &new_waypoint)) {
			if ( (_tcscmp(new_waypoint.Name, gettext(TEXT(RESWP_TAKEOFF_NAME)))==0) && (new_waypoint.Number==RESWP_ID)) {
				StartupStore(_T("... FOUND TAKEOFF (%s) INSIDE WAYPOINTS FILE%s"), gettext(TEXT(RESWP_TAKEOFF_NAME)), NEWLINE);
				memcpy(&WayPointList[RESWP_TAKEOFF],&new_waypoint,sizeof(WAYPOINT));
				continue;
			}

			if (WaypointInTerrainRange(&new_waypoint)) { 
				if(!AddWaypoint(std::move(new_waypoint))) {
					return -1; // failed to allocate
				}
			}
		}
	}
	if ( fileformat == LKW_COMPE ) {
		if (ParseCOMPEWayPointString(nTemp2String, &new_waypoint)) {
			if ( (_tcscmp(new_waypoint.Name, gettext(TEXT(RESWP_TAKEOFF_NAME)))==0) && (new_waypoint.Number==RESWP_ID)) {
				StartupStore(_T("... FOUND TAKEOFF (%s) INSIDE WAYPOINTS FILE%s"), gettext(TEXT(RESWP_TAKEOFF_NAME)), NEWLINE);
				memcpy(&WayPointList[RESWP_TAKEOFF],&new_waypoint,sizeof(WAYPOINT));
				continue;
			}

			if (WaypointInTerrainRange(&new_waypoint)) {
                if(!AddWaypoint(std::move(new_waypoint))) {
					return -1; // failed to allocate
				}
			}
		}
	}

	if(fileformat == LKW_OZI){
		// Ignore first four header lines
		if(nLineNumber <= 3)
			continue;

		if(ParseOZIWayPointString(nTemp2String, &new_waypoint)){
			if ( (_tcscmp(new_waypoint.Name, gettext(TEXT(RESWP_TAKEOFF_NAME)))==0) && (new_waypoint.Number==RESWP_ID)) {
				StartupStore(_T("... FOUND TAKEOFF (%s) INSIDE WAYPOINTS FILE%s"), gettext(TEXT(RESWP_TAKEOFF_NAME)), NEWLINE);
				memcpy(&WayPointList[RESWP_TAKEOFF],&new_waypoint,sizeof(WAYPOINT));
				continue;
			}

			if (WaypointInTerrainRange(&new_waypoint)) {
                if(!AddWaypoint(std::move(new_waypoint))) {
					return -1; // failed to allocate
				}
			}
		}
	}

	memset(nTemp2String, 0, sizeof(nTemp2String)); // clear Temp Buffer

	continue;

  }

  return fileformat;

}
