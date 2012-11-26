/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"



extern int globalFileNum;


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
  LK_tcsncpy(tName,&tString[3],j);
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
  LK_tcsncpy(tLatitude,&tString[i],p-i);

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
  LK_tcsncpy(tLongitude,&tString[i],p-i);

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
  LK_tcsncpy(tAltitude,&tString[i-1],p-i);
  
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
  LK_tcsncpy(tComment,&tString[i],p-i);

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
	if (Temp->Comment) _tcscpy(Temp->Comment,tComment);
  } else
	Temp->Comment=NULL; //@ 101104



 return true;

 }


