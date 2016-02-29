/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"



extern int globalFileNum;

void CleanCupCode(TCHAR* TpCode) {
    TCHAR Tname[NAME_SIZE + 1];
    // remove trailing spaces
    for (size_t i = _tcslen(TpCode) - 1; i > 1; i--) {
        if (TpCode[i] == ' ') {
            TpCode[i] = 0;
        } else {
            break;
        }
    }

    // now remove " " (if there)
    _tcscpy(Tname, TpCode);
    size_t j = 0;
    for (size_t i = 0; i < _tcslen(Tname); i++) {
        if (Tname[i] != '\"') {
            TpCode[j++] = Tname[i];
        }
    }
    TpCode[j] = _T('\0');
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
  Temp->Number = WayPointList.size();

  Temp->FileNum = globalFileNum;

  #if BUGSTOP
  // This should never happen
  LKASSERT(_tcslen(String) < sizeof(OrigString));
  #endif
  LK_tcsncpy(OrigString, String,READLINE_LENGTH);  
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

  _tcscpy(Temp->Name, pToken); 
  CleanCupCode(Temp->Name);

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
  LK_tcsncpy(Temp->Country,pToken,CUPSIZE_COUNTRY);
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
	  Temp->Altitude=0;
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
  TrimRight(Temp->Freq);
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
		if (Temp->Comment) _tcscpy(Temp->Comment, ctemp);
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



double ReadLength(TCHAR *temp)
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
