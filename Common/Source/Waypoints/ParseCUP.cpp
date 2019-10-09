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

enum class CSVState {
    UnquotedField,
    QuotedField,
    QuotedQuote,
};

std::vector<tstring> CupStringToFieldArray(const TCHAR *row) {

    std::vector<tstring> fields = {_T("")};

    if(row) {

      CSVState state = CSVState::UnquotedField;

      for( size_t n =0 ; n < _tcslen(row); n++)
      {
        const TCHAR c = row [n];
        switch (state) {
            case CSVState::UnquotedField:
                switch (c) {
                    case ',': // end of field
                              fields.push_back(_T(""));
                              break;
                    case '"': state = CSVState::QuotedField;
                              break;
                    default:  fields.back().push_back(c);
                              break; 
                }
                break;
            case CSVState::QuotedField:
                switch (c) {
                    case '"': state = CSVState::QuotedQuote;
                              break;
                    default:  fields.back().push_back(c);
                              break; 
                }
                break;
            case CSVState::QuotedQuote:
                switch (c) {
                    case ',': // , after closing quote
                              fields.push_back(_T(""));
                              state = CSVState::UnquotedField;
                              break;
                    case '"': // "" -> "
                              fields.back().push_back('"');
                              state = CSVState::QuotedField;
                              break;
                    default:  // end of quote
                              fields.back().push_back(c);
                              state = CSVState::QuotedField;
                              break; 
                }
                break;
        }
      }
    }
    return fields;
}

//#define CUPDEBUG
bool ParseCUPWayPointString(const TCHAR *String,WAYPOINT *Temp)
{
  #define   MAXBUF 128
  TCHAR Buffer[MAXBUF];
  int flags=0;
  bool ishome=false; // 100310

  Temp->Visible = true; // default all waypoints visible at start
  Temp->FarVisible = true;
  Temp->Format = LKW_CUP;
  Temp->Number = WayPointList.size();
  Temp->FileNum = globalFileNum;

  std::vector<tstring> Entries =   CupStringToFieldArray(String);

  if(Entries.size() < 11) {
    return false;
  }

  // ---------------- NAME ----------------
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
  Temp->Latitude = CUPToLat(Entries[3].c_str() );

  if((Temp->Latitude > 90) || (Temp->Latitude < -90)) {
	return false;
  }
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP LATITUDE=<%f>%s"),Temp->Latitude,NEWLINE);
  #endif


  // ---------------- LONGITUDE  ------------------
  Temp->Longitude  = CUPToLon(Entries[4].c_str());
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP LONGITUDE=<%f>%s"),Temp->Longitude,Entries[4].c_str());
  #endif
  if((Temp->Longitude  > 180) || (Temp->Longitude  < -180)) {
	return false;
  }
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP LONGITUDE=<%f>%s"),Temp->Longitude,NEWLINE);
  #endif


  // ---------------- ELEVATION  ------------------
  Temp->Altitude = ReadAltitude(Entries[5].c_str());
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
  if ((Entries[7].length()) == 1)
	Temp->RunwayDir=-1;
  else
	Temp->RunwayDir = (int)AngleLimit360(_tcstol(Entries[7].c_str(), NULL, 10));
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP RUNWAY DIRECTION=<%d>%s"),Temp->RunwayDir,NEWLINE);
  #endif


  // ---------------- RWY LENGTH   ------------------

  if (Entries[8].length() == 1)
	Temp->RunwayLen = -1;
  else
  {
    _tcsncpy(Buffer,Entries[8].c_str(),MAXBUF );
    Temp->RunwayLen = (int)ReadLength(Buffer);
  }
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
    Temp->Comment = _tcsdup( Entries[10].c_str());


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
  const TCHAR *stop=temp;
  double len = StrToDouble(temp, &stop);
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
