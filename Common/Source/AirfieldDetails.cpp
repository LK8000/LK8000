/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id: AirfieldDetails.cpp,v 8.3 2010/12/10 22:13:23 root Exp root $
*/


#include "externs.h"
#include "Dialogs/dlgProgress.h"
#include "AirfieldDetails.h"
#include "Waypointparser.h"
#include "utils/zzip_stream.h"
#include "LocalPath.h"
#include "utils/charset_helper.h"
#include <regex>

namespace {

// get waypoint name without A/F suffix
tstring get_wp_mane(const TCHAR* Name) {
  const std::basic_regex<TCHAR> re(_T(R"(^(.*?)(?: A\/?F)?$)"));
  std::match_results<const TCHAR*> match;
  if (!std::regex_match(Name, match, re)) {
    return Name;
  }
  return match[1].str();
}

// extract waypoint name and flag from the lookup string
std::tuple<tstring, tstring> get_lookup(const TCHAR* Name) {
  const std::basic_regex<TCHAR> re(_T(R"(^(.*?)(?:=((?:PREF)|(?:HOME)|(?:PREFERRED)))?$)"));
  std::match_results<const TCHAR*> match;
  if (!std::regex_match(Name, match, re)) {
    return { Name, _T("") };
  }
  return { match[1], match[2] };
}

bool compare_nocase(const tstring& a, const tstring& b) {
  return _tcsicmp(a.c_str(), b.c_str()) == 0;
}

void SetAirfieldDetail(const TCHAR* Name, const TCHAR* Details) {
  const auto [lookup_name, lookup_flag] = get_lookup(Name);

  bool isHome = lookup_flag == _T("HOME");
  bool isPreferred = lookup_flag == _T("PREF") || lookup_flag == _T("PREFERRED");

  for (unsigned i = NUMRESWP; i < WayPointList.size(); ++i) {
    auto& wp = WayPointList[i];
    auto wp_name = get_wp_mane(wp.Name);

    if (!compare_nocase(lookup_name, wp_name)) {
      continue;
    }

    if (isHome) {
      if (wp.Flags & (AIRPORT | LANDPOINT)) {
        WayPointCalc[i].Preferred = true;
      }
      HomeWaypoint = AirfieldsHomeWaypoint = i;  // make it survive a reset..
    }

    if (isPreferred) {
      WayPointCalc[i].Preferred = true;
    }

    if (Details) {
      SetWaypointDetails(wp, Details);
    }
  }
}

void SetAirfieldDetail(const std::string& Name, const std::string& Details) {
  tstring tname = from_unknown_charset(Name.c_str());
  trim_inplace(tname);

  tstring tdetails = from_unknown_charset(Details.c_str());
  trim_inplace(tdetails);

  if (!tname.empty()) {
    SetAirfieldDetail(tname.c_str(), tdetails.empty() ? nullptr : tdetails.c_str());
  }
}

void ParseAirfieldDetails() {

  zzip_stream stream;

  if (_tcslen(szAirfieldFile)>0) {
    TCHAR zfilename[MAX_PATH];
    LocalPath(zfilename, _T(LKD_WAYPOINTS), szAirfieldFile);
    stream.open(zfilename, "rb");
  }

  if(!stream) {
    StartupStore(_T(". open AirfieldFile FAILED <%s>"), szAirfieldFile);
    return;
  }

  StartupStore(_T(". open AirfieldFile <%s>"), szAirfieldFile);

  std::string name;
  std::string new_name;
  std::string detail;

  enum class state { comment, line_start, name, detail };

  state s = state::line_start;

  for (std::istreambuf_iterator<char> it(&stream); it != std::istreambuf_iterator<char>(); ++it) {
    switch (s) {
      case state::comment:
        switch (*it) {
          // skip comment lines
          case '\r':
          case '\n':
            s = state::line_start;
            break;
        }
        break;
      case state::line_start:
        switch (*it) {
          case '#':
            // start of comment
            s = state::comment;
            break;
          case '[':
            // start of airfield name
            s = state::name;
            break;
          case '\r':
          case '\n':
            break;
          default:
            s = state::detail;
            detail += *it;
            break;
        }
        break;
      case state::name:
        switch (*it) {
          case ']':
            // end of airfield name
            s = state::line_start;
            SetAirfieldDetail(name, detail);
            name = std::exchange(new_name, {});
            detail.clear();
            break;
          case '\r':
          case '\n':
            // if we have newlines in the name, it's not a valid airfield name
            // append to detail instead
            s = state::detail;
            detail += '[' + new_name + '\n';
            new_name.clear();
            break;
          default:
            new_name += *it;
            break;
        }
        break;
      case state::detail:
        switch (*it) {
          case '\r':
          case '\n':
            s = state::line_start;
            detail += '\n';
            break;
          default:
            detail += *it;
            break;
        }
        break;
    }
  }

  SetAirfieldDetail(name, detail);
}

}  // namespace

void ReadAirfieldFile() {
  // LKTOKEN  _@M400_ = "Loading Waypoint Notes File..."
  CreateProgressDialog(MsgToken<400>());
  ParseAirfieldDetails();
}
