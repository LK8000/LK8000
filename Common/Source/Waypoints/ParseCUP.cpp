/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "LKStyle.h"
#include "utils/lookup_table.h"
#include "utils/printf.h"
#include "utils/charset_helper.h"
#include <ranges>

extern int globalFileNum;

enum class CSVState {
    UnquotedField,
    QuotedField,
    QuotedQuote,
};

cup_header_t CupStringToHeader(std::string_view row) {

/* 
  this "alias" table is required to maintain backward compatibility 
  with cup files header used by "soaringspot.com" and "soaringweb.org"
*/
  constexpr auto alias = lookup_table<tstring_view, tstring_view>({
    { _T("title"),       _T("name") },
//    { _T("code"),        _T("code") },
//    { _T("country"),     _T("country") },
    { _T("latitude"),    _T("lat") },
    { _T("longitude"),   _T("lon") },
    { _T("elevation"),   _T("elev") },
//    { _T("style"),       _T("style") },
    { _T("direction"),   _T("rwdir") },
    { _T("length"),      _T("rwlen") },
    { _T("frequency"),   _T("freq") },
    { _T("description"), _T("desc") }
  });

  cup_header_t header;
  const auto entries = CupStringToFieldArray(row);
  for (size_t i = 0; i < entries.size(); ++i) {
    const tstring lower_text = to_lower_ascii(from_unknown_charset(entries[i].c_str()));
    header[tstring(alias.get(lower_text))] = i;
  }
  return header;
}

