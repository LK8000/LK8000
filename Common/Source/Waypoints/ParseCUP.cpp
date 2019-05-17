/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "LKStyle.h"


extern int globalFileNum;
/*  // no longer needed
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
*/

enum class CSVState {
    UnquotedField,
    QuotedField,
    QuotedQuote,
};

std::vector<std::basic_string<TCHAR> > readCSVRow(TCHAR *row) {

    CSVState state = CSVState::UnquotedField;
    std::vector<std::basic_string<TCHAR> > fields = {_T("")};

    size_t i = 0; // index of the current field
    if(row != NULL)

      for( uint n =0 ; n < _tcslen(row); n++)
      {
	TCHAR c = row [n];
        switch (state) {
            case CSVState::UnquotedField:
                switch (c) {
                    case ',': // end of field
                              fields.push_back(_T("")); i++;
                              break;
                    case '"': state = CSVState::QuotedField;
                              break;

                    default:  fields[i].push_back(c);
                              break; }
                break;
            case CSVState::QuotedField:
                switch (c) {
                    case '"': state = CSVState::QuotedQuote;
                              break;
                    default:  fields[i].push_back(c);
                              break; }
                break;
            case CSVState::QuotedQuote:
                switch (c) {
                    case ',': // , after closing quote
                              fields.push_back(_T("")); i++;
                              state = CSVState::UnquotedField;
                              break;
                    case '"': // "" -> "
                              fields[i].push_back('"');
                              state = CSVState::QuotedField;
                              break;
                    default:  // end of quote
                              fields[i].push_back(c);
                              state = CSVState::QuotedField;
                              break; }
                break;
        }
    }
    return fields;
}

//#define CUPDEBUG
bool ParseCUPWayPointString(TCHAR *String,WAYPOINT *Temp)
{
  TCHAR TempString[READLINE_LENGTH+1];
  TCHAR OrigString[READLINE_LENGTH+1];

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
//  pToken = _tcstok(TempString, TEXT("\""));  44
  std::vector<std::basic_string<TCHAR>> Entries =   readCSVRow(TempString);


  _sntprintf(Temp->Name,NAME_SIZE, _T("%s"), Entries[0].c_str() );


#ifdef CUPDEBUG
  StartupStore(_T("   CUP NAME=<%s>%s"),Temp->Name,NEWLINE);
#endif


  // ---------------- CODE ------------------
  _sntprintf(Temp->Code,CUPSIZE_CODE, _T("%s"), Entries[1].c_str() );
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP CODE=<%s>%s"),Temp->Code,NEWLINE);
  #endif


  // ---------------- COUNTRY ------------------
  _sntprintf(Temp->Country,CUPSIZE_COUNTRY, _T("%s"), Entries[2].c_str() );
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP COUNTRY=<%s>%s"),Temp->Country,NEWLINE);
  #endif


  // ---------------- LATITUDE  ------------------
  Temp->Latitude = CUPToLat((TCHAR*)Entries[3].c_str() );

  if((Temp->Latitude > 90) || (Temp->Latitude < -90)) {
	return false;
  }
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP LATITUDE=<%f>%s"),Temp->Latitude,NEWLINE);
  #endif


  // ---------------- LONGITUDE  ------------------
  Temp->Longitude  = CUPToLon((TCHAR*)Entries[4].c_str());
  if((Temp->Longitude  > 180) || (Temp->Longitude  < -180)) {
	return false;
  }
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP LONGITUDE=<%f>%s"),Temp->Longitude,NEWLINE);
  #endif


  // ---------------- ELEVATION  ------------------
  Temp->Altitude = ReadAltitude((TCHAR*)Entries[5].c_str());
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP ELEVATION=<%f>%s"),Temp->Altitude,NEWLINE);
  #endif
  if (Temp->Altitude == -9999){
	  Temp->Altitude=0;
  }


  // ---------------- STYLE  ------------------
  Temp->Style = (int)_tcstol(Entries[6].c_str(),NULL,10);
  switch(Temp->Style) {
	case STYLE_AIRFIELDGRASS:	// airfield grass
	case STYLE_GLIDERSITE:		// glider site
	case STYLE_AIRFIELDSOLID:	// airfield solid
		flags = AIRPORT;
		flags += LANDPOINT;
		break;
	case STYLE_OUTLANDING:		// outlanding
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
  if (_tcslen((TCHAR*)Entries[7].c_str()) == 1)
	Temp->RunwayDir=-1;
  else
	Temp->RunwayDir = (int)AngleLimit360(_tcstol(Entries[7].c_str(), NULL, 10));
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP RUNWAY DIRECTION=<%d>%s"),Temp->RunwayDir,NEWLINE);
  #endif


  // ---------------- RWY LENGTH   ------------------
  if (_tcslen((TCHAR*)Entries[8].c_str()) == 1)
	Temp->RunwayLen = -1;
  else
	Temp->RunwayLen = (int)ReadLength((TCHAR*)Entries[8].c_str());
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP RUNWAY LEN=<%d>%s"),Temp->RunwayLen,NEWLINE);
  #endif



  // ---------------- AIRPORT FREQ   ------------------
  _sntprintf(Temp->Freq,CUPSIZE_FREQ, _T("%s"), Entries[9].c_str() );

  #ifdef CUPDEBUG
  StartupStore(_T("   CUP FREQ=<%s>%s"),Temp->Freq,NEWLINE);
  #endif


  // ---------------- COMMENT   ------------------
  if (Entries[10].length() >0 ) {
    if (Temp->Comment) {
      free(Temp->Comment);
    }
    Temp->Comment = (TCHAR*)malloc((Entries[10].length()+1)*sizeof(TCHAR));
    if (Temp->Comment)  _sntprintf(Temp->Comment,COMMENT_SIZE, _T("%s"), Entries[10].c_str() );


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
  if (stop){	// number converted endpointer is set

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
