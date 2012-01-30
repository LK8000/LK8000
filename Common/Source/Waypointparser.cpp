/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id: Waypointparser.cpp,v 8.7 2010/12/13 01:13:54 root Exp root $
*/

#include "externs.h"
#include "Waypointparser.h"
#include "LKStyle.h"
#include "MapWindow.h"
#include "LKProfiles.h"
#include "resource.h"

#include "RasterTerrain.h"

#include "xmlParser.h"


static int globalFileNum = 0;

TCHAR *strtok_r(TCHAR *s, TCHAR *delim, TCHAR **lasts);

//static void ExtractParameter(TCHAR *Source, TCHAR *Destination, int DesiredFieldNumber);
static int ParseWayPointString(TCHAR *mTempString,WAYPOINT *Temp);
static bool ParseCUPWayPointString(TCHAR *mTempString,WAYPOINT *Temp);
static bool ParseCOMPEWayPointString(TCHAR *mTempString,WAYPOINT *Temp);
static bool ParseOZIWayPointString(TCHAR *mTempString,WAYPOINT *Temp);
static double CUPToLat(TCHAR *temp);
static double CUPToLon(TCHAR *temp);
static double CalculateAngle(TCHAR *temp);
static int CheckFlags(TCHAR *temp);
static double ReadAltitude(TCHAR *temp);
static double ReadLength(TCHAR *temp);

static TCHAR nTemp2String[READLINE_LENGTH*2]; // 100205 BUGFIX? NO

static int WaypointOutOfTerrainRangeDontAskAgain = -1;


void CloseWayPoints() {
  #if TESTBENCH
  StartupStore(TEXT(". CloseWayPoints%s"),NEWLINE);
  #endif
  unsigned int i;
  if (NumberOfWayPoints>0) { // we must free also RESWps comments!
	StartupStore(TEXT(". Waypoint list was not empty, closing.%s"),NEWLINE);
	for (i=0; i<NumberOfWayPoints; i++) {
		if (WayPointList[i].Details) {
			free(WayPointList[i].Details);
			WayPointList[i].Details = NULL;
		}
		if (WayPointList[i].Comment) {
			free(WayPointList[i].Comment);
			WayPointList[i].Comment = NULL;
		}
	}
  }
  NumberOfWayPoints = 0; // we must force realloc also for RESWPs
 
  // here we should not have any memory allocated for wps including Reswp
  if(WayPointList != NULL) {
	#if TESTBENCH
	StartupStore(TEXT(". WayPointList not null, LocalFree on.%s"),NEWLINE);
	#endif
	LocalFree((HLOCAL)WayPointList);
	WayPointList = NULL;
	LocalFree((HLOCAL)WayPointCalc); // VENTA3
	WayPointCalc = NULL;
  }
  WaypointOutOfTerrainRangeDontAskAgain = WaypointsOutOfRange;
}


int dlgWaypointOutOfTerrain(TCHAR *Message);


static bool WaypointInTerrainRange(WAYPOINT *List) {

  if (WaypointOutOfTerrainRangeDontAskAgain == 1){
    return(true);
  }

  if (!RasterTerrain::isTerrainLoaded()) {
    return(true);
  }

  if (RasterTerrain::WaypointIsInTerrainRange(List->Latitude,
                                              List->Longitude)) {
    return true;
  } else {
    if (WaypointOutOfTerrainRangeDontAskAgain == 0){
      
      TCHAR sTmp[250];
      int res;

      _stprintf(sTmp, _T("Waypoint #%d \"%s\" \r\n%s\r\n\r\n%s"), 
                List->Number, List->Name,
	// LKTOKEN  _@M837_ = "out of Terrain bound" 
		gettext(TEXT("_@M837_")),
	// LKTOKEN  _@M395_ = "Load anyway?" 
		gettext(TEXT("_@M395_")));
      
      res = dlgWaypointOutOfTerrain(sTmp);
      
      switch(res){
      case wpTerrainBoundsYes: 
        return true;
      case wpTerrainBoundsNo: 
        return false;
      case wpTerrainBoundsYesAll: 
        WaypointOutOfTerrainRangeDontAskAgain = 1;
        // WaypointsOutOfRange = 1; // this would override user choice in configuration!
	#if OLDPROFILES
        SetToRegistry(szRegistryWaypointsOutOfRange, WaypointsOutOfRange);
        StoreRegistry();
	#endif
        return true;
      case mrCancle: 
      case wpTerrainBoundsNoAll: 
        WaypointOutOfTerrainRangeDontAskAgain = 2;
        // WaypointsOutOfRange = 2; // this would override user choice in configuration!
	#if OLDPROFILES
        SetToRegistry(szRegistryWaypointsOutOfRange, WaypointsOutOfRange);
        StoreRegistry();
	#endif
        return false;
      }
      
    } else {
      if (WaypointOutOfTerrainRangeDontAskAgain == 2)
        return(false);
      if (WaypointOutOfTerrainRangeDontAskAgain == 1)
        return(true);
    }
    return false;
  }
}

// VENTA3 added additional WP calculated list
bool AllocateWaypointList(void) {
  if (!WayPointList) {
    NumberOfWayPoints = 0;
    #if TESTBENCH
    StartupStore(_T(". AllocateWaypointList: "));
    #endif
    WayPointList = (WAYPOINT *)LocalAlloc(LPTR, 50 * sizeof(WAYPOINT));
    if(WayPointList == NULL) 
      {
	StartupStore(_T("FAILED!%s"),NEWLINE);
        MessageBoxX(hWndMainWindow,
	// LKTOKEN  _@M486_ = "Not Enough Memory For Waypoints" 
                    gettext(TEXT("_@M486_")),
	// LKTOKEN  _@M266_ = "Error" 
                    gettext(TEXT("_@M266_")),MB_OK|MB_ICONSTOP);
        return 0;
      }
    #if TESTBENCH
    StartupStore(_T("OK%s"),NEWLINE);
    StartupStore(_T(". AllocateWayPointCalc..."));
    #endif

    WayPointCalc = (WPCALC *)LocalAlloc(LPTR, 50 * sizeof(WPCALC));
    if(WayPointCalc == NULL) 
      {
	StartupStore(_T("FAILED!%s"),NEWLINE);
        MessageBoxX(hWndMainWindow,
	// LKTOKEN  _@M486_ = "Not Enough Memory For Waypoints" 
                    gettext(TEXT("_@M486_")),
	// LKTOKEN  _@M266_ = "Error" 
                    gettext(TEXT("_@M266_")),MB_OK|MB_ICONSTOP);
        return 0;
      }
    #if TESTBENCH
    StartupStore(_T("OK%s"),NEWLINE);
    #endif
    return true;
  }
  return true;
}


WAYPOINT* GrowWaypointList() {
  // memory allocation
  if (!AllocateWaypointList()) {
    return 0;
  }

  if (((NumberOfWayPoints+1) % 50) == 0) {
    WAYPOINT *p;
    
    if ((p = 
         (WAYPOINT *)LocalReAlloc(WayPointList, 
                                  (((NumberOfWayPoints+1)/50)+1) 
                                  * 50 * sizeof(WAYPOINT), 
                                  LMEM_MOVEABLE | LMEM_ZEROINIT)) == NULL){
      
	StartupStore(_T("+++ GrowWaypointList FAILED!%s"),NEWLINE);
      MessageBoxX(hWndMainWindow,
	// LKTOKEN  _@M486_ = "Not Enough Memory For Waypoints" 
                  gettext(TEXT("_@M486_")),
	// LKTOKEN  _@M266_ = "Error" 
                  gettext(TEXT("_@M266_")),MB_OK|MB_ICONSTOP);
      
      return 0; // failed to allocate
    }
    
    if (p != WayPointList){
      WayPointList = p;      
    }

    WPCALC *q;
    
    if ((q = 
         (WPCALC *)LocalReAlloc(WayPointCalc, 
                                  (((NumberOfWayPoints+1)/50)+1) 
                                  * 50 * sizeof(WPCALC), 
                                  LMEM_MOVEABLE | LMEM_ZEROINIT)) == NULL){
      
	StartupStore(_T("+++ GrowWaypointCalc FAILED!%s"),NEWLINE);
      MessageBoxX(hWndMainWindow,
	// LKTOKEN  _@M486_ = "Not Enough Memory For Waypoints" 
                  gettext(TEXT("_@M486_")),
	// LKTOKEN  _@M266_ = "Error" 
                  gettext(TEXT("_@M266_")),MB_OK|MB_ICONSTOP);
      
      return 0; // failed to allocate
    }
    
    if (q != WayPointCalc){
      WayPointCalc = q;      
    }
  }

  NumberOfWayPoints++;
  return WayPointList + NumberOfWayPoints-1;
  // returns the newly created waypoint
}

