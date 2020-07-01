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

std::map<tstring, size_t> CupStringToHeader(const TCHAR *row) {
  const std::map<tstring, tstring> alias = {
    { _T("Title"),       _T("name") },
    { _T("Code"),        _T("code") },
    { _T("Country"),     _T("country") },
    { _T("Latitude"),    _T("lat") },
    { _T("Longitude"),   _T("lon") },
    { _T("Elevation"),   _T("elev") },
    { _T("Style"),       _T("style") },
    { _T("Direction"),   _T("rwdir") },
    { _T("Length"),      _T("rwlen") },
    { _T("Frequency"),   _T("freq") },
    { _T("Description"), _T("desc") }
  };

  std::map<tstring, size_t> header;
  const auto entries = CupStringToFieldArray(row);
  for (size_t i = 0; i < entries.size(); ++i) {
    const tstring& text = entries[i];
    tstring lower_text;
    std::transform(text.begin(), text.end(), std::back_inserter(lower_text), ::tolower);
    auto it = alias.find(lower_text);
    if (it != alias.end()) {
      lower_text = it->second;
    }
    header[lower_text] = i;
  }
  return header;
}

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

namespace {
  class cup_line {
  public:
    cup_line(const std::map<tstring, size_t>& _Headers, std::vector<tstring>&& _Entries) 
          : Headers(_Headers), Entries(std::forward<std::vector<tstring>>(_Entries)) {}

    const tstring& operator[](const TCHAR* Name) {
      auto it = Headers.find(Name);
      if(it != Headers.end()) {
        return Entries[it->second];
      }
      return empty;
    }

    size_t size() {
      return Entries.size();
    }

  private:
    const tstring empty = _T("");
    const std::map<tstring, size_t>& Headers;
    const std::vector<tstring> Entries;
  };
}

//#define CUPDEBUG
bool ParseCUPWayPointString(const std::map<tstring, size_t>& cup_header, const TCHAR *String,WAYPOINT *Temp)
{
  int flags=0;
  bool ishome=false; // 100310

  Temp->Visible = true; // default all waypoints visible at start
  Temp->FarVisible = true;
  Temp->Format = LKW_CUP;
  Temp->Number = WayPointList.size();
  Temp->FileNum = globalFileNum;

  cup_line Entries(cup_header, CupStringToFieldArray(String));

  if(Entries.size() < 11) {
    return false;
  }

  // ---------------- NAME ----------------
  _sntprintf(Temp->Name,NAME_SIZE, _T("%s"), Entries[_T("name")].c_str());
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP NAME=<%s>%s"),Temp->Name,NEWLINE);
  #endif


  // ---------------- CODE ------------------
  _sntprintf(Temp->Code,CUPSIZE_CODE, _T("%s"),  Entries[_T("code")].c_str() );
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP CODE=<%s>%s"),Temp->Code,NEWLINE);
  #endif


  // ---------------- COUNTRY ------------------
  _sntprintf(Temp->Country,CUPSIZE_COUNTRY, _T("%s"),  Entries[_T("country")].c_str() );
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP COUNTRY=<%s>%s"),Temp->Country,NEWLINE);
  #endif


  // ---------------- LATITUDE  ------------------
  Temp->Latitude = CUPToLat( Entries[_T("lat")].c_str() );

  if((Temp->Latitude > 90) || (Temp->Latitude < -90)) {
	return false;
  }
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP LATITUDE=<%f>%s"),Temp->Latitude,NEWLINE);
  #endif


  // ---------------- LONGITUDE  ------------------
  Temp->Longitude  = CUPToLon( Entries[_T("lon")].c_str());
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
  Temp->Altitude = ReadAltitude( Entries[_T("elev")].c_str());
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP ELEVATION=<%f>%s"),Temp->Altitude,NEWLINE);
  #endif
  if (Temp->Altitude == -9999){
	  Temp->Altitude=0;
  }


  // ---------------- STYLE  ------------------
  Temp->Style = (int)_tcstol( Entries[_T("style")].c_str(),NULL,10);
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
  const tstring& rwdir =  Entries[_T("rwdir")];
  if ((rwdir.length()) == 1) {
    Temp->RunwayDir=-1;
  } else {
    Temp->RunwayDir = (int)AngleLimit360(_tcstol(rwdir.c_str(), NULL, 10));
  }
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP RUNWAY DIRECTION=<%d>%s"),Temp->RunwayDir,NEWLINE);
  #endif


  // ---------------- RWY LENGTH   ------------------
  const tstring& rwlen =  Entries[_T("rwlen")];
  if (rwlen.length() == 1) {
    Temp->RunwayLen = -1;
  } else {
    Temp->RunwayLen = (int)ReadLength(rwlen.c_str());
  }
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP RUNWAY LEN=<%d>%s"),Temp->RunwayLen,NEWLINE);
  #endif



  // ---------------- AIRPORT FREQ   ------------------
  const tstring& freq =  Entries[_T("freq")];
  _sntprintf(Temp->Freq,CUPSIZE_FREQ, _T("%s"), freq.c_str() );

  #ifdef CUPDEBUG
  StartupStore(_T("   CUP FREQ=<%s>%s"),Temp->Freq,NEWLINE);
  #endif


  // ---------------- COMMENT   ------------------
  SetWaypointComment(*Temp,  Entries[_T("desc")].c_str());
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP COMMENT=<%s>%s"),Temp->Comment,NEWLINE);
  #endif

  if(Temp->Altitude <= 0) {
    WaypointAltitudeFromTerrain(Temp);
  }

  if (Temp->Details) {
    free(Temp->Details);
  }

  return true;
}



double ReadLength(const TCHAR *temp)
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
