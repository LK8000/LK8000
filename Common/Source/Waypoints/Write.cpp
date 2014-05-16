/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"

extern int globalFileNum;


// globalFileNum will tell us what is the file type we are writing to
// HEADERs for each format are written inside calling function, not here.
void WriteWayPointFileWayPoint(FILE *fp, WAYPOINT* wpt) {
  TCHAR flags[MAX_PATH];
  TCHAR latitude[MAX_PATH];
  TCHAR longitude[MAX_PATH];
  TCHAR comment[COMMENT_SIZE*2]; //@ 101112
  TCHAR rwdirection[30];
  TCHAR rwlen[30];
  TCHAR cupFreq[CUPSIZE_FREQ*2];
  TCHAR cupCode[CUPSIZE_CODE*2];
  // TCHAR cupCountry[CUPSIZE_COUNTRY*2];
  
  int filemode;
  
  flags[0]=0;
 
  if (globalFileNum<0 || globalFileNum>1) {
	StartupStore(_T("++++++ WriteWayPoint error: impossible file index!%s"),NEWLINE);
	return;
  }
  // WpFileType use position 1 and 2 for file1 and file2. Position 0 is reserved
  filemode=WpFileType[globalFileNum+1];

  if (filemode == LKW_DAT) {
 
	WaypointLatitudeToString(wpt->Latitude, latitude);
	WaypointLongitudeToString(wpt->Longitude, longitude);
	WaypointFlagsToString(wpt->Flags, flags);
 
	if (wpt->Comment!=NULL) {
		LK_tcsncpy(comment,wpt->Comment,COMMENT_SIZE);
	} else
		_tcscpy(comment,_T(""));

	fprintf(fp,"%d,%S,%S,%dM,%S,%S,%S\r\n",
		wpt->Number,
		latitude,
		longitude,
		iround(wpt->Altitude),
		flags,
		wpt->Name,
		comment);  

	return;
  } // DAT

  if (filemode == LKW_COMPE) {

	char NS[]= "SN";
	char EW[]= "WE";


	if (wpt->Comment!=NULL) {
		LK_tcsncpy(comment,wpt->Comment,COMMENT_SIZE);
	} else
		_tcscpy(comment,_T(""));

	fprintf(fp,"W  %S A %.10f%c%c %.10f%c%c 27-MAR-62 00:00:00 %.6f %S\r\n",
	wpt->Name,
	fabs(wpt->Latitude),
	0xba,
	NS[wpt->Latitude<0?0:1],
	fabs(wpt->Longitude),
	0xba,
	EW[wpt->Longitude<0?0:1],
	wpt->Altitude,
	comment);


	return;
  }

  if (filemode == LKW_CUP) {  

	LatitudeToCUPString(wpt->Latitude, latitude);
	LongitudeToCUPString(wpt->Longitude, longitude);

	if (wpt->RunwayDir >= 0)	
		_stprintf(rwdirection,_T("%d"),wpt->RunwayDir);
	else
		_tcscpy(rwdirection,_T(""));
	if (wpt->RunwayLen > 0)	
		_stprintf(rwlen,_T("%d.0m"),wpt->RunwayLen);
	else
		_tcscpy(rwlen,_T(""));

	if ( (_tcslen(wpt->Freq)>0) && (_tcslen(wpt->Freq)<=CUPSIZE_FREQ)) { // 100221 buffer overflow fix
		_stprintf(cupFreq,_T("\"%s\""),wpt->Freq);
	} else
		_tcscpy(cupFreq,_T(""));

	if ( (_tcslen(wpt->Code)>0)&&(_tcslen(wpt->Code)<=CUPSIZE_CODE)) {
		_stprintf(cupCode,_T("\"%s\""),wpt->Code);
	} else
		_tcscpy(cupCode,_T(""));

	if (wpt->Comment!=NULL) {
		LK_tcsncpy(comment,wpt->Comment,COMMENT_SIZE);
	} else
		_tcscpy(comment,_T(""));
	fprintf(fp,"\"%S\",%S,%S,%S,%S,%d.0m,%d,%S,%S,%S,%S\r\n",
		wpt->Name,
		wpt->Code,
		wpt->Country,
		latitude,
		longitude,
		iround(wpt->Altitude),
		wpt->Style,
		rwdirection,
		rwlen,
		cupFreq,
		comment);

	return;
  }

  if(filemode == LKW_OZI) {

	if (wpt->Comment!=NULL) {
		LK_tcsncpy(comment,wpt->Comment,COMMENT_SIZE);
	} else
		_tcscpy(comment,_T(""));

	if(_tcslen(comment) > 40){
		comment[40] = _T('\0');
	}

	// Calc Waypoint pos in file
	int nWaypointPos = 1;
	for(int i = NUMRESWP; i < (wpt-WayPointList); i++) {
		if(WayPointList[i].FileNum == wpt->FileNum)
			nWaypointPos++;
	}

	fprintf(fp, "%d,%S,%.6f,%.6f,,0,1,3,0,65635,%S,0,0,0,%d,6,0,17\r\n",
			nWaypointPos,// position in file
			wpt->Name,
			wpt->Latitude,
			wpt->Longitude,
			comment,
			iround(wpt->Altitude*TOFEET));

  }


  StartupStore(_T("...... Invalid filemode=%d file=%d wp=%d%s"), filemode,globalFileNum,wpt->Number ,NEWLINE);

}