// returns -1 if error, or the WpFileType 
int ReadWayPointFile(ZZIP_FILE *fp, TCHAR *CurrentWpFileName)
{
  WAYPOINT *new_waypoint;
  TCHAR szTemp[100];
  DWORD fSize, fPos=0;
  int nLineNumber=0;
  short fileformat=LKW_DAT;

  HWND hProgress;

  hProgress = CreateProgressDialog(gettext(TEXT("_@M903_"))); // Loading Waypoints File...

  fSize = zzip_file_size(fp);

  fileformat=GetWaypointFileFormatType(CurrentWpFileName);

  if (fileformat<0) {
	StartupStore(_T("... Unknown file format in waypoint file <%s\n"),CurrentWpFileName);
	// We do NOT return, because first we analyze the content.
  }

  if (fSize <10) {
	StartupStore(_T("... ReadWayPointFile: waypoint file %s type=%d is empty%s"), CurrentWpFileName,fileformat,NEWLINE);
	return -1;
  }

  if (!AllocateWaypointList()) {
	StartupStore(_T("!!!!!! ReadWayPointFile: AllocateWaypointList FAILED%s"),NEWLINE);
	return -1;
  }

  new_waypoint = WayPointList+NumberOfWayPoints;


  memset(nTemp2String, 0, sizeof(nTemp2String)); // clear Temp Buffer

  // check file format
  bool fempty=true;
  int  slen=0; // 100204 WIP
  while ( ReadString(fp,READLINE_LENGTH,nTemp2String) ) {
	slen=_tcslen(nTemp2String);
	if (slen<1) continue;
	if ( _tcsncmp(_T("G  WGS 84"),nTemp2String,9) == 0 ||
	   // consider UCS header, 3 bytes in fact. This is a workaround.
	   _tcsncmp(_T("G  WGS 84"),&nTemp2String[3],9) == 0) {
		if ( !ReadString(fp,READLINE_LENGTH,nTemp2String) ) {
			StartupStore(_T(". Waypoint file %d format: CompeGPS truncated, rejected%s"),globalFileNum+1,NEWLINE);
			return -1;
		}
		slen=_tcslen(nTemp2String);
		if (slen<1) {
			StartupStore(_T(". Waypoint file %d format: CompeGPS MISSING second U line, rejected%s"),globalFileNum+1,NEWLINE);
			return -1;
		}
		if ( _tcsncmp(_T("U  0"),nTemp2String,4) == 0) {
			StartupStore(_T(". Waypoint file %d format: CompeGPS with UTM coordinates UNSUPPORTED%s"),globalFileNum+1,NEWLINE);
			return -1;
		}
		if ( _tcsncmp(_T("U  1"),nTemp2String,4) != 0) {
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
	TCHAR virtualdatheader[3];
	wsprintf(virtualdatheader,_T("%d,"),RESWP_END+2);
	if ( _tcsncmp(_T("1,"),nTemp2String,2) == 0 ||
	  _tcsncmp(virtualdatheader,nTemp2String,2) == 0) {
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

  while(ReadString(fp, READLINE_LENGTH, nTemp2String)){
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

	new_waypoint->Details = NULL; 
	new_waypoint->Comment = NULL; 

	if ( fileformat == LKW_DAT || fileformat== LKW_XCW ) {
		if (ParseWayPointString(nTemp2String, new_waypoint)) {

			if ( (_tcscmp(new_waypoint->Name, gettext(TEXT(RESWP_TAKEOFF_NAME)))==0) && (new_waypoint->Number==RESWP_ID)) {
				StartupStore(_T("... FOUND TAKEOFF (%s) INSIDE WAYPOINTS FILE%s"), gettext(TEXT(RESWP_TAKEOFF_NAME)), NEWLINE);
				memcpy(WayPointList,new_waypoint,sizeof(WAYPOINT));
				continue;
			}

			if (WaypointInTerrainRange(new_waypoint)) { 
				new_waypoint = GrowWaypointList();
				if (!new_waypoint) {
					return -1; // failed to allocate
				}
				new_waypoint++; // we want the next blank one
			}	
	  	}
	}
	if ( fileformat == LKW_CUP ) {
		if ( _tcsncmp(_T("-----Related Tasks"),nTemp2String,18)==0) {
			break;
		}
		if (ParseCUPWayPointString(nTemp2String, new_waypoint)) {
			if ( (_tcscmp(new_waypoint->Name, gettext(TEXT(RESWP_TAKEOFF_NAME)))==0) && (new_waypoint->Number==RESWP_ID)) {
				StartupStore(_T("... FOUND TAKEOFF (%s) INSIDE WAYPOINTS FILE%s"), gettext(TEXT(RESWP_TAKEOFF_NAME)), NEWLINE);
				memcpy(WayPointList,new_waypoint,sizeof(WAYPOINT));
				continue;
			}

			if (WaypointInTerrainRange(new_waypoint)) { 
				new_waypoint = GrowWaypointList();
				if (!new_waypoint) {
					return -1; // failed to allocate
				}
				new_waypoint++; // we want the next blank one
			}
		}
	}
	if ( fileformat == LKW_COMPE ) {
		if (ParseCOMPEWayPointString(nTemp2String, new_waypoint)) {
			if ( (_tcscmp(new_waypoint->Name, gettext(TEXT(RESWP_TAKEOFF_NAME)))==0) && (new_waypoint->Number==RESWP_ID)) {
				StartupStore(_T("... FOUND TAKEOFF (%s) INSIDE WAYPOINTS FILE%s"), gettext(TEXT(RESWP_TAKEOFF_NAME)), NEWLINE);
				memcpy(WayPointList,new_waypoint,sizeof(WAYPOINT));
				continue;
			}

			if (WaypointInTerrainRange(new_waypoint)) { 
				new_waypoint = GrowWaypointList();
				if (!new_waypoint) {
					return -1; // failed to allocate
				}
				new_waypoint++; // we want the next blank one
			}
		}
	}

	if(fileformat == LKW_OZI){
		// Ignore first four header lines
		if(nLineNumber <= 3)
			continue;

		if(ParseOZIWayPointString(nTemp2String, new_waypoint)){
			if ( (_tcscmp(new_waypoint->Name, gettext(TEXT(RESWP_TAKEOFF_NAME)))==0) && (new_waypoint->Number==RESWP_ID)) {
				StartupStore(_T("... FOUND TAKEOFF (%s) INSIDE WAYPOINTS FILE%s"), gettext(TEXT(RESWP_TAKEOFF_NAME)), NEWLINE);
				memcpy(WayPointList,new_waypoint,sizeof(WAYPOINT));
				continue;
			}

			if (WaypointInTerrainRange(new_waypoint)) {
				new_waypoint = GrowWaypointList();
				if (!new_waypoint) {
					return -1; // failed to allocate
				}
				new_waypoint++; // we want the next blank one
			}
		}
	}

	memset(nTemp2String, 0, sizeof(nTemp2String)); // clear Temp Buffer

	continue;

  }

  if (hProgress) {
	_stprintf(szTemp,TEXT("100%%"));       
	SetDlgItemText(hProgress,IDC_PROGRESS,szTemp);
  }
  return fileformat;

}


void WaypointAltitudeFromTerrain(WAYPOINT* Temp) {
  double myalt;
  RasterTerrain::Lock();
  RasterTerrain::SetTerrainRounding(0.0,0.0);
  
  myalt = RasterTerrain::GetTerrainHeight(Temp->Latitude, Temp->Longitude);
  if (myalt==TERRAIN_INVALID) myalt=0; //@ 101027 FIX

  if (myalt>0) {
    Temp->Altitude = myalt;
  } else {
    // error, can't find altitude for waypoint!
  }
  RasterTerrain::Unlock();
  
}

// This is converting DAT Winpilot
int ParseWayPointString(TCHAR *String,WAYPOINT *Temp)
{
  TCHAR ctemp[(COMMENT_SIZE*2)+1]; // 101102 BUGFIX, we let +1 for safety
  TCHAR *Number;
  TCHAR *pWClast = NULL;
  TCHAR *pToken;
  TCHAR TempString[READLINE_LENGTH];

  _tcscpy(TempString, String);  
  // 20060513:sgi added wor on a copy of the string, do not modify the
  // source string, needed on error messages

  Temp->Visible = true; // default all waypoints visible at start
  Temp->FarVisible = true;
  Temp->Format = LKW_DAT;

  Temp->FileNum = globalFileNum;

  // ExtractParameter(TempString,ctemp,0);
  if ((pToken = strtok_r(TempString, TEXT(","), &pWClast)) == NULL)
    return FALSE;
  Temp->Number = _tcstol(pToken, &Number, 10);
        
  //ExtractParameter(TempString,ctemp,1); //Latitude
  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL)
    return FALSE;
  Temp->Latitude = CalculateAngle(pToken);

  if((Temp->Latitude > 90) || (Temp->Latitude < -90))
    {
      return FALSE;
    }

  //ExtractParameter(TempString,ctemp,2); //Longitude
  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL)
    return FALSE;
  Temp->Longitude  = CalculateAngle(pToken);
  if((Temp->Longitude  > 180) || (Temp->Longitude  < -180))
    {
      return FALSE;
    }

  //ExtractParameter(TempString,ctemp,3); //Altitude
  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL)
    return FALSE;
  Temp->Altitude = ReadAltitude(pToken);
  if (Temp->Altitude == -9999){
    return FALSE;
  }

  //ExtractParameter(TempString,ctemp,4); //Flags
  if ((pToken = strtok_r(NULL, TEXT(","), &pWClast)) == NULL)
    return FALSE;
  Temp->Flags = CheckFlags(pToken);

  //ExtractParameter(TempString,ctemp,5); // Name
  if ((pToken = strtok_r(NULL, TEXT(",\n\r"), &pWClast)) == NULL)
    return FALSE;

  // guard against overrun
  if (_tcslen(pToken)>NAME_SIZE) {
    pToken[NAME_SIZE-1]= _T('\0');
  }

  _tcscpy(Temp->Name, pToken);
  int i;
  for (i=_tcslen(Temp->Name)-1; i>1; i--) {
    if (Temp->Name[i]==' ') {
      Temp->Name[i]=0;
    } else {
      break;
    }
  }

  //ExtractParameter(TempString,ctemp,6); // Comment
  // DAT Comment
  if ((pToken = strtok_r(NULL, TEXT("\n\r"), &pWClast)) != NULL){
    _tcsncpy(ctemp, pToken, COMMENT_SIZE); //@ 101102 BUGFIX bad. ctemp was not sized correctly!
    ctemp[COMMENT_SIZE] = '\0';

    // sgi, move "panic-stripping" of the comment-field after we extract
    // the zoom factor
    ctemp[COMMENT_SIZE] = '\0';

    if (_tcslen(ctemp) >0 ) {
	if (Temp->Comment) {
		free(Temp->Comment);
	}
	Temp->Comment = (TCHAR*)malloc((_tcslen(ctemp)+1)*sizeof(TCHAR));
	_tcscpy(Temp->Comment, ctemp);
    }

  } else {
    Temp->Comment = NULL; // useless
  }

  if(Temp->Altitude <= 0) {
    WaypointAltitudeFromTerrain(Temp);
  } 

  if (Temp->Details) {
    free(Temp->Details);
  }

  return TRUE;
}

  /*
void ExtractParameter(TCHAR *Source, TCHAR *Destination, int DesiredFieldNumber)
{
  int index = 0;
  int dest_index = 0;
  int CurrentFieldNumber = 0;
  int StringLength        = 0;

  StringLength = _tcslen(Source);

  while( (CurrentFieldNumber < DesiredFieldNumber) && (index < StringLength) )
    {
      if ( Source[ index ] == ',' )
        {
          CurrentFieldNumber++;
        }
      index++;
    }

  if ( CurrentFieldNumber == DesiredFieldNumber )
    {
      while( (index < StringLength)    &&
             (Source[ index ] != ',') &&
             (Source[ index ] != 0x00) )
        {
          Destination[dest_index] = Source[ index ];
          index++; dest_index++;
        }
      Destination[dest_index] = '\0';
    }
  // strip trailing spaces
  for (int i=dest_index-1; i>0; i--) {
    if (Destination[i]==' ') {
      Destination[i]= '\0';
    } else return;
  }
}
*/

static double CalculateAngle(TCHAR *temp)
{
  TCHAR *Colon;
  TCHAR *Stop;
  double Degrees, Mins;

  Colon = _tcschr(temp,':');

  if(!Colon)
    {
      return -9999;
    }

  *Colon = _T('\0');
  Colon ++;

  Degrees = (double)_tcstol(temp, &Stop, 10);
  Mins = (double)StrToDouble(Colon, &Stop);
  if (*Stop == ':') {
    Stop++;
    Mins += ((double)_tcstol(Stop, &Stop, 10)/60.0);
  }

  Degrees += (Mins/60);
        
  if((*Stop == 'N') || (*Stop == 'E'))
    {
    }
  else if((*Stop == 'S') || (*Stop == 'W'))
    {
      Degrees *= -1;
    }
  else
    {
      return -9999;
    }
        
  return Degrees;
}

static double CUPToLat(TCHAR *temp)
{
  TCHAR *dot, td;
  TCHAR tdeg[4], tmin[4], tsec[5];
  double degrees, mins, secs;
  unsigned int slen;
  bool north=false;

  // lat is 4555.0X minimum
  if (_tcslen(temp)<7||_tcslen(temp)>9) return -9999;
  // check there is a dot, to be sure
  dot = _tcschr(temp,'.');
  if(!dot) return -9999;
  *dot = _T('\0');
  dot++;

  _tcscpy(tsec,dot);
  slen=_tcslen(tsec);
  
  // seconds are 0X minimum including the letter
  if (slen<2 || slen>4) return -9999;
  td= tsec[slen-1];

  if ( (td != _T('N')) && ( td != _T('S')) ) return -9999;
  if ( td == _T('N') ) north=true;

  td='\0';

  tdeg[0]=temp[0];
  tdeg[1]=temp[1];
  tdeg[2]=_T('\0');

  tmin[0]=temp[2];
  tmin[1]=temp[3];
  tmin[2]=_T('\0');

  degrees = (double)_tcstol(tdeg, NULL, 10);
  mins     = (double)_tcstol(tmin, NULL, 10);
  secs     = (double)_tcstol(tsec, NULL, 10);

  // if seconds are only a decimal, for example 3 , they really are 300
  switch (slen) {
	case 2:
		// 3X 
		secs*=100;
		break;
	case 3:
		// 33X
		secs*=10;
		break;
	default:
		break;
  }

  mins += secs / 1000.0;
  degrees += mins / 60.0;

  if (!north) degrees *= -1;
        
  return degrees;
}

static double CUPToLon(TCHAR *temp)
{
  TCHAR *dot, td;
  TCHAR tdeg[4], tmin[4], tsec[5];
  double degrees, mins, secs;
  unsigned int slen;
  bool east=false;

  // longit can be 01234.5X
  if (_tcslen(temp)<8 || _tcslen(temp)>10) return -9999;

  // check there is a dot, to be sure
  dot = _tcschr(temp,'.');
  if(!dot) return -9999;
  *dot = _T('\0');
  dot++;

  _tcscpy(tsec,dot);
  slen=_tcslen(tsec);
  // seconds are 0X minimum including the letter
  if (slen<2 || slen>4) return -9999;
  td= tsec[slen-1];

  if ( (td != _T('E')) && ( td != _T('W')) ) return -9999;
  if ( td == _T('E') ) east=true;

  td='\0';

  tdeg[0]=temp[0];
  tdeg[1]=temp[1];
  tdeg[2]=temp[2];
  tdeg[3]=_T('\0');

  tmin[0]=temp[3];
  tmin[1]=temp[4];
  tmin[2]=_T('\0');

  degrees = (double)_tcstol(tdeg, NULL, 10);
  mins     = (double)_tcstol(tmin, NULL, 10);
  secs     = (double)_tcstol(tsec, NULL, 10);

  // if seconds are only a decimal, for example 3 , they really are 300
  switch (slen) {
	case 2:
		// 3X 
		secs*=100;
		break;
	case 3:
		// 33X
		secs*=10;
		break;
	default:
		break;
  }

  mins += secs / 1000.0;
  degrees += mins / 60.0;

  if (!east) degrees *= -1;
        
  return degrees;
}


static int CheckFlags(TCHAR *temp)
{
  int Flags = 0;

  if(_tcschr(temp,'A')) Flags += AIRPORT;
  if(_tcschr(temp,'T')) Flags += TURNPOINT;
  if(_tcschr(temp,'L')) Flags += LANDPOINT;
  if(_tcschr(temp,'H')) Flags += HOME;
  if(_tcschr(temp,'S')) Flags += START;
  if(_tcschr(temp,'F')) Flags += FINISH;
  if(_tcschr(temp,'R')) Flags += RESTRICTED;
  if(_tcschr(temp,'W')) Flags += WAYPOINTFLAG;

  return Flags;
}

static double ReadAltitude(TCHAR *temp)
{
  TCHAR *Stop=temp;
  double Altitude=-9999;
  Altitude = StrToDouble(temp, &Stop);

  if (temp == Stop)		// error at begin
	Altitude=-9999;
  else {
	if (Stop != NULL){	// number converted endpointer is set
		switch(*Stop){
			case 'M':				// meter's nothing to do
			case 'm':
			case '\0':
				break;
			case 'F':				// feet, convert to meter
			case 'f':
				Altitude = Altitude / TOFEET;
				break;
			default:				// anything else is a syntax error   
				Altitude = -9999;
				break;
		}
	}
  }
  return Altitude;
}

static double ReadLength(TCHAR *temp)
{
  TCHAR *stop=temp;
  double len;
  len = StrToDouble(temp, &stop);
  if (temp == stop) {		// error at begin
	len=-9999;
	return len;
  }
  if (stop != NULL){	// number converted endpointer is set

	if ( *stop == 'n' ) {
		len = len / TONAUTICALMILES;
		return len;
	} 
	if ( (*stop == 'm') && (*(stop+1) == 'l') ) {
		len = len / TOMILES;
		return len;
	} 
	if ( (*stop == 'f') || (*stop == 'F') ) {
		len = len / TOFEET;
		return len;
	} 
	if ( (*stop == 'm') || (*stop == '\0') ) {
		return len;
	} 
  }
  len = -9999;
  return len;
}

void ReadWayPoints(void)
{
  StartupStore(TEXT(". ReadWayPoints%s"),NEWLINE);

  TCHAR szFile1[MAX_PATH] = TEXT("\0");
  TCHAR szFile2[MAX_PATH] = TEXT("\0");
        
  ZZIP_FILE *fp=NULL;
#ifdef HAVEEXCEPTIONS
  __try{
#endif

    LockTaskData();
    CloseWayPoints(); // BUGFIX 091104 duplicate waypoints entries
    InitVirtualWaypoints();	// 091103

    #if OLDPROFILES
    GetRegistryString(szRegistryWayPointFile, szFile1, MAX_PATH);
    #else
    _tcscpy(szFile1,szWaypointFile);
    #endif

    #ifndef HAVEEXCEPTIONS
    #if OLDPROFILES
    SetRegistryString(szRegistryWayPointFile, TEXT("\0"));  
    #else
    _tcscpy(szWaypointFile,_T(""));
    #endif
    #endif
      
    if (_tcslen(szFile1)>0) {
      ExpandLocalPath(szFile1);
      fp = zzip_fopen(szFile1, "rt");
    } else {
      #if OLDPROFILES
      static TCHAR  szMapFile[MAX_PATH] = TEXT("\0");
      GetRegistryString(szRegistryMapFile, szMapFile, MAX_PATH);
      ExpandLocalPath(szMapFile);
      _tcscat(szMapFile,TEXT("/"));
      _tcscat(szMapFile,TEXT("waypoints.xcw"));
      fp = zzip_fopen(szMapFile, "rt");
      #else
      // search for cup waypoints inside mapfile LKM
      TCHAR szFile[MAX_PATH]=_T("\0");
      _tcscpy(szFile,szMapFile);
      ExpandLocalPath(szFile);
      _tcscat(szFile,TEXT("/"));
      _tcscat(szFile,TEXT("waypoints.cup"));
      fp = zzip_fopen(szFile, "rt");
      #endif
      if (fp != NULL) {
      StartupStore(TEXT("... Waypoint file embedded inside lkm%s"), NEWLINE);
      StartupStore(TEXT("... lkm: <%s>%s"), szFile,NEWLINE);
      }
    }

    if(fp != NULL)
      {
        globalFileNum = 0;
        WpFileType[1]=ReadWayPointFile(fp, szFile1);
        zzip_fclose(fp);
        fp = 0;
        // read OK, so set the registry to the actual file name
        #ifndef HAVEEXCEPTIONS
        ContractLocalPath(szFile1);
        #if OLDPROFILES
        SetRegistryString(szRegistryWayPointFile, szFile1);  
	#else
	_tcscpy(szWaypointFile,szFile1);
	#endif
        #endif
      } else {
      StartupStore(TEXT("--- No waypoint file 1%s"),NEWLINE);
    }
#ifdef HAVEEXCEPTIONS
  }__except(EXCEPTION_EXECUTE_HANDLER){
    MessageBoxX(hWndMainWindow,
                TEXT("Unhandled Error in first Waypoint file\r\nNo Wp's loaded from that File!"),
	// LKTOKEN  _@M266_ = "Error" 
                gettext(TEXT("_@M266_")),
                MB_OK|MB_ICONSTOP);
    #if OLDPROFILES
    SetRegistryString(szRegistryWayPointFile, TEXT("\0"));  
    #else
    _tcscpy(szWaypointFile,_T(""));
    #endif
  }
#endif

  // read additional waypoint file
#ifdef HAVEEXCEPTIONS
  int NumberOfWayPointsAfterFirstFile = NumberOfWayPoints;
#endif

#ifdef HAVEEXCEPTIONS
  __try{
#endif

    #if OLDPROFILES
    GetRegistryString(szRegistryAdditionalWayPointFile, szFile2, MAX_PATH);
    SetRegistryString(szRegistryAdditionalWayPointFile, TEXT("\0"));  
    #else
    // reset to empty until we verified it is existing
    _tcscpy(szFile2,szAdditionalWaypointFile);
    _tcscpy(szAdditionalWaypointFile,_T(""));
    #endif

    if (_tcslen(szFile2)>0){
      ExpandLocalPath(szFile2);
      fp = zzip_fopen(szFile2, "rt");
      if(fp != NULL){
        globalFileNum = 1;
        WpFileType[2]=ReadWayPointFile(fp, szFile2);
        zzip_fclose(fp);
        fp = NULL;
        // read OK, so set the registry to the actual file name
        ContractLocalPath(szFile2);
	#if OLDPROFILES
        SetRegistryString(szRegistryAdditionalWayPointFile, szFile2);  
	#else
	_tcscpy(szAdditionalWaypointFile,szFile2);
	#endif
      } else {
        StartupStore(TEXT("--- No waypoint file 2%s"),NEWLINE);
      }
    }

#ifdef HAVEEXCEPTIONS
  }__except(EXCEPTION_EXECUTE_HANDLER){

    if (NumberOfWayPointsAfterFirstFile == 0){
    } else {
      unsigned int i;
      for (i=NumberOfWayPointsAfterFirstFile; i<NumberOfWayPoints; i++) {
        if (WayPointList[i].Details) {
          free(WayPointList[i].Details);
        }
        if (WayPointList[i].Comment) {
          free(WayPointList[i].Comment);
        }
      }
    }
    MessageBoxX(hWndMainWindow,
                TEXT("Unhandled Error in second Waypoint file\r\nNo Wp's loaded from that File!"),
	// LKTOKEN  _@M266_ = "Error" 
                gettext(TEXT("_@M266_")),
                MB_OK|MB_ICONSTOP);
    #if OLDPROFILES
    SetRegistryString(szRegistryAdditionalWayPointFile, TEXT("\0"));  
    #else
    _tcscpy(szAdditionalWaypointFile,_T(""));
    #endif
  }
#endif

  UnlockTaskData();

}


void SetHome(bool reset)
{

  #if TESTBENCH
  StartupStore(TEXT(". SetHome%s"),NEWLINE);
  #endif

  unsigned int i;
  bool resetalternates=false;

  if (reset || !ValidWayPoint(NUMRESWP) || !ValidNotResWayPoint(HomeWaypoint) ) { // BUGFIX 100213 see if really we have wps!
	    HomeWaypoint = -1;
  }

  // If one of the alternates is no longer valid, we reset both of them 
  if (Alternate1 !=-1 ) {
 	 if (!ValidNotResWayPoint(Alternate1) ) {
	     resetalternates=true;
 	 }
  }
  if (Alternate2 !=-1 ) {
 	 if (!ValidNotResWayPoint(Alternate2) ) {
	     resetalternates=true;
 	 }
  }
  if (reset || resetalternates) {
      Alternate1= -1; Alternate2= -1;
  }


  // check invalid task ref waypoint or forced reset due to file change
  if (reset || !ValidNotResWayPoint(TeamCodeRefWaypoint)) {
    TeamCodeRefWaypoint = -1;
  }

  if ( ValidNotResWayPoint(AirfieldsHomeWaypoint) ) {
	HomeWaypoint = AirfieldsHomeWaypoint;
  }
  if (!ValidNotResWayPoint(HomeWaypoint)) {
    // search for home in waypoint list, if we don't have a home
    HomeWaypoint = -1;
    for(i=NUMRESWP;i<NumberOfWayPoints;i++) {
	if( (WayPointList[i].Flags & HOME) == HOME) {
		if (HomeWaypoint < 0) {
			HomeWaypoint = i;
		}
	}
    }
  }

  // set team code reference waypoint if we don't have one
  if (TeamCodeRefWaypoint== -1) {
    TeamCodeRefWaypoint = HomeWaypoint;
  }

  // if we still don't have a valid home , search for match against memory home
  // This will fix a problem reloading waypoints after editing, or changing files with similars
  if ( (!ValidNotResWayPoint(HomeWaypoint)) && (WpHome_Lat!=0)) {
	for(i=NUMRESWP;i<NumberOfWayPoints;i++) {
		if( WayPointList[i].Latitude  ==  WpHome_Lat)
			if( WayPointList[i].Longitude  == WpHome_Lon)
				if ( _tcscmp(WayPointList[i].Name,WpHome_Name) == 0 ) {
					HomeWaypoint=i;
					break;
				}
	}
  }

  // set team code reference waypoint if we don't have one or set it -1
  if (TeamCodeRefWaypoint== -1) {
    TeamCodeRefWaypoint = HomeWaypoint;
  }

  if (ValidNotResWayPoint(HomeWaypoint)) { // 091213
	StartupStore(_T(". HomeWaypoint set to <%s> wpnum=%d%s"),WayPointList[HomeWaypoint].Name,HomeWaypoint,NEWLINE);
	GPS_INFO.Latitude = WayPointList[HomeWaypoint].Latitude;
	GPS_INFO.Longitude = WayPointList[HomeWaypoint].Longitude;
	GPS_INFO.Altitude = WayPointList[HomeWaypoint].Altitude;
	// Update memory HomeWaypoint
	WpHome_Lat=WayPointList[HomeWaypoint].Latitude; // 100213
	WpHome_Lon=WayPointList[HomeWaypoint].Longitude;
	_tcscpy(WpHome_Name,WayPointList[HomeWaypoint].Name);

  } else {

    // no home at all, so set it from center of terrain if available
    double lon, lat;
    if (RasterTerrain::GetTerrainCenter(&lat, &lon)) {
	GPS_INFO.Latitude = lat;
	GPS_INFO.Longitude = lon;
	GPS_INFO.Altitude = 0;
	StartupStore(_T("...... No HomeWaypoint, default position set to terrain center%s"),NEWLINE);
    } else
	StartupStore(_T("...... HomeWaypoint NOT SET%s"),NEWLINE);
  }

  #if OLDPROFILES
  // 
  // Save the home waypoint number in the resgistry
  //
  // VENTA3> this is probably useless, since HomeWayPoint &c were currently 
  //         just loaded from registry. 
  SetToRegistry(szRegistryHomeWaypoint,HomeWaypoint);
  SetToRegistry(szRegistryAlternate1,Alternate1);
  SetToRegistry(szRegistryAlternate2,Alternate2);
  SetToRegistry(szRegistryTeamcodeRefWaypoint,TeamCodeRefWaypoint);
  #endif
}

// This is slow, careful!
int FindNearestWayPoint(double X, double Y, double MaxRange,
                        bool exhaustive)
{
  unsigned int i;
  int NearestIndex = -1;
  double NearestDistance, Dist;

  if(NumberOfWayPoints <= NUMRESWP ) // BUGFIX 100227
    {
      return -1;
    }
  NearestDistance = MaxRange;

    for(i=NUMRESWP;i<NumberOfWayPoints;i++) {
      DistanceBearing(Y,X,
                      WayPointList[i].Latitude, 
                      WayPointList[i].Longitude, &Dist, NULL);
      if(Dist < NearestDistance) {
        NearestIndex = i;
        NearestDistance = Dist;
      }
    }
	// now look at TAKEOFF... TODO check all virtuals too
	// Takeoff can be normally very closed to actual airport, but not the same point!
	DistanceBearing(Y,X, WayPointList[RESWP_TAKEOFF].Latitude, WayPointList[RESWP_TAKEOFF].Longitude, &Dist, NULL);
	if ( Dist<=NearestDistance ) {
		// takeoff is closer, and next wp is not even visible...maybe because of zoom
		if  (NearestIndex >RESWP_TAKEOFF) { //  100227 BUGFIX
			if ( WayPointList[NearestIndex].Visible == FALSE ) {
				NearestIndex = RESWP_TAKEOFF;
				NearestDistance = Dist;
			}
		} else { // else ok 100227
			NearestIndex = RESWP_TAKEOFF;
			NearestDistance = Dist;
		}
	}

  if(NearestDistance < MaxRange) {
    return NearestIndex;
  } else {
    return -1;
  }
}


  // Number,Latitude,Longitude,Altitude,Flags,Name,Comment
  // Number starts at 1
  // Lat/long expressed as D:M:S[N/S/E/W]
  // Altitude as XXXM
  // Flags: T,H,A,L


void WaypointFlagsToString(int FlagsNum,
                           TCHAR *Flags) {

  if ((FlagsNum & AIRPORT) == AIRPORT) {
    wcscat(Flags,TEXT("A"));
  }
  if ((FlagsNum & TURNPOINT) == TURNPOINT) {
    wcscat(Flags,TEXT("T"));
  }
  if ((FlagsNum & LANDPOINT) == LANDPOINT) {
    wcscat(Flags,TEXT("L"));
  }
  if ((FlagsNum & HOME) == HOME) {
    wcscat(Flags,TEXT("H"));
  }
  if ((FlagsNum & START) == START) {
    wcscat(Flags,TEXT("S"));
  }
  if ((FlagsNum & FINISH) == FINISH) {
    wcscat(Flags,TEXT("F"));
  }
  if ((FlagsNum & RESTRICTED) == RESTRICTED) {
    wcscat(Flags,TEXT("R"));
  }
  if ((FlagsNum & WAYPOINTFLAG) == WAYPOINTFLAG) {
    wcscat(Flags,TEXT("W"));
  }
  if (_tcslen(Flags)==0) {
    wcscat(Flags,TEXT("T"));
  }
}

void LongitudeToCUPString(double Longitude, 
                               TCHAR *Buffer) {
  TCHAR EW[] = TEXT("WE");
  double dd, mm, ss;
  double lon;
  int idd,imm,iss;
  
  int sign = Longitude<0 ? 0 : 1;
  lon=fabs(Longitude);
 
  dd = (int)lon;
  mm = (lon-dd) * 60.0; 
  ss = (mm-(int)mm)*1000;

  idd = (int)dd;
  imm = (int)mm;
  iss = (int)(ss+0.5); // QUI
  if (iss>=1000) {
	imm++;
	iss-=1000;
  }

  _stprintf(Buffer, TEXT("%03d%02d.%03d%c"), idd,imm,iss, EW[sign]);
}

void LatitudeToCUPString(double Latitude,
                              TCHAR *Buffer) {
  TCHAR EW[] = TEXT("SN");
  double dd, mm, ss;
  double lat;

  int sign = Latitude<0 ? 0 : 1;
  lat = fabs(Latitude);

  dd = (int)lat;
  mm = (lat-dd) * 60.0;
  ss = (mm-(int)mm)*1000;

  mm = (int)mm;
  ss = (int)(ss+0.5);
  if (ss>=1000) {
	mm++;
	ss-=1000;
  }

  _stprintf(Buffer, TEXT("%02.0f%02.0f.%03.0f%c"), dd, mm, ss, EW[sign]);
}

void WaypointLongitudeToString(double Longitude,
                               TCHAR *Buffer) {
  TCHAR EW[] = TEXT("WE");
  int dd, mm, ss;
  
  int sign = Longitude<0 ? 0 : 1;
  Longitude = fabs(Longitude);
  
  dd = (int)Longitude;
  Longitude = (Longitude - dd) * 60.0;
  mm = (int)(Longitude);
  Longitude = (Longitude - mm) * 60.0;
  ss = (int)(Longitude + 0.5);
  if (ss >= 60)
    {
      mm++;
      ss -= 60;
    }
  if (mm >= 60)
    {
      dd++;
      mm -= 60;
    }
  _stprintf(Buffer, TEXT("%03d:%02d:%02d%c"), dd, mm, ss, EW[sign]);
}


void WaypointLatitudeToString(double Latitude,
                              TCHAR *Buffer) {
  TCHAR EW[] = TEXT("SN");
  int dd, mm, ss;

  int sign = Latitude<0 ? 0 : 1;
  Latitude = fabs(Latitude);

  dd = (int)Latitude;
  Latitude = (Latitude - dd) * 60.0;
  mm = (int)(Latitude);
  Latitude = (Latitude - mm) * 60.0;
  ss = (int)(Latitude + 0.5);
  if (ss >= 60) {
    mm++;
    ss -= 60;
  }
  if (mm >= 60) {
    dd++;
    mm -= 60;
  }
  _stprintf(Buffer, TEXT("%02d:%02d:%02d%c"), dd, mm, ss, EW[sign]);
}

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
 
	#if 0 
	_stprintf(comment, wpt->Comment);
	for (int j=0; j<(int)_tcslen(comment); j++) {
		if (comment[j]==_T('\r')) {
			comment[j] = 0;
		}
		if (comment[j]==_T('\n')) {
			comment[j] = 0;
		}
	}
	#endif

	if (wpt->Comment!=NULL)
		_tcscpy(comment,wpt->Comment);
	else
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


	if (wpt->Comment!=NULL)
		_tcscpy(comment,wpt->Comment);
	else
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

	if (wpt->Comment!=NULL)
		_tcscpy(comment,wpt->Comment);
	else
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

	if (wpt->Comment!=NULL)
		_tcscpy(comment,wpt->Comment);
	else
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
  #if OLDPROFILES
  GetRegistryString(szRegistryWayPointFile, szFile1, MAX_PATH);
  #else
  _tcscpy(szFile1,szWaypointFile);
  #endif
  ExpandLocalPath(szFile1);

  #if 0	// 101214 READ ONLY FILES
  if (WpFileType[1] == LKW_COMPE) goto goto_file2; // 100212
  #endif

  if (_tcslen(szFile1)>0) {
    fp = _tfopen(szFile1, TEXT("wb"));
  } else {

	/* 091206
	LocalPath(szFile1);
	_tcscat(szFile1,TEXT("\\waypoints1.dat"));
	*/

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
  #if 0 // 101214
  goto_file2:
  #endif
  #if OLDPROFILES
  GetRegistryString(szRegistryAdditionalWayPointFile, szFile2, MAX_PATH);
  #else
  _tcscpy(szFile2,szAdditionalWaypointFile);
  #endif
  ExpandLocalPath(szFile2);

  #if 0 // 101214 READ ONLY FILES
  if (WpFileType[2] == LKW_COMPE) goto goto_endwrite; // 100212
  #endif
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
  #if 0
  goto_endwrite:
  #endif
  UnlockTaskData();
}


int FindMatchingWaypoint(WAYPOINT *waypoint) {
  if (!WayPointList) {
    return -1;
  }
  unsigned int i;

  for (i=NUMRESWP; i<NumberOfWayPoints; i++) {

	// if different name, no match
	if (_tcscmp(waypoint->Name, WayPointList[i].Name)!=0) continue;

	// if same name, lat lon and flags must be the same in order to match
	// a previously existing waypoint
	if ((fabs(waypoint->Latitude-WayPointList[i].Latitude)<1.0e-6) 
	&& (fabs(waypoint->Longitude-WayPointList[i].Longitude)<1.0e-6) &&
	(waypoint->Flags == WayPointList[i].Flags) ) {
		// name, lat,lon, flags are the same: same wp!
		return i;
	}
  }
  
  return -1;
}

// LK8000: important!  ALL values in WPCALC must be initialized here!
// There is no other init of this structure elsewhere!
void InitWayPointCalc() {

  for (unsigned int i=0; i< NumberOfWayPoints; i++) {

	WayPointCalc[i].Preferred = false;
	WayPointCalc[i].Distance=-1;
	WayPointCalc[i].Bearing=-1;
	WayPointCalc[i].GR=-1;
	WayPointCalc[i].VGR=-1;
	WayPointCalc[i].NextETE=0;

	if ( (WayPointList[i].Flags & AIRPORT) == AIRPORT) {
		WayPointCalc[i].IsAirport=true;
		WayPointCalc[i].IsLandable=true;
		WayPointCalc[i].IsOutlanding=false;
		WayPointCalc[i].WpType=WPT_AIRPORT;
	} else {
		if ((WayPointList[i].Flags & LANDPOINT) == LANDPOINT) {
			WayPointCalc[i].IsAirport=false;
			WayPointCalc[i].IsLandable=true;
			WayPointCalc[i].IsOutlanding=true;
			WayPointCalc[i].WpType=WPT_OUTLANDING;
		} else {
			WayPointCalc[i].IsAirport=false;
			WayPointCalc[i].IsLandable=false;
			WayPointCalc[i].IsOutlanding=false;
			WayPointCalc[i].WpType=WPT_TURNPOINT;
		}
	}
	for (short j=0; j<ALTA_SIZE; j++) {
		WayPointCalc[i].AltArriv[j]=-1;
		WayPointCalc[i].AltReqd[j]=-1;
	}

  }
}

void AddReservedWaypoints()
{
	WayPointList[RESWP_TAKEOFF].Number=RESWP_TAKEOFF+1;
	WayPointList[RESWP_TAKEOFF].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_TAKEOFF].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_TAKEOFF].Altitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_TAKEOFF].Flags=TURNPOINT;
	_tcscpy(WayPointList[RESWP_TAKEOFF].Name, gettext(TEXT(RESWP_TAKEOFF_NAME)) ); // 100227
	if ( WayPointList[RESWP_TAKEOFF].Comment == NULL) {
		WayPointList[RESWP_TAKEOFF].Comment = (TCHAR*)malloc(100*sizeof(TCHAR));
	}
	_tcscpy(WayPointList[RESWP_TAKEOFF].Comment,_T("WAITING FOR GPS POSITION")); // 100227
	WayPointList[RESWP_TAKEOFF].Reachable=FALSE;
	WayPointList[RESWP_TAKEOFF].AltArivalAGL=0.0;
	WayPointList[RESWP_TAKEOFF].Visible=FALSE;
	WayPointList[RESWP_TAKEOFF].InTask=false;
	WayPointList[RESWP_TAKEOFF].Details=(TCHAR *)NULL;
	
	WayPointList[RESWP_TAKEOFF].FarVisible=false;
	WayPointList[RESWP_TAKEOFF].FileNum=-1;  // 100219  so it cannot be saved
	WayPointList[RESWP_TAKEOFF].Format= LKW_VIRTUAL;  //@ bugfix 101110
	
	WayPointList[RESWP_LASTTHERMAL].Number=RESWP_LASTTHERMAL+1;
	WayPointList[RESWP_LASTTHERMAL].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_LASTTHERMAL].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_LASTTHERMAL].Altitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_LASTTHERMAL].Flags=TURNPOINT;
	_tcscpy(WayPointList[RESWP_LASTTHERMAL].Name, gettext(TEXT(RESWP_LASTTHERMAL_NAME)) );
	WayPointList[RESWP_LASTTHERMAL].Comment = (TCHAR*)malloc(100*sizeof(TCHAR));
	// LKTOKEN _@M1320_ "LAST GOOD THERMAL"
	_tcscpy(WayPointList[RESWP_LASTTHERMAL].Comment, gettext(TEXT("_@M1320_")));		
	WayPointList[RESWP_LASTTHERMAL].Reachable=FALSE;
	WayPointList[RESWP_LASTTHERMAL].AltArivalAGL=0.0;
	WayPointList[RESWP_LASTTHERMAL].Visible=TRUE; // careful! 100929
	WayPointList[RESWP_LASTTHERMAL].InTask=false;
	WayPointList[RESWP_LASTTHERMAL].Details=(TCHAR *)NULL;
	WayPointList[RESWP_LASTTHERMAL].FarVisible=TRUE; // careful! 100929
	WayPointList[RESWP_LASTTHERMAL].FileNum=-1;

	WayPointCalc[RESWP_LASTTHERMAL].WpType = WPT_TURNPOINT;
	WayPointCalc[RESWP_LASTTHERMAL].IsLandable = false;
	WayPointCalc[RESWP_LASTTHERMAL].IsAirport = false;
	WayPointCalc[RESWP_LASTTHERMAL].IsOutlanding = false;
	WayPointList[RESWP_LASTTHERMAL].Format= LKW_VIRTUAL;  //@ bugfix 101110

	WayPointList[RESWP_TEAMMATE].Number=RESWP_TEAMMATE+1;
	WayPointList[RESWP_TEAMMATE].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_TEAMMATE].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_TEAMMATE].Altitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_TEAMMATE].Flags=TURNPOINT;
	_tcscpy(WayPointList[RESWP_TEAMMATE].Name, gettext(TEXT(RESWP_TEAMMATE_NAME)) );
	WayPointList[RESWP_TEAMMATE].Comment = (TCHAR*)malloc(100*sizeof(TCHAR));
	// LKTOKEN _@M1321_ "TEAM MATE"
	_tcscpy(WayPointList[RESWP_TEAMMATE].Comment, gettext(TEXT("_@M1321_")));
	WayPointList[RESWP_TEAMMATE].Reachable=FALSE;
	WayPointList[RESWP_TEAMMATE].AltArivalAGL=0.0;
	WayPointList[RESWP_TEAMMATE].Visible=FALSE;
	WayPointList[RESWP_TEAMMATE].InTask=false;
	WayPointList[RESWP_TEAMMATE].Details=(TCHAR *)NULL;
	WayPointList[RESWP_TEAMMATE].FarVisible=false;
	WayPointList[RESWP_TEAMMATE].FileNum=-1;

	WayPointCalc[RESWP_TEAMMATE].WpType = WPT_TURNPOINT;
	WayPointCalc[RESWP_TEAMMATE].IsLandable = false;
	WayPointCalc[RESWP_TEAMMATE].IsAirport = false;
	WayPointCalc[RESWP_TEAMMATE].IsOutlanding = false;
	WayPointList[RESWP_TEAMMATE].Format= LKW_VIRTUAL;  //@ bugfix 101110

	WayPointList[RESWP_FLARMTARGET].Number=RESWP_FLARMTARGET+1;
	WayPointList[RESWP_FLARMTARGET].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_FLARMTARGET].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_FLARMTARGET].Altitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_FLARMTARGET].Flags=TURNPOINT;
	_tcscpy(WayPointList[RESWP_FLARMTARGET].Name, gettext(TEXT(RESWP_FLARMTARGET_NAME)) );
	WayPointList[RESWP_FLARMTARGET].Comment = (TCHAR*)malloc(100*sizeof(TCHAR));
	// LKTOKEN _@M1322_ "FLARM TARGET"
	_tcscpy(WayPointList[RESWP_FLARMTARGET].Comment, gettext(TEXT("_@M1322_")));
	WayPointList[RESWP_FLARMTARGET].Reachable=FALSE;
	WayPointList[RESWP_FLARMTARGET].AltArivalAGL=0.0;
	WayPointList[RESWP_FLARMTARGET].Visible=FALSE;
	WayPointList[RESWP_FLARMTARGET].InTask=false;
	WayPointList[RESWP_FLARMTARGET].Details=(TCHAR *)NULL;
	WayPointList[RESWP_FLARMTARGET].FarVisible=false;
	WayPointList[RESWP_FLARMTARGET].FileNum=-1;

	WayPointCalc[RESWP_FLARMTARGET].WpType = WPT_TURNPOINT;
	WayPointCalc[RESWP_FLARMTARGET].IsLandable = false;
	WayPointCalc[RESWP_FLARMTARGET].IsAirport = false;
	WayPointCalc[RESWP_FLARMTARGET].IsOutlanding = false;
	WayPointList[RESWP_FLARMTARGET].Format= LKW_VIRTUAL;  //@ bugfix 101110

	WayPointList[RESWP_OPTIMIZED].Number=RESWP_OPTIMIZED+1;
	WayPointList[RESWP_OPTIMIZED].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_OPTIMIZED].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_OPTIMIZED].Altitude=RESWP_INVALIDNUMBER;
	WayPointList[RESWP_OPTIMIZED].Flags=TURNPOINT;
	// name will be assigned by function dynamically
	_tcscpy(WayPointList[RESWP_OPTIMIZED].Name, _T("OPTIMIZED") );
	WayPointList[RESWP_OPTIMIZED].Comment = (TCHAR*)NULL;
	WayPointList[RESWP_OPTIMIZED].Reachable=FALSE;
	WayPointList[RESWP_OPTIMIZED].AltArivalAGL=0.0;
	WayPointList[RESWP_OPTIMIZED].Visible=FALSE;
	WayPointList[RESWP_OPTIMIZED].InTask=false;
	WayPointList[RESWP_OPTIMIZED].Details=(TCHAR *)NULL;
	WayPointList[RESWP_OPTIMIZED].FarVisible=false;
	WayPointList[RESWP_OPTIMIZED].FileNum=-1;

	WayPointCalc[RESWP_OPTIMIZED].WpType = WPT_TURNPOINT;
	WayPointCalc[RESWP_OPTIMIZED].IsLandable = false;
	WayPointCalc[RESWP_OPTIMIZED].IsAirport = false;
	WayPointCalc[RESWP_OPTIMIZED].IsOutlanding = false;
	WayPointList[RESWP_OPTIMIZED].Format= LKW_VIRTUAL;

   for (short i=RESWP_FIRST_MARKER; i<=RESWP_LAST_MARKER; i++) {
	WayPointList[i].Number=i+1;
	WayPointList[i].Latitude=RESWP_INVALIDNUMBER;
	WayPointList[i].Longitude=RESWP_INVALIDNUMBER;
	WayPointList[i].Altitude=RESWP_INVALIDNUMBER;
	WayPointList[i].Flags=TURNPOINT;
	_tcscpy(WayPointList[i].Name, _T("LKMARKER"));
	WayPointList[i].Comment = (TCHAR*)malloc(100*sizeof(TCHAR));
	_tcscpy(WayPointList[i].Comment, _T(""));
	WayPointList[i].Reachable=FALSE;
	WayPointList[i].AltArivalAGL=0.0;
	WayPointList[i].Visible=FALSE;
	WayPointList[i].InTask=false;
	WayPointList[i].Details=(TCHAR *)NULL;
	WayPointList[i].FarVisible=FALSE;
	WayPointList[i].FileNum=-1;
	WayPointList[i].Style = STYLE_MARKER;

	WayPointCalc[i].WpType = WPT_UNKNOWN;
	WayPointCalc[i].IsLandable = false;
	WayPointCalc[i].IsAirport = false;
	WayPointCalc[i].IsOutlanding = false;
	WayPointList[i].Format= LKW_VIRTUAL;
   }

}


