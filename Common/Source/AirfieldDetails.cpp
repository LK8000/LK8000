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
#include "Defines.h"
#include <regex>
#include <map>
#include <vector>
#include <sstream>

namespace {

std::map<unsigned, std::vector<tstring>> g_waypoint_images;
std::map<unsigned, std::vector<tstring>> g_waypoint_files;
std::map<unsigned, tstring> g_waypoint_reporting_point;
// name (lowercase, without A/F) -> waypoint indices; built once per ParseAirfieldDetails
std::map<tstring, std::vector<unsigned>> g_waypoint_name_to_indices;

static tstring to_lower(const tstring& s) {
  tstring r = s;
  for (auto& c : r) c = (TCHAR)_totlower((wint_t)c);
  return r;
}

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

// Resolve path relative to base directory (waynotes file directory).
tstring resolve_path(const tstring& base_dir, const tstring& path) {
  if (path.empty()) return path;
  if (base_dir.empty()) return path;
  tstring p = path;
  trim_inplace(p);
  // Skip leading path separators in path
  while (!p.empty() && (p.front() == _T('/') || p.front() == _T('\\'))) {
    p.erase(0, 1);
  }
  if (p.empty()) return base_dir;
  tstring result = base_dir;
  if (result.back() != _T('/') && result.back() != _T('\\')) {
    result += _T(DIRSEP);
  }
  result += p;
  return result;
}

// Valid reportingpoint= values: north, south, east, west (report when flying opposite direction), or * (all headings).
static bool parse_reportingpoint(const tstring& value, tstring& out) {
  if (value == _T("north") || value == _T("south") || value == _T("east") || value == _T("west")) {
    out = value;
    return true;
  }
  if (value == _T("*")) {
    out = _T("*");
    return true;
  }
  return false;
}

// Parse detail block into text (without image=/file=/reportingpoint= lines), image paths, file paths.
void parse_detail_block(const std::string& detail, const tstring& base_dir,
                        std::string& text_out, std::vector<tstring>& images_out, std::vector<tstring>& files_out,
                        tstring& reportingpoint_out) {
  text_out.clear();
  images_out.clear();
  files_out.clear();
  reportingpoint_out.clear();
  std::istringstream stream(detail);
  std::string line;
  bool first = true;
  while (std::getline(stream, line)) {
    tstring tline = from_unknown_charset(line.c_str());
    trim_inplace(tline);
    if (tline.size() >= 6 && _tcsnicmp(tline.c_str(), _T("image="), 6) == 0) {
      tstring path = tline.substr(6);
      trim_inplace(path);
      if (!path.empty()) {
        images_out.push_back(resolve_path(base_dir, path));
      }
      continue;
    }
    if (tline.size() >= 5 && _tcsnicmp(tline.c_str(), _T("file="), 5) == 0) {
      tstring path = tline.substr(5);
      trim_inplace(path);
      if (!path.empty()) {
        files_out.push_back(resolve_path(base_dir, path));
      }
      continue;
    }
    if (tline.size() >= 15 && _tcsnicmp(tline.c_str(), _T("reportingpoint="), 15) == 0) {
      tstring value = tline.substr(15);
      trim_inplace(value);
      for (auto& c : value) c = (TCHAR)_totlower((wint_t)c);
      tstring rp;
      if (reportingpoint_out.empty() && parse_reportingpoint(value, rp)) {
        reportingpoint_out = rp;
      }
      continue;
    }
    if (!first) text_out += '\n';
    text_out += line;
    first = false;
  }
  trim_inplace(text_out);
}

void SetAirfieldDetail(const TCHAR* Name, const TCHAR* Details,
                       const std::vector<tstring>* pImages, const std::vector<tstring>* pFiles,
                       const TCHAR* reportingpoint) {
  const auto [lookup_name, lookup_flag] = get_lookup(Name);

  bool isHome = lookup_flag == _T("HOME");
  bool isPreferred = lookup_flag == _T("PREF") || lookup_flag == _T("PREFERRED");

  tstring key = to_lower(lookup_name);
  auto it = g_waypoint_name_to_indices.find(key);
  if (it == g_waypoint_name_to_indices.end()) return;

  for (unsigned i : it->second) {
    auto& wp = WayPointList[i];

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
    if (pImages) {
      g_waypoint_images[i] = *pImages;
    }
    if (pFiles) {
      g_waypoint_files[i] = *pFiles;
    }
    if (reportingpoint && reportingpoint[0]) {
      g_waypoint_reporting_point[i] = reportingpoint;
    } else {
      g_waypoint_reporting_point.erase(i);
    }
  }
}

void SetAirfieldDetail(const std::string& Name, const std::string& text_detail,
                       const std::vector<tstring>& images, const std::vector<tstring>& files,
                       const TCHAR* reportingpoint) {
  tstring tname = from_unknown_charset(Name.c_str());
  trim_inplace(tname);

  if (tname.empty()) return;

  tstring tdetails = from_unknown_charset(text_detail.c_str());
  trim_inplace(tdetails);

  SetAirfieldDetail(tname.c_str(),
                   tdetails.empty() ? nullptr : tdetails.c_str(),
                   images.empty() ? nullptr : &images,
                   files.empty() ? nullptr : &files,
                   reportingpoint);
}

void ParseAirfieldDetails() {

  g_waypoint_images.clear();
  g_waypoint_files.clear();
  g_waypoint_reporting_point.clear();

  zzip_stream stream;
  tstring base_dir;

  if (_tcslen(szAirfieldFile)>0) {
    TCHAR zfilename[MAX_PATH];
    LocalPath(zfilename, _T(LKD_WAYPOINTS), szAirfieldFile);
    stream.open(zfilename, "rb");
    base_dir = zfilename;
    size_t pos = base_dir.find_last_of(_T("\\/"));
    if (pos != tstring::npos) {
      base_dir = base_dir.substr(0, pos);
    } else {
      base_dir.clear();
    }
  }

  if(!stream) {
    StartupStore(_T(". open AirfieldFile FAILED <%s>"), szAirfieldFile);
    return;
  }

  StartupStore(_T(". open AirfieldFile <%s>"), szAirfieldFile);

  g_waypoint_name_to_indices.clear();
  for (unsigned i = NUMRESWP; i < WayPointList.size(); ++i) {
    tstring key = to_lower(get_wp_mane(WayPointList[i].Name));
    g_waypoint_name_to_indices[key].push_back(i);
  }

  std::string name;
  std::string new_name;
  std::string detail;

  enum class state { comment, line_start, name, detail };

  state s = state::line_start;

  auto apply_section = [&base_dir](const std::string& section_name, const std::string& section_detail) {
    std::string text_detail;
    std::vector<tstring> image_list;
    std::vector<tstring> file_list;
    tstring reportingpoint;
    parse_detail_block(section_detail, base_dir, text_detail, image_list, file_list, reportingpoint);
    if (!image_list.empty() || !file_list.empty()) {
      tstring tsection = from_unknown_charset<tstring::value_type>(section_name.c_str());
      StartupStore(_T(". waynotes [%s] images=%u files=%u%s"),
                   tsection.c_str(),
                   (unsigned)image_list.size(), (unsigned)file_list.size(), NEWLINE);
    }
    SetAirfieldDetail(section_name, text_detail, image_list, file_list,
                     reportingpoint.empty() ? nullptr : reportingpoint.c_str());
  };

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
            // start of next section: apply previous section (name + detail) before reading new name
            if (!name.empty()) {
              apply_section(name, detail);
              detail.clear();
            }
            new_name.clear();
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
            // end of section name; content for this section will be read into detail until next '['
            s = state::line_start;
            name = std::exchange(new_name, {});
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

  apply_section(name, detail);
}

}  // namespace

const std::vector<tstring>* GetWaypointImages(unsigned waypoint_index) {
  auto it = g_waypoint_images.find(waypoint_index);
  if (it == g_waypoint_images.end() || it->second.empty()) return nullptr;
  return &it->second;
}

const std::vector<tstring>* GetWaypointFiles(unsigned waypoint_index) {
  auto it = g_waypoint_files.find(waypoint_index);
  if (it == g_waypoint_files.end() || it->second.empty()) return nullptr;
  return &it->second;
}

const TCHAR* GetWaypointReportingPoint(unsigned waypoint_index) {
  auto it = g_waypoint_reporting_point.find(waypoint_index);
  if (it == g_waypoint_reporting_point.end() || it->second.empty()) return nullptr;
  return it->second.c_str();
}

void ReadAirfieldFile() {
  // LKTOKEN  _@M400_ = "Loading Waypoint Notes File..."
  CreateProgressDialog(MsgToken<400>());
  ParseAirfieldDetails();
}