// globalFileNum is 0 for file1 and 1 for file2
void WriteWayPointFile(FILE *fp) {
  int i;

  // remove previous home if it exists in this file
  for (i=NUMRESWP; i<(int)NumberOfWayPoints; i++) {  // BUGFIX 091206
    if (WayPointList[i].FileNum == globalFileNum) {
      if ((WayPointList[i].Flags & HOME) == HOME) {
        WayPointList[i].Flags -= HOME;
      }
    }
  }
  // Write specific format header 
  if (globalFileNum>=0 && globalFileNum<2) { // 100208
	if ( WpFileType[globalFileNum+1] == LKW_CUP ) {
	 fprintf(fp,"name,code,country,lat,lon,elev,style,rwdir,rwlen,freq,desc\r\n");
	}
	if ( WpFileType[globalFileNum+1] == LKW_COMPE ) { // 100212
	 fprintf(fp,"G  WGS 84\r\n");
	 fprintf(fp,"U  1\r\n");
	}
	if ( WpFileType[globalFileNum+1] == LKW_OZI ) {
		//Always Use V1.1 file format
	 	fprintf(fp,"OziExplorer Waypoint File Version 1.1\r\n");
	 	fprintf(fp,"WGS 84\r\n");
	 	fprintf(fp,"Reserved 2\r\n");
	 	fprintf(fp,"Reserved 3\r\n");
	}
  } else {
	StartupStore(_T("... WriteWayPointFile: invalid globalFileNum%s"),NEWLINE);
	return;
  }
  for (i=NUMRESWP; i<(int)NumberOfWayPoints; i++) {
    if (WayPointList[i].FileNum == globalFileNum) {

      // set home flag if it's the home
      if (i==HomeWaypoint) {
        if ((WayPointList[i].Flags & HOME) != HOME) {
          WayPointList[i].Flags += HOME;
        }
      }

      WriteWayPointFileWayPoint(fp, &WayPointList[i]);
    }
  }
}

// called from dlgConfiguration
// CAREFUL we must check for read only files here!!
void WaypointWriteFiles(void) {
  LockTaskData();

  TCHAR szFile1[MAX_PATH] = TEXT("\0");
  TCHAR szFile2[MAX_PATH] = TEXT("\0");
        
  FILE *fp=NULL;
  _tcscpy(szFile1,szWaypointFile);
  ExpandLocalPath(szFile1);

  if (_tcslen(szFile1)>0) {
    fp = _tfopen(szFile1, TEXT("wb"));
  } else {

	LocalPath(szFile1,_T(LKD_WAYPOINTS));
	_tcscat(szFile1,_T("\\")); _tcscat(szFile1,_T(LKF_WAYPOINTS1)); // 091206

	fp = _tfopen(szFile1, TEXT("wb"));
  }

  if(fp != NULL) {
    globalFileNum = 0;
    WriteWayPointFile(fp);
    fprintf(fp,"\r\n");
    fclose(fp);
    fp = NULL;
  } 
  _tcscpy(szFile2,szAdditionalWaypointFile);
  ExpandLocalPath(szFile2);

  if (_tcslen(szFile2)>0) {
    fp = _tfopen(szFile2, TEXT("wb"));
  } else {

/* 091206
	LocalPath(szFile2);
	_tcscat(szFile2,TEXT("\\waypoints2.dat"));
*/
	LocalPath(szFile2,_T(LKD_WAYPOINTS));
	_tcscat(szFile2,_T("\\")); _tcscat(szFile2,_T(LKF_WAYPOINTS2)); // 091206

	fp = _tfopen(szFile2, TEXT("wb"));
  }
  
  if(fp != NULL) {
    globalFileNum = 1;
    WriteWayPointFile(fp);
    fprintf(fp,"\r\n");
    fclose(fp);
    fp = NULL;
  }
  UnlockTaskData();
}