// Must be called BEFORE ReadWaypoints()!! 
void InitVirtualWaypoints()	// 091102
{

  #if TESTBENCH
  StartupStore(_T(". InitVirtualWaypoints: start%s"),NEWLINE);
  #endif
    LockTaskData();

  if (!AllocateWaypointList()) {
  	StartupStore(_T("!!!!!! InitVirtualWaypoints: AllocateWaypointList FAILED. CRITIC!%s"),NEWLINE);
    	UnlockTaskData(); // BUGFIX 091122
	return;
  }

  // if first load, reserve space
  if (NumberOfWayPoints<=NUMRESWP) {
	AddReservedWaypoints();
	NumberOfWayPoints=NUMRESWP;
	#if TESTBENCH
	StartupStore(_T(". InitVirtualWaypoints: done (%d vwp)%s"),NUMRESWP,NEWLINE);
	#endif
  } else {
	#if TESTBENCH
	StartupStore(_T(".. InitVirtualWaypoints: already done, skipping.%s"),NEWLINE);
        #endif
  }

    UnlockTaskData();

}

//#define CUPDEBUG
bool ParseCUPWayPointString(TCHAR *String,WAYPOINT *Temp)
{
  TCHAR ctemp[(COMMENT_SIZE*2)+1]; // must be bigger than COMMENT_SIZE!
  TCHAR *pToken;
  TCHAR TempString[READLINE_LENGTH+1];
  TCHAR OrigString[READLINE_LENGTH+1];
  TCHAR Tname[NAME_SIZE+1];
  int flags=0;

  unsigned int i, j;
  bool ishome=false; // 100310

  // strtok does not return empty fields. we create them here with special char
  #define DUMCHAR	'|'

  Temp->Visible = true; // default all waypoints visible at start
  Temp->FarVisible = true;
  Temp->Format = LKW_CUP;
  Temp->Number = NumberOfWayPoints;

  Temp->FileNum = globalFileNum;

  _tcscpy(OrigString, String);  
  // if string is too short do nothing
  if (_tcslen(OrigString)<11) return false;

  #ifdef CUPDEBUG
  StartupStore(_T("OLD:<%s>%s"),OrigString,NEWLINE);
  #endif

  for (i=0,j=0; i<_tcslen(OrigString); i++) {

	// skip last comma, and avoid overruning the end
	if (  (i+1)>= _tcslen(OrigString)) break;
	if ( (OrigString[i] == _T(',')) && (OrigString[i+1] == _T(',')) ) {
		TempString[j++] = _T(',');
		TempString[j++] = _T(DUMCHAR);
		continue;
	} 
	/* we need terminations for comments
	if ( OrigString[i] == _T('\r') ) continue;
	if ( OrigString[i] == _T('\n') ) continue; 
	*/

	TempString[j++] = OrigString[i];
  }
  TempString[j] = _T('\0');

  #ifdef CUPDEBUG
  StartupStore(_T("NEW:<%s>%s"),TempString,NEWLINE);
  #endif
  // ---------------- NAME ----------------
  pToken = _tcstok(TempString, TEXT(","));
  if (pToken == NULL) return false;

  if (_tcslen(pToken)>NAME_SIZE) {
	pToken[NAME_SIZE-1]= _T('\0');
  }

  // remove trailing spaces
  _tcscpy(Temp->Name, pToken); 
  for (i=_tcslen(Temp->Name)-1; i>1; i--) if (Temp->Name[i]==' ') Temp->Name[i]=0; else break;

  // now remove " " (if there)
  _tcscpy(Tname,Temp->Name);
  for (j=0, i=0; i<_tcslen(Tname)-1; i++) 
	if (Tname[i]!='\"') Temp->Name[j++]=Tname[i];
  Temp->Name[j]= _T('\0');

  #ifdef CUPDEBUG
  StartupStore(_T("   CUP NAME=<%s>%s"),Temp->Name,NEWLINE);
  #endif


  // ---------------- CODE ------------------
  pToken = _tcstok(NULL, TEXT(","));
  if (pToken == NULL) return false;

  if (_tcslen(pToken)>CUPSIZE_CODE) pToken[CUPSIZE_CODE-1]= _T('\0');
  _tcscpy(Temp->Code, pToken); 
  for (i=_tcslen(Temp->Code)-1; i>1; i--) if (Temp->Code[i]==' ') Temp->Code[i]=0; else break;
  _tcscpy(Tname,Temp->Code);
  for (j=0, i=0; i<_tcslen(Tname); i++) 
	//if (Tname[i]!='\"') Temp->Code[j++]=Tname[i];
	if ( (Tname[i]!='\"') && (Tname[i]!=DUMCHAR) ) Temp->Code[j++]=Tname[i]; Temp->Code[j]= _T('\0');

  if (_tcslen(Temp->Code)>5) { // 100310
	if (  _tcscmp(Temp->Code,_T("LKHOME")) == 0 ) {
		StartupStore(_T(". Found LKHOME inside CUP waypoint <%s>%s"),Temp->Name,NEWLINE);
		ishome=true;
	}
  }
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP CODE=<%s>%s"),Temp->Code,NEWLINE);
  #endif

        
  // ---------------- COUNTRY ------------------
  pToken = _tcstok(NULL, TEXT(","));
  if (pToken == NULL) return false;
  _tcscpy(Temp->Country,pToken);
  if (_tcslen(Temp->Country)>3) {
	Temp->Country[3]= _T('\0');
  }
  if ((_tcslen(Temp->Country) == 1) && Temp->Country[0]==DUMCHAR) Temp->Country[0]=_T('\0');

  #ifdef CUPDEBUG
  StartupStore(_T("   CUP COUNTRY=<%s>%s"),Temp->Country,NEWLINE);
  #endif


  // ---------------- LATITUDE  ------------------
  pToken = _tcstok(NULL, TEXT(","));
  if (pToken == NULL) return false;

  Temp->Latitude = CUPToLat(pToken);

  if((Temp->Latitude > 90) || (Temp->Latitude < -90)) {
	return false;
  }
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP LATITUDE=<%f>%s"),Temp->Latitude,NEWLINE);
  #endif


  // ---------------- LONGITUDE  ------------------
  pToken = _tcstok(NULL, TEXT(","));
  if (pToken == NULL) return false;
  Temp->Longitude  = CUPToLon(pToken);
  if((Temp->Longitude  > 180) || (Temp->Longitude  < -180)) {
	return false;
  }
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP LONGITUDE=<%f>%s"),Temp->Longitude,NEWLINE);
  #endif



  // ---------------- ELEVATION  ------------------
  pToken = _tcstok(NULL, TEXT(","));
  if (pToken == NULL) return false;
  Temp->Altitude = ReadAltitude(pToken);
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP ELEVATION=<%f>%s"),Temp->Altitude,NEWLINE);
  #endif
  if (Temp->Altitude == -9999){
	return false;
  }


  // ---------------- STYLE  ------------------
  pToken = _tcstok(NULL, TEXT(","));
  if (pToken == NULL) return false;
  
  Temp->Style = (int)_tcstol(pToken,NULL,10);
  switch(Temp->Style) {
	case 2:				// airfield grass
	case 4:				// glider site
	case 5:				// airfield solid
		flags = AIRPORT;
		flags += LANDPOINT;
		break;
	case 3:				// outlanding
		flags = LANDPOINT;
		break;
	default:
		flags = TURNPOINT;
		break;
  }
  if (ishome) flags += HOME;
  Temp->Flags = flags;

  #ifdef CUPDEBUG
  StartupStore(_T("   CUP STYLE=<%d> flags=%d %s"),Temp->Style,Temp->Flags,NEWLINE);
  #endif

  // ---------------- RWY DIRECTION   ------------------
  pToken = _tcstok(NULL, TEXT(","));
  if (pToken == NULL) return false;
  if ((_tcslen(pToken) == 1) && (pToken[0]==DUMCHAR))
	Temp->RunwayDir=-1;
  else
	Temp->RunwayDir = (int)_tcstol(pToken, NULL, 10);
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP RUNWAY DIRECTION=<%d>%s"),Temp->RunwayDir,NEWLINE);
  #endif


  // ---------------- RWY LENGTH   ------------------
  pToken = _tcstok(NULL, TEXT(","));
  if (pToken == NULL) return false;
  if ((_tcslen(pToken) == 1) && (pToken[0]==DUMCHAR))
	Temp->RunwayLen = -1;
  else
	Temp->RunwayLen = (int)ReadLength(pToken);
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP RUNWAY LEN=<%d>%s"),Temp->RunwayLen,NEWLINE);
  #endif



  // ---------------- AIRPORT FREQ   ------------------
  pToken = _tcstok(NULL, TEXT(","));
  if (pToken == NULL) return false;
  if (_tcslen(pToken)>CUPSIZE_FREQ) pToken[CUPSIZE_FREQ-1]= _T('\0');
  _tcscpy(Temp->Freq, pToken); 
  for (i=_tcslen(Temp->Freq)-1; i>1; i--) if (Temp->Freq[i]==' ') Temp->Freq[i]=0; else break;
  _tcscpy(Tname,Temp->Freq);
  for (j=0, i=0; i<_tcslen(Tname); i++) 
	if ( (Tname[i]!='\"') && (Tname[i]!=DUMCHAR) ) Temp->Freq[j++]=Tname[i];
  Temp->Freq[j]= _T('\0');

  #ifdef CUPDEBUG
  StartupStore(_T("   CUP FREQ=<%s>%s"),Temp->Freq,NEWLINE);
  #endif


  // ---------------- COMMENT   ------------------
  pToken = _tcstok(NULL, TEXT("\n\r"));
  if (pToken != NULL) {

	if (_tcslen(pToken)>=COMMENT_SIZE) pToken[COMMENT_SIZE-1]= _T('\0');

	// remove trailing spaces and CR LF
	_tcscpy(ctemp, pToken); 
	for (i=_tcslen(ctemp)-1; i>1; i--) {
		if ( (ctemp[i]==' ') || (ctemp[i]=='\r') || (ctemp[i]=='\n') ) ctemp[i]=0;
		else
			break;
	}

	// now remove " " (if there)
	for (j=0, i=0; i<_tcslen(ctemp); i++) 
		if (ctemp[i]!='\"') ctemp[j++]=ctemp[i];
	ctemp[j]= _T('\0');
	if (_tcslen(ctemp) >0 ) {
		if (Temp->Comment) {
			free(Temp->Comment);
		}
		Temp->Comment = (TCHAR*)malloc((_tcslen(ctemp)+1)*sizeof(TCHAR));
		_tcscpy(Temp->Comment, ctemp);
	}

	#ifdef CUPDEBUG
	StartupStore(_T("   CUP COMMENT=<%s>%s"),Temp->Comment,NEWLINE);
	#endif
  } else {
	Temp->Comment=NULL; // useless
  }

  if(Temp->Altitude <= 0) {
	WaypointAltitudeFromTerrain(Temp);
  } 

  if (Temp->Details) {
	free(Temp->Details);
  }

  return true;
}

