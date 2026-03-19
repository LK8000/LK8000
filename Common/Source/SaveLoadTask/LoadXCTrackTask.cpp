/*
 * LK8000 Tactical Flight Computer -  WWW.LK8000.IT
 * Released under GNU/GPL License v.2 or later
 * See CREDITS.TXT file for authors and copyrights
 *
 * File:   LoadXCTrackTask.cpp
 * Author: Bruno de Lacheisserie
 */

#include "Compiler.h"
#include "externs.h"
#include "Util/ScopeExit.hxx"
#include "Util/Clamp.hpp"
#include "utils/zzip_file_stream.h"
#include "utils/charset_helper.h"
#include <string>
#include <cassert>
#include "Waypointparser.h"
#include "Waypoints/SetHome.h"
#include "Calc/Task/TimeGates.h"
#include "nlohmann/json.hpp"

namespace {

using json = nlohmann::json;
using polyline_t = std::vector<int32_t>;

/**
 * XCTrack polyline implementation decoder 
 * https://gitlab.com/xcontest-public/xctrack-public/snippets/1927372
 */

template<typename iterator = std::string::const_iterator>
double polyline_decode(iterator& it, iterator end) {
  assert(it != end); // empty string

  int32_t shift = 0;
  int32_t value = 0;
  uint8_t byte = 0;

  do {
    byte = static_cast<uint8_t>(*(it++) - 63);
    value |= (byte & 0x1f) << shift;
    shift += 5;
  } while (byte > 0x1f);

  return (value & 0x01) ? ~(value >> 1) : value >> 1;
}

polyline_t polyline_decode(const std::string& string) {

  polyline_t polyline;
  auto it = std::begin(string);
  while (it != std::end(string)) {
    polyline.push_back(polyline_decode(it, std::end(string)));
  }
  return polyline;
}

/**
 * helper to create Waypoint from json data
 */
struct waypoint_helper : public WAYPOINT {
  waypoint_helper(const waypoint_helper&) = delete;

  waypoint_helper() : WAYPOINT{} { }

  explicit waypoint_helper(const json& waypoint) : WAYPOINT{} {
      set_name(waypoint.value("name", "").c_str());

      Longitude = waypoint.value("lon", 0.0);
      Latitude = waypoint.value("lat", 0.0);
      Altitude = waypoint.value("altSmoothed", 0.0);

      std::string desc_str = waypoint.value("description", "");
      if (!desc_str.empty()) {
        set_description(desc_str.c_str());
      }
  }

  waypoint_helper(const std::string& name, const polyline_t& polyline) : WAYPOINT{} {
      set_name(name.c_str());

      assert(polyline.size() >= 3);

      Longitude = polyline[0] / 1e5;
      Latitude = polyline[1] / 1e5;
      Altitude = polyline[2];
  }

  void set_name(const char* utf8_string) {
    from_utf8(utf8_string, Name);
  }