std::vector<std::string> CupStringToFieldArray(std::string_view row) {
  std::vector<std::string> fields = {""};

  CSVState state = CSVState::UnquotedField;

  for (auto c : row) {
    switch (state) {
      case CSVState::UnquotedField:
        switch (c) {
          case ',':  // end of field
            fields.push_back("");
            break;
          case '"':
            state = CSVState::QuotedField;
            break;
          default:
            fields.back().push_back(c);
            break;
        }
        break;
      case CSVState::QuotedField:
        switch (c) {
          case '"':
            state = CSVState::QuotedQuote;
            break;
          default:
            fields.back().push_back(c);
            break;
        }
        break;
      case CSVState::QuotedQuote:
        switch (c) {
          case ',':  // , after closing quote
            fields.push_back("");
            state = CSVState::UnquotedField;
            break;
          case '"':  // "" -> "
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
  return fields;
}

namespace {
  class cup_line {
  public:
    cup_line(const cup_header_t& _Headers, std::vector<std::string>&& _Entries) 
          : empty(), Headers(_Headers), Entries(std::forward<std::vector<std::string>>(_Entries)) {}

    const std::string& operator[](const TCHAR* Name) const {
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
    const std::string empty;
    const cup_header_t& Headers;
    const std::vector<std::string> Entries;
  };
}

//#define CUPDEBUG
bool ParseCUPWayPointString(const cup_header_t& cup_header,
                            std::string_view row, WAYPOINT* Temp) {
  int flags=0;
  bool ishome=false; // 100310

  Temp->Visible = true; // default all waypoints visible at start
  Temp->FarVisible = true;
  Temp->Format = LKW_CUP;
  Temp->Number = WayPointList.size();
  Temp->FileNum = globalFileNum;

  cup_line Entries(cup_header, CupStringToFieldArray(row));

  if(Entries.size() < 11) {
    return false;
  }

  // ---------------- NAME ----------------
  from_unknown_charset(Entries[_T("name")].c_str(), Temp->Name);

  // ---------------- CODE ------------------
  from_unknown_charset(Entries[_T("code")].c_str(), Temp->Code);

  // ---------------- COUNTRY ------------------
  from_unknown_charset(Entries[_T("country")].c_str(), Temp->Country);

  // ---------------- LATITUDE  ------------------
  Temp->Latitude = CUPToLat(Entries[_T("lat")].c_str());
  if ((Temp->Latitude > 90) || (Temp->Latitude < -90)) {
    return false;
  }

  // ---------------- LONGITUDE  ------------------
  Temp->Longitude = CUPToLon(Entries[_T("lon")].c_str());
  if ((Temp->Longitude > 180) || (Temp->Longitude < -180)) {
    return false;
  }

  // ---------------- ELEVATION  ------------------
  Temp->Altitude = ReadAltitude(Entries[_T("elev")].c_str());
  if (Temp->Altitude == -9999) {
    Temp->Altitude = 0;
  }

  // ---------------- STYLE  ------------------
  Temp->Style = (int)strtol(Entries[_T("style")].c_str(), nullptr, 10);
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
  const std::string& rwdir =  Entries[_T("rwdir")];
  if ((rwdir.length()) == 1) {
    Temp->RunwayDir=-1;
  } else {
    Temp->RunwayDir = (int)AngleLimit360(strtol(rwdir.c_str(), nullptr, 10));
  }
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP RUNWAY DIRECTION=<%d>%s"),Temp->RunwayDir,NEWLINE);
  #endif


  // ---------------- RWY LENGTH   ------------------
  const std::string& rwlen =  Entries[_T("rwlen")];
  if (rwlen.length() == 1) {
    Temp->RunwayLen = -1;
  } else {
    Temp->RunwayLen = ReadLength(rwlen.c_str());
  }
  #ifdef CUPDEBUG
  StartupStore(_T("   CUP RUNWAY LEN=<%d>%s"),Temp->RunwayLen,NEWLINE);
  #endif



  // ---------------- AIRPORT FREQ   ------------------
  const std::string& freq =  Entries[_T("freq")];
  from_unknown_charset(freq.c_str(), Temp->Freq);
#ifdef CUPDEBUG
  StartupStore(_T("   CUP FREQ=<%s>%s"), Temp->Freq, NEWLINE);
#endif

  // ---------------- COMMENT   ------------------
  tstring Comment = from_unknown_charset(Entries[_T("desc")].c_str());
  SetWaypointComment(*Temp,  Comment.c_str());

  if(Temp->Altitude <= 0) {
    WaypointAltitudeFromTerrain(Temp);
  }

  // ---------------- PICS   ------------------
  std::string pics = Entries[_T("pics")];
  if (!pics.empty()) {
    for (auto&& sub : std::views::split(pics, ';')) {
      Temp->pictures.emplace_back(sub.begin(), sub.end());
    }
  }

  return true;
}

double ReadLength(const char *temp)
{
  const char *stop=temp;
  double len = StrToDouble(temp, &stop);
  if (temp == stop) {		// error at begin
	len=-9999;
	return len;
  }
  if (stop){	// number converted endpointer is set

	if ( *stop == 'n' ) {
		len = Units::From(unNauticalMiles, len);
		return len;
	}
	if ( (*stop == 'm') && (*(stop+1) == 'l') ) {
		len = Units::From(unStatuteMiles, len);
		return len;
	}
	if ( (*stop == 'f') || (*stop == 'F') ) {
		len = Units::From(unFeet, len);
		return len;
	}
	if ( (*stop == 'm') || (*stop == '\0') ) {
		return len;
	}
  }
  len = -9999;
  return len;
}

#ifndef DOCTEST_CONFIG_DISABLE
#include <doctest/doctest.h>

TEST_CASE("ParseCUP") {

	SUBCASE("ReadLength") {
		CHECK(ReadLength("1m") == doctest::Approx(1).epsilon(0.0000001));
		CHECK(ReadLength("12345.12m") == doctest::Approx(12345.12).epsilon(0.0000001));
		CHECK(ReadLength("6.6658315335nm") == doctest::Approx(12345.12).epsilon(0.0000001));
		CHECK(ReadLength("7.6709019327ml") == doctest::Approx(12345.12).epsilon(0.0000001));
		CHECK(ReadLength("1000000.0m") == doctest::Approx(1000000.0).epsilon(0.0000001));
		CHECK(ReadLength("539.956803nm") == doctest::Approx(1000000.0).epsilon(0.0000001));
		CHECK(ReadLength("621.371192ml") == doctest::Approx(1000000.0).epsilon(0.0000001));
	}
}
#endif