bool ParseCOMPEWayPointString(TCHAR *String,WAYPOINT *Temp)
{
  TCHAR tComment[(COMMENT_SIZE*2)+1]; // must be bigger than COMMENT_SIZE!
  TCHAR tString[READLINE_LENGTH+1];
  TCHAR tName[NAME_SIZE+1];
  unsigned int slen;
  //int flags=0;
  bool ok;

  unsigned int i, j;

  #define MAXCOMPENAME	16

  slen=_tcslen(String);
  if (slen<65) return false;
  _tcscpy(tString, String);  


  // only handle W field, format:  W__NAME
  if (tString[0] != 'W') return false;
  if ( (tString[1] != ' ') || (tString[2]!= ' ') ) return false;
  if ( tString[3] == ' ' ) return false;

  Temp->Visible = true; // default all waypoints visible at start
  Temp->FarVisible = true;
  Temp->Format = LKW_COMPE;
  Temp->Number = NumberOfWayPoints;
  Temp->FileNum = globalFileNum;


  #ifdef COMPEDEBUG
  StartupStore(_T("COMPE IN:<%s>%s"),tString,NEWLINE);
  #endif

  // Name starts at position 4, index 3 . Search for space at the end of name (<=)
  for (i=3, j=0, ok=false; i<= 3+MAXCOMPENAME; i++) {
	if (tString[i] != _T(' ')) {; j++; continue; }
	ok=true; break;
  }
  if (j<1) {
	#ifdef COMPEDEBUG
	StartupStore(_T("Name is empty ! %s"),NEWLINE);
	#endif
	return false;
  }
  if (!ok) {
	#ifdef COMPEDEBUG
	StartupStore(_T("Name too long! %s"),NEWLINE);
	#endif
	return false;
  }
  // i now point to first space after name
  _tcsncpy(tName,&tString[3],j);
  tName[j]=_T('\0');
  #ifdef COMPEDEBUG
  StartupStore(_T("WP NAME size=%d: <%s>%s"),j,tName,NEWLINE);
  #endif
 
  if (tString[++i] != _T('A')) { 
	#ifdef COMPEDEBUG
	StartupStore(_T("Missing A field! %s"),NEWLINE);
	#endif
	return false;
  }
  if (tString[++i] != _T(' ')) { 
	#ifdef COMPEDEBUG
	StartupStore(_T("Missing space after A field! %s"),NEWLINE);
	#endif
	return false;
  }
  i++; 

  // we are now on the first digit of latitude

  // aaaaaaaahhhhh f**k unicode
  TCHAR tdeg[5];
  char  sdeg[5];
  sprintf(sdeg,"%c",0xBA);
  _stprintf(tdeg,_T("%S"),sdeg);
  TCHAR cDeg = tdeg[0];
  unsigned int p;

  // search for cDeg delimiter
  for (p=i, ok=false; p<(i+20);p++) {
	if (tString[p] == cDeg) {
		ok=true;
		break;
	}
  }
  if (!ok) {
	#ifdef COMPEDEBUG
	StartupStore(_T("Missing delimiter in latitude %s"),NEWLINE);
	#endif
	return false;
  }
  // p points to delimiter

  // latitude from i to i+12, starting from i counts 13
  TCHAR tLatitude[16];
  if ( (p-i)>15 ) {
	#ifdef COMPEDEBUG
	StartupStore(_T("latitude p-i exceed 15%s"),NEWLINE);
	#endif
	return false;
  }
  _tcsncpy(tLatitude,&tString[i],p-i);
  tLatitude[(p-i)+1]=_T('\0');

  i=p+1;
  // i points to NS

  bool north=false;
  TCHAR tNS = tString[i];
  TCHAR NS[]=_T("NS");
  if ( (tNS != NS[0]) && (tNS != NS[1])) { 
	#ifdef COMPEDEBUG
	StartupStore(_T("Wrong NS latitude! %s"),NEWLINE);
	#endif
	return false;
  }
  if ( tNS == NS[0] ) north=true;
  #ifdef COMPEDEBUG
  StartupStore(_T("WP LATITUDE : <%s> N1S0=%d%s"),tLatitude,north,NEWLINE);
  #endif

  // We are now on the space after latitude
  if (tString[++i] != _T(' ')) { 
	#ifdef COMPEDEBUG
	StartupStore(_T("Missing space after latitude %s"),NEWLINE);
	#endif
	return false;
  }
  i++; 
  // we are now on the first digit of longitude
  // search for cDeg delimiter
  for (p=i, ok=false; p<(i+20);p++) {
	if (tString[p] == cDeg) {
		ok=true;
		break;
	}
  }
  if (!ok) {
	#ifdef COMPEDEBUG
	StartupStore(_T("Missing delimiter in longitude %s"),NEWLINE);
	#endif
	return false;
  }
  // p points to delimiter

  TCHAR tLongitude[16];
  if ( (p-i)>15 ) {
	#ifdef COMPEDEBUG
	StartupStore(_T("longitude p-i exceed 15%s"),NEWLINE);
	#endif
	return false;
  }
  _tcsncpy(tLongitude,&tString[i],p-i);
  tLongitude[(p-i)]=_T('\0');

  i=p+1;
  // i points to EW
  bool east=false;
  TCHAR tEW = tString[i];
  TCHAR EW[]=_T("EW");
  if ( (tEW != EW[0]) && (tEW != EW[1])) { 
	#ifdef COMPEDEBUG
	StartupStore(_T("Wrong EW longitude! %s"),NEWLINE);
	#endif
	return false;
  }
  if ( tEW == EW[0] ) east=true;
  #ifdef COMPEDEBUG
  StartupStore(_T("WP LONGITUDE : <%s> E1W0=%d%s"),tLongitude,east,NEWLINE);
  #endif

  // We are now on the space after latitude
  if (tString[++i] != _T(' ')) { 
	#ifdef COMPEDEBUG
	StartupStore(_T("Missing space after longitude %s"),NEWLINE);
	#endif
	return false;
  }
  i++;  // point to beginning of tDummy..
  if ( (i+19)>slen ) {
	#ifdef COMPEDEBUG
	StartupStore(_T("Line overflow before dummy%s"),NEWLINE);
	#endif
	return false;
  }
  TCHAR tDummy[]=_T("27-MAR-62 00:00:00 "); // 19 tchars
  if ( _tcsncmp(&tString[i],tDummy,19 ) != 0 ) {
	#ifdef COMPEDEBUG
	StartupStore(_T("Missing dummy string and space after longitude %s"),NEWLINE);
	#endif
  }
  i+=20;
  // i now points to first digit of altitude, minim 8 chars
  // this check can be avoided
  if ( slen < (i+8) ) {
	#ifdef COMPEDEBUG
	StartupStore(_T("Line overflow before altitude%s"),NEWLINE);
	#endif
	return false;
  }

  // we are now on the first digit of altitude
  // search for space delimiter
  for (p=i, ok=false; p<slen;p++) {
	if (tString[p] == _T(' ')) {
		ok=true;
		break;
	}
  }
  if (!ok) {
	#ifdef COMPEDEBUG
	StartupStore(_T("Missing space after altitude%s"),NEWLINE);
	#endif
	return false;
  }
  // p points to space after altitude

  TCHAR tAltitude[16];
  if ( (p-i)>15 ) {
	#ifdef COMPEDEBUG
	StartupStore(_T("altitude p-i exceed 15%s"),NEWLINE);
	#endif
	return false;
  }
  _tcsncpy(tAltitude,&tString[i-1],p-i);
  tAltitude[(p-i)]=_T('\0');
  
  #ifdef COMPEDEBUG
  StartupStore(_T("WP ALTITUDE : <%s>%s"),tAltitude,NEWLINE);
  #endif

  i=p+1;
  // we are now on first char of comment
  // search for line termination
  for (p=i, ok=false; p<slen;p++) {
	if ( (tString[p] == _T('\n')) ||
	     (tString[p] == _T('\r')) 
        ) {
		ok=true;
		break;
	}
  }
  if (!ok) {
	#ifdef COMPEDEBUG
	StartupStore(_T("Missing CRLF after comment%s"),NEWLINE);
	#endif
	return false;
  }
  // p points to CR or LF after comment
  if ( (p-i)>((COMMENT_SIZE*2)-1) ) { 
	#ifdef COMPEDEBUG
	StartupStore(_T("Comment too long%s"),NEWLINE);
	#endif
	return false;
  } 
  _tcsncpy(tComment,&tString[i],p-i);
  tComment[(p-i)]=_T('\0');

  #ifdef COMPEDEBUG
  StartupStore(_T("WP COMMENT : <%s>%s"),tComment,NEWLINE);
  #endif

  if (_tcslen(tName) > NAME_SIZE ) tName[NAME_SIZE-1]=_T('\0');
  _tcscpy(Temp->Name,tName);


  Temp->Latitude = _tcstod(tLatitude,NULL);
  if (!north) Temp->Latitude *= -1; // 100218
  if((Temp->Latitude > 90) || (Temp->Latitude < -90)) {
	return false;
  }
  Temp->Longitude = _tcstod(tLongitude,NULL);
  if (!east) Temp->Longitude *= -1; 
  if((Temp->Longitude > 180) || (Temp->Longitude < -180)) {
	return false;
  }
  Temp->Altitude = ReadAltitude(tAltitude);
  if (Temp->Altitude == -9999) return false;

  Temp->Flags = TURNPOINT;

  if (_tcslen(tComment) >COMMENT_SIZE) {
	tComment[COMMENT_SIZE-1]=_T('\0');
  }
  if (_tcslen(tComment) >0 ) {
	if (Temp->Comment) {
		free(Temp->Comment);
	}
	Temp->Comment = (TCHAR*)malloc((_tcslen(tComment)+1)*sizeof(TCHAR));
	_tcscpy(Temp->Comment,tComment);
  } else
	Temp->Comment=NULL; //@ 101104



 return true;

 }