  void set_description(const char* utf8_string) {
    Comment = from_utf8(utf8_string);
  }

};

void StrToLocalTime(const char* string, int& Hour, int& Min) {
  char* sz = nullptr;
  if (string) {
    Hour = Clamp<int>(strtol(string, &sz, 10), 0, 23);
    if (*sz == _T(':')) {
      Min = Clamp<int>(strtol(sz + 1, &sz, 10), 0, 59);
    }

    int Time = (((Hour * 60) + Min) * 60) + GetUTCOffset();
    Hour = Time / 3600;
    Min = (Time - (Hour * 3600)) / 60;
  }
}

void ParseTimeGates(const json &timegates) {
  if (!timegates.is_array()) {
    return;
  }
  if (timegates.empty()) {
    return;
  }
  if (!timegates[0].is_string()) {
    return;
  }
  std::string first_str_time = timegates[0];
  // Time of first gate in local timezone
  StrToLocalTime(first_str_time.c_str(), TimeGates::PGOpenTimeH, TimeGates::PGOpenTimeM);
  // How many gates, 1-x
  TimeGates::PGNumberOfGates = timegates.size();
  if (TimeGates::PGNumberOfGates > 1 && timegates[1].is_string()) {
    // Interval, in minutes
    const std::string& second_str_time = timegates[1];
    int H, M;
    StrToLocalTime(second_str_time.c_str(), H, M);
    TimeGates::PGGateIntervalTime = (H * 60 + M) - (TimeGates::PGOpenTimeH * 60 + TimeGates::PGOpenTimeM);
  }
}

/**
 *  https://xctrack.org/Competition_Interfaces.html#task-definition-format
 */
bool LoadXctrackTask_V1(const json& task_json) {
  auto task_it = std::begin(Task);

  const json& turnpoints = task_json.value("turnpoints", json::array());
  for (const auto& tp : turnpoints) {

    waypoint_helper newPoint(tp.value("waypoint", json::object()));

    const json& type = tp.value("type", json());
    if (type.is_string()) {
      const std::string& type_str = type;
      if (type_str == "TAKEOFF") {
        // TODO : set as HOME
      }
      else if (type_str == "SSS") {

        // !! warning if not the first after takeoff ( unmanaged by LK8000 )
        if (std::distance(std::begin(Task), task_it) >= 1) {
          // set first tp as "HomeWaypoint"
          SetNewHome(Task[0].Index);
        }

        StartLine = sector_type_t::CIRCLE;
        StartRadius = tp.value("radius", 0.0);

        task_it = std::begin(Task); // always start task with Start of Speed Turnpoint
        task_it->AATType = sector_type_t::CIRCLE;
        task_it->AATCircleRadius = StartRadius;
      }
      else if (type_str == "ESS") {
        task_it->AATType = sector_type_t::ESS_CIRCLE;
      } else {
        task_it->AATType = sector_type_t::CIRCLE;
      }
    } else {
      task_it->AATType = sector_type_t::CIRCLE;
    }
  
    task_it->Index = FindOrAddWaypoint(&newPoint, false);
    task_it->AATCircleRadius = tp.value("radius", 0.0);

    ++task_it;
  }

#if __TODO__
  // TODO : currently not managed in LK8000
  const json::value& takeoff = task_json.get("takeoff");
  if (takeoff.is<json::object>()) {
    const json::value& timeopen = takeoff.get("timeOpen");
    const json::value& timeclose = takeoff.get("timeClose");
  }
#endif

  const json& sss = task_json.value("sss", json());
  if (sss.is_object()) {
    const std::string type = sss.value("type", ""); // string - one of "RACE" / "ELAPSED-TIME", required
    bool elapsed_time = (type == "ELAPSED-TIME");
    if(!elapsed_time) { // ignore timesgates in case of ELAPSED-TIME ( not managed in LK8000)
      ParseTimeGates(sss.value("timeGates", json())); // array of times, required
    }
  }


  if(task_it != std::begin(Task)) { // Empty Task ??
    auto goal_tp = std::prev(task_it);
    FinishRadius = goal_tp->AATCircleRadius;
    FinishLine = sector_type_t::CIRCLE;

    json goal = task_json.value("goal", json());
    if (goal.is_object()) {
      std::string type = goal.value("type", ""); // string - one of "CYLINDER"/"LINE", optional (default CYLINDER)
      if (type == "LINE") {
        FinishLine = sector_type_t::LINE; // LINE
      }
    }

#if __TODO__
    json deadline = goal.value("time", json()); // time, optional (default 23:00 local UTC equivalent)
#endif
  }

#ifdef _WGS84
  std::string earthModel = task_json.value("earthModel", ""); // string, optional - one of "WGS84"(default) / "FAI_SPHERE"
  earth_model_wgs84 = !(earthModel == "FAI_SPHERE");
#endif

  return true;
}

/**
 * https://xctrack.org/Competition_Interfaces.html#task-definition-format-2---for-qr-codes
 */
bool LoadXctrackTask_V2(const json& task_json) {
  const json& turnpoints = task_json.value("t", json());
  if (!turnpoints.is_array()) {
    return false;
  }

  auto task_it = std::begin(Task);

  for (const auto& tp : turnpoints) {

    std::string name = tp.value("n", "");
    std::string polyline_str = tp.value("z", "");
    polyline_t polyline = polyline_decode(polyline_str);

    waypoint_helper newPoint(name, polyline);

    std::string description = tp.value("d", "");
    if (!description.empty()) {
      newPoint.set_description(description.c_str());
    }

    json type = tp.value("t", json());
    if (type.is_number()) {
      switch (static_cast<int>(type)) {
      case 2: // SSS

        // !! warning if not the first after takeoff ( unmanaged by LK8000 )
        if (std::distance(std::begin(Task), task_it) >= 1) {
          // set first tp as "HomeWaypoint"
          SetNewHome(Task[0].Index);
        }

        task_it = std::begin(Task); // always start task with Start of Speed Turnpoint
        task_it->AATType = sector_type_t::CIRCLE;
        StartLine = sector_type_t::CIRCLE;
        StartRadius = polyline[3];
        break;
      case 3: // ESS
        task_it->AATType = sector_type_t::ESS_CIRCLE;
        break;
      default:
        task_it->AATType = sector_type_t::CIRCLE;
        break;
      }
    } else {
      task_it->AATType = sector_type_t::CIRCLE;
    }

    task_it->Index = FindOrAddWaypoint(&newPoint, false);
    task_it->AATCircleRadius = polyline[3];

    ++task_it;
  }

  const json& sss = task_json.value("s", json());
  if (sss.is_object()) {
    unsigned type = sss.value("t", 1); // number, required, one of 1 (RACE), 2 (ELAPSED-TIME)
    bool elapsed_time = (type == 2);
    if(!elapsed_time) { // ignore timesgates in case of ELAPSED-TIME ( unmanaged in )
      ParseTimeGates(sss.value("g", json())); // array of times, required - Time gates, start open time
    }
  }

  if(task_it != std::begin(Task)) { // Empty task ??
    auto goal_tp = std::prev(task_it);
    FinishRadius = goal_tp->AATCircleRadius;
    FinishLine = sector_type_t::CIRCLE;

    json goal = task_json.value("g", json());
    if (goal.is_object()) {
      
      int type = goal.value("t", 0); // number, optional one of 1 (LINE), 2 (CYLINDER) (default 2)
      if (type == 1) {
        FinishLine = sector_type_t::LINE;
      }

#if __TODO__
      // TODO : deadline is not managed in LK8000
      json deadline = goal.value("d", json()); // time, optional - Deadline (default 23:00 local time UTC equivalent 
#endif
    }
  }

#ifdef _WGS84
  int earthModel = task_json.value("e", 0); // number, optional, 0 (wgs84, default), 1(fai sphere)
  earth_model_wgs84 = !(earthModel == 1);
#endif

  return true;
}

bool LoadXctrackTask(const json& task_json) {
  if(!task_json.is_object()) {
    return false;
  }

  ScopeLock lock(CritSec_TaskData);

  ClearTask();

  gTaskType = task_type_t::GP;
  TskOptimizeRoute=true;

  if (task_json.value("taskType", "") == "CLASSIC") {
    switch (task_json.value("version", 0)) {
      case 1: // application/xctsk
        return LoadXctrackTask_V1(task_json);
      case 2: // from QR codes
        return LoadXctrackTask_V2(task_json);
    }
  }
  return false;
}

void LogError(std::exception& e) {
  StartupStore(_T(".... Invalid XCTrack task : %s"), to_tstring(e.what()).c_str());
}

} // namespace


bool LoadXctrackTask(const TCHAR* szFilePath) {
  try {
    zzip_file_stream file(szFilePath, "rb");
    if (file) {
      std::istream stream(&file);
      return LoadXctrackTask(stream);
    }
  } catch (std::exception& e) {
    LogError(e);
  }
  return false;
}

bool LoadXctrackTask(std::istream& stream) {
  try {
    return LoadXctrackTask(json::parse(stream));
  }
  catch (std::exception& e) {
    LogError(e);
  }
  return false;
}

bool LoadXctrackTask(const char* begin, const char* end) {
  try {
    std::string_view task_string(begin, std::distance(begin, end));
    return LoadXctrackTask(json::parse(task_string));
  }
  catch (std::exception& e) {
    LogError(e);
  }
  return false;
}
