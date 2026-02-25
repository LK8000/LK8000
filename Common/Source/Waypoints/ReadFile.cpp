/*
   LK8000 Tactical Flight Computer -  WWW.LK8000.IT
   Released under GNU/GPL License v.2 or later
   See CREDITS.TXT file for authors and copyrights

   $Id$
*/

#include "externs.h"
#include "Waypointparser.h"
#include "Dialogs/dlgProgress.h"
#include "resource.h"
#include "utils/charset_helper.h"
#include <format>

extern int globalFileNum;

// returns -1 if error, or the WpFileType
int ReadWayPointFile(std::istream& stream, int fileformat) {
  WAYPOINT new_waypoint{};
  int nLineNumber = 0;

  cup_header_t cup_header;

  CreateProgressDialog(MsgToken<903>());  // Loading Waypoints File...

  std::string src_line;

  // check file format
  bool fempty = true;
  while (std::getline(stream, src_line)) {
    trim_inplace(src_line);
    if (src_line.empty()) {
      continue;  // Skip empty line
    }
    if (src_line.front() == 'B') {
      continue;  // Skip encoding
    }

    if (src_line.starts_with("G  WGS 84") || src_line.starts_with("G WGS 84") ||
        src_line.starts_with("\xEF\xBB\xBFG  WGS 84")) {
      if (!std::getline(stream, src_line)) {
        StartupStore(
            _T(". Waypoint file %d format: CompeGPS truncated, rejected"),
            globalFileNum);
        return -1;
      }
      if (src_line.empty()) {
        StartupStore(
            _T(". Waypoint file %d format: CompeGPS MISSING second U line, ")
            _T("rejected"),
            globalFileNum);
        return -1;
      }
      if (src_line.starts_with("U  0") || src_line.starts_with("U 0")) {
        StartupStore(
            _T(". Waypoint file %d format: CompeGPS with UTM coordinates ")
            _T("UNSUPPORTED"),
            globalFileNum);
        return -1;
      }
      if (src_line.starts_with("U  1") || src_line.starts_with("U 1")) {
        StartupStore(
            _T(". Waypoint file %d format: CompeGPS unknown U field, rejected"),
            globalFileNum);
        return -1;
      }

      StartupStore(
          _T(". Waypoint file %d format: CompeGPS, LatLon coordinates"),
          globalFileNum);
      fempty = false;
      fileformat = LKW_COMPE;
      break;
    }

    if (src_line.starts_with("name,code,country") ||
        src_line.starts_with("Title,Code,Country")) {
      StartupStore(_T(". Waypoint file %d format: SeeYou"), globalFileNum);
      cup_header = CupStringToHeader(src_line);
      fempty = false;
      fileformat = LKW_CUP;
      break;
    }
    if (src_line.starts_with("OziExplorer Waypoint File") ||
        src_line.starts_with("\xEF\xBB\xBFOziExplorer Waypoint File")) {
      StartupStore(_T(". Waypoint file %d format: OziExplorer"), globalFileNum);
      fempty = false;
      fileformat = LKW_OZI;
      break;
    }

    // consider also the case of empty file, when a waypoint if saved starting
    // with numbering after the virtual wps (including the 0); Warning, using
    // virtualdatheader 3 tcsncmp because virtuals are now more than 9.
    std::string virtualdatheader = std::format("{},", RESWP_END + 2);
    if (src_line.starts_with("1,") || src_line.starts_with(virtualdatheader)) {
      StartupStore(_T(". Waypoint file %d format: WinPilot"), globalFileNum);
      fempty = false;
      fileformat = LKW_DAT;
      break;
    }
    // Otherwise we use the fileformat .xxx suffix.
    // Why we did not do it since the beginning? Simply because we should not
    // rely on .xxx suffix because some formats like CompeGPS and OZI, for
    // example, share the same .WPT suffix.
    //
    if (fileformat < 0) {
      tstring String = from_unknown_charset(src_line.c_str());
      StartupStore(_T(".. Unknown WP header, unknown format in <%s>"),
                   String.c_str());
      // leaving fempty true, so no good file available
      break;
    }
    else {
      fempty = false;
      tstring String = from_unknown_charset(src_line.c_str());
      StartupStore(_T(".. Unknown WP header, using format %d.  Header: <%s>"),
                   fileformat, String.c_str());
      break;
    }
  }
  if (fempty) {
    return -1;
  }

  // a real shame, too lazy to change into do while loop
  // Skip already read lines containing header, unless we are using DAT, which
  // has no header
  if (fileformat == LKW_DAT) {
    goto goto_inloop;
  }

  while (std::getline(stream, src_line)) {
  goto_inloop:
    nLineNumber++;
    trim_inplace(src_line);
    if (src_line.empty()) {
      continue;  // Skip empty line
    }
    if (src_line.front() == '*') {
      continue;  // Skip comment
    }

    new_waypoint.Details.clear();
    new_waypoint.Comment.clear();

    if (fileformat == LKW_DAT || fileformat == LKW_XCW) {
      tstring String = from_unknown_charset(src_line.c_str());
      if (ParseDAT(String.c_str(), &new_waypoint)) {
        if ((_tcscmp(new_waypoint.Name, LKGetText(TEXT(RESWP_TAKEOFF_NAME))) ==
             0) &&
            (new_waypoint.Number == RESWP_ID)) {
          StartupStore(_T("... FOUND TAKEOFF (%s) INSIDE WAYPOINTS FILE%s"),
                       LKGetText(TEXT(RESWP_TAKEOFF_NAME)), NEWLINE);
          WayPointList[RESWP_TAKEOFF] = new_waypoint;
          continue;
        }

        if (WaypointInTerrainRange(&new_waypoint)) {
          if (!AddWaypoint(new_waypoint)) {
            return -1;  // failed to allocate
          }
        }
      }
    }
    else if (fileformat == LKW_CUP) {
      if (src_line.starts_with("-----Related Tasks")) {
        break;
      }
      if (ParseCUPWayPointString(cup_header, src_line, &new_waypoint)) {
        if ((_tcscmp(new_waypoint.Name, LKGetText(TEXT(RESWP_TAKEOFF_NAME))) ==
             0) &&
            (new_waypoint.Number == RESWP_ID)) {
          StartupStore(_T("... FOUND TAKEOFF (%s) INSIDE WAYPOINTS FILE%s"),
                       LKGetText(TEXT(RESWP_TAKEOFF_NAME)), NEWLINE);
          WayPointList[RESWP_TAKEOFF] = new_waypoint;
          continue;
        }

        if (WaypointInTerrainRange(&new_waypoint)) {
          if (!AddWaypoint(new_waypoint)) {
            return -1;  // failed to allocate
          }
        }
      }
    }
    else if (fileformat == LKW_COMPE) {
      tstring String = from_unknown_charset(src_line.c_str());
      if (ParseCOMPEWayPointString(String.c_str(), &new_waypoint)) {
        if ((_tcscmp(new_waypoint.Name, LKGetText(TEXT(RESWP_TAKEOFF_NAME))) ==
             0) &&
            (new_waypoint.Number == RESWP_ID)) {
          StartupStore(_T("... FOUND TAKEOFF (%s) INSIDE WAYPOINTS FILE%s"),
                       LKGetText(TEXT(RESWP_TAKEOFF_NAME)), NEWLINE);
          WayPointList[RESWP_TAKEOFF] = new_waypoint;
          continue;
        }

        if (WaypointInTerrainRange(&new_waypoint)) {
          if (!AddWaypoint(new_waypoint)) {
            return -1;  // failed to allocate
          }
        }
      }
    }
    else if (fileformat == LKW_OZI) {
      // Ignore first four header lines
      if (nLineNumber <= 3) {
        continue;
      }

      tstring String = from_unknown_charset(src_line.c_str());
      if (ParseOZIWayPointString(String.c_str(), &new_waypoint)) {
        if ((_tcscmp(new_waypoint.Name, LKGetText(TEXT(RESWP_TAKEOFF_NAME))) ==
             0) &&
            (new_waypoint.Number == RESWP_ID)) {
          StartupStore(_T("... FOUND TAKEOFF (%s) INSIDE WAYPOINTS FILE%s"),
                       LKGetText(TEXT(RESWP_TAKEOFF_NAME)), NEWLINE);
          WayPointList[RESWP_TAKEOFF] = new_waypoint;
          continue;
        }

        if (WaypointInTerrainRange(&new_waypoint)) {
          if (!AddWaypoint(new_waypoint)) {
            return -1;  // failed to allocate
          }
        }
      }
    }
  }

  return fileformat;
}