// Returns -1 if no result
int FindNearestFarVisibleWayPoint(double X, double Y, double maxRange, short wpType)
{
  unsigned int i;
  int nearestIndex = -1;
  double nearestDistance, dist;

  #if TESTBENCH
  int farvisibles=0;
  #endif

  if(NumberOfWayPoints <= NUMRESWP ) return -1;
  nearestDistance = maxRange;

  for(i=NUMRESWP;i<NumberOfWayPoints;i++) {

	if (!WayPointList[i].FarVisible) continue;
	if (wpType && (WayPointCalc[i].WpType != wpType)) continue;

	#if TESTBENCH
	farvisibles++;
	#endif

	DistanceBearing(Y,X, WayPointList[i].Latitude, WayPointList[i].Longitude, &dist, NULL);

	if(dist < nearestDistance) {
		nearestIndex = i;
		nearestDistance = dist;
	}
  }

  #if TESTBENCH
  StartupStore(_T("...... Checked %d farvisibles waypoints for maxRange=%f, type=%d\n"),farvisibles,maxRange,wpType);
  #endif

  if(nearestDistance < maxRange) {
	return nearestIndex;
  } else {
	return -1;
  }
}


bool ParseOZIWayPointString(TCHAR *String,WAYPOINT *Temp){

	Temp->Visible = true; // default all waypoints visible at start
	Temp->FarVisible = true;
	Temp->Format = LKW_OZI;
	Temp->Number = NumberOfWayPoints;
	Temp->FileNum = globalFileNum;
	Temp->Flags = TURNPOINT;

	memset(Temp->Name, 0, sizeof(Temp->Name)); // clear Name

	TCHAR TempString[READLINE_LENGTH];
	memset(TempString, 0, sizeof(TempString)); // clear TempString

	_tcscpy(TempString, String);

	// strtok_r skip empty field, It's not compatible with OziExplorer Waypoint File Version 1.1
	// use strsep_r instead of ( cf. Utils.h )

	TCHAR *pToken = NULL;
	TCHAR *Stop= NULL;

	TCHAR *pWClast = TempString;


	//	Field 1 : Number - this is the location in the array (max 1000), must be unique, usually start at 1 and increment. Can be set to -1 (minus 1) and the number will be auto generated.
	if ((pToken = strsep_r(TempString, TEXT(","), &pWClast)) == NULL)
		return false;

	//	Field 2 : Name - the waypoint name, use the correct length name to suit the GPS type.
	if ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) == NULL)
		return false;

	// guard against overrun
	if (_tcslen(pToken)>NAME_SIZE) {
		pToken[NAME_SIZE-1]= _T('\0');
	}

	// remove trailing spaces
	for (int i=_tcslen(pToken)-1; i>1; i--) if (pToken[i]==' ') pToken[i]=0; else break;

	_tcscpy(Temp->Name, pToken);

	//	Field 3 : Latitude - decimal degrees.
	if ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) == NULL)
		return false;

	Temp->Latitude = (double)StrToDouble(pToken, &Stop);

	if((Temp->Latitude > 90) || (Temp->Latitude < -90)) {
		return false;
	}

	//	Field 4 : Longitude - decimal degrees.
	if ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) == NULL)
		return false;

	Temp->Longitude  = (double)StrToDouble(pToken, &Stop);
	if((Temp->Longitude  > 180) || (Temp->Longitude  < -180)) {
		return false;
	}
	//	Field 5 : Date - see Date Format below, if blank a preset date will be used
	if ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) == NULL)
		return false;

	//	Field 6 : Symbol - 0 to number of symbols in GPS
	if ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) == NULL)
		return false;

	//	Field 7 : Status - always set to 1
	if ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) == NULL)
		return false;

	//	Field 8 : Map Display Format
	if ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) == NULL)
		return false;

	//	Field 9 : Foreground Color (RGB value)
	if ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) == NULL)
		return false;

	//	Field 10 : Background Color (RGB value)
	if ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) == NULL)
		return false;

	//	Field 11 : Description (max 40), no commas
	if ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) == NULL)
		return false;

    if (_tcslen(pToken) >0 ) {
    	// remove trailing spaces
    	for (int i=_tcslen(pToken)-1; i>1; i--) if (pToken[i]==' ') pToken[i]=0; else break;

    	if (Temp->Comment) {
    		free(Temp->Comment);
    	}
    	Temp->Comment = (TCHAR*)malloc((_tcslen(pToken)+1)*sizeof(TCHAR));
    	_tcscpy(Temp->Comment, pToken);
    }
    else {
    	Temp->Comment = NULL; // useless
    }

	//	Field 12 : Pointer Direction
	if ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) == NULL)
		return false;

	//	Field 13 : Garmin Display Format
	if ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) == NULL)
		return false;

	//	Field 14 : Proximity Distance - 0 is off any other number is valid
	if ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) == NULL)
		return false;

	//	Field 15 : Altitude - in feet (-777 if not valid)
	if ((pToken = strsep_r(NULL, TEXT(","), &pWClast)) == NULL)
		return false;

	Temp->Altitude = (double)StrToDouble(pToken, &Stop)/TOFEET;
	if(Temp->Altitude <= 0) {
		WaypointAltitudeFromTerrain(Temp);
	}

	//	Field 16 : Font Size - in points
	//	Field 17 : Font Style - 0 is normal, 1 is bold.
	//	Field 18 : Symbol Size - 17 is normal size
	//	Field 19 : Proximity Symbol Position
	//	Field 20 : Proximity Time
	//	Field 21 : Proximity or Route or Both
	//	Field 22 : File Attachment Name
	//	Field 23 : Proximity File Attachment Name
	//	Field 24 : Proximity Symbol Name

	return true;
}
