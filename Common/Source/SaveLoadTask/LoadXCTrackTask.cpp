

#include "Compiler.h"
#include "externs.h"
#include "Util/ScopeExit.hxx"
#include "Util/Clamp.hpp"
#include "utils/zzip_stream.h"
#include "utils/stringext.h"
#include "picojson.h"
#include <string>
#include <cassert>
#include "Waypointparser.h"

namespace {

namespace json = picojson;

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

  explicit waypoint_helper(const json::value& waypoint) : WAYPOINT{} {
      set_name(waypoint.get("name").get<std::string>().c_str());

      Longitude = waypoint.get("lon").get<double>();
      Latitude = waypoint.get("lat").get<double>();
      Altitude = waypoint.get("altSmoothed").get<double>();

      const json::value& description = waypoint.get("description");
      if (description.is<std::string>()) {
        const std::string& desc_str = description.get<std::string>();
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

  ~waypoint_helper() {
    if(Comment) {
      free(Comment);
    }
    if (Details) {
      free(Details);
    }
  }

  void set_name(const char* utf8_string) {
    from_utf8(utf8_string, Name);
  }

  void set_description(const char* utf8_string) {
    size_t len = strlen(utf8_string) + 1;
    Comment = (TCHAR*) malloc(len * sizeof(TCHAR));
    from_utf8(utf8_string, Comment, len);
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

void ParseTimeGates(const json::value &timegates) {
  if (timegates.is<json::array>()) {
    const auto& gates_array = timegates.get<json::array>();
    if (!gates_array.empty()) {
      const std::string& first_str_time = gates_array[0].get<std::string>();
      // Time of first gate in local timezone
      StrToLocalTime(first_str_time.c_str(), PGOpenTimeH, PGOpenTimeM);
      // How many gates, 1-x
      PGNumberOfGates = gates_array.size();
      if (PGNumberOfGates > 1) {
        // Interval, in minutes
        const std::string& second_str_time = gates_array[1].get<std::string>();
        int H, M;
        StrToLocalTime(second_str_time.c_str(), H, M);
        PGGateIntervalTime = (H * 60 + M) - (PGOpenTimeH * 60 + PGOpenTimeM);
      }
    }
  }
}

/**
 *  https://xctrack.org/Competition_Interfaces.html#task-definition-format
 */
bool LoadXctrackTask_V1(const json::value& task_json) {

  const json::value& turnpoints = task_json.get("turnpoints");
  if (!turnpoints.is<json::array>()) {
    return false;
  }

  auto task_it = std::begin(Task);

  for (const auto& tp : turnpoints.get<picojson::array>()) {

    waypoint_helper newPoint(tp.get("waypoint"));
    task_it->Index = FindOrAddWaypoint(&newPoint, false);

    task_it->AATType = CIRCLE;
    task_it->AATCircleRadius = tp.get("radius").get<double>();

    const json::value& type = tp.get("type"); // TAKEOFF / SSS / ESS
    if (type.is<std::string>()) {
      const std::string& type_str = type.get<std::string>();
      if (type_str == "TAKEOFF") {
        // TODO : set as HOME
      }
      else if (type_str == "SSS") {

        // warning if not the first after takeoff ( unmanaged by LK8000 )
        assert(std::distance(task_it, std::next(std::begin(Task))) <= 1);

        StartLine = 0; // CIRCLE
        StartRadius = task_it->AATCircleRadius;

        task_it = std::begin(Task); // always start task with Start of Speed Turnpoint
        task_it->AATType = CIRCLE;
        task_it->AATCircleRadius = StartRadius;
      }
      else if (type_str == "ESS") {
        task_it->AATType = 3; // ESS_CIRCLE;
      }
    }

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

  const json::value& sss = task_json.get("sss");
  if (sss.is<json::object>()) {
    const json::value& direction = sss.get("direction"); // string - one of "ENTER" / "EXIT", required
    const std::string direction_str = direction.get<std::string>();
    if (direction_str == "ENTER") {
      Task[0].OutCircle = false;
      PGStartOut = true;
    } else if (direction_str == "EXIT") {
      Task[0].OutCircle = true;
      PGStartOut = false;
    }

    const json::value& type = sss.get("type"); // string - one of "RACE" / "ELAPSED-TIME", required
    bool elapsed_time = (type.is<std::string>() && type.get<std::string>() == "ELAPSED-TIME");
    if(!elapsed_time) { // ignore timesgates in case of ELAPSED-TIME ( not managed in LK8000)
      ParseTimeGates(sss.get("timeGates")); // array of times, required
    }
  }


  if(task_it != std::begin(Task)) { // Empty Task ??
    auto goal_tp = std::prev(task_it);
    FinishRadius = goal_tp->AATCircleRadius;
    FinishLine = 0; // CIRCLE

    const json::value& goal = task_json.get("goal");
    if (goal.is<json::object>()) {
      const json::value& type = goal.get("type"); // string - one of "CYLINDER"/"LINE", optional (default CYLINDER)
      if(type.is<std::string>()) {
        if(type.get<std::string>() == "LINE") {
          FinishLine = 1; // LINE
        }
      }
    }

#if __TODO__
    const json::value& deadline = goal.get("time"); // time, optional (default 23:00 local UTC equivalent)
#endif
  }

#ifdef _WGS84
  const json::value& earthModel = task_json.get("earthModel"); // string, optional - one of "WGS84"(default) / "FAI_SPHERE"
  earth_model_wgs84 = !(earthModel.is<std::string>() && earthModel.get<std::string>() == "FAI_SPHERE");
#endif

  return true;
}

/**
 * https://xctrack.org/Competition_Interfaces.html#task-definition-format-2---for-qr-codes
 */
bool LoadXctrackTask_V2(const json::value& task_json) {
  const json::value& turnpoints = task_json.get("t");
  if (!turnpoints.is<json::array>()) {
    return false;
  }

  auto task_it = std::begin(Task);

  for (const auto& tp : turnpoints.get<picojson::array>()) {

    const std::string& name = tp.get("n").get<std::string>();

    const std::string& polyline_str = tp.get("z").get<std::string>();
    polyline_t polyline = polyline_decode(polyline_str);

    waypoint_helper newPoint(name, polyline);

    const json::value& description = tp.get("d");
    if (description.is<std::string>()) {
      newPoint.set_description(description.get<std::string>().c_str());
    }

    const json::value& type = tp.get("t");
    if (type.is<double>()) {
      switch (static_cast<int>(type.get<double>())) {
      case 2: // SSS
        // warning if not the first after takeoff ( unmanaged by LK8000 )
        assert(std::distance(task_it, std::next(std::begin(Task))) <= 1);

        task_it = std::begin(Task); // always start task with Start of Speed Turnpoint
        task_it->AATType = CIRCLE;
        StartLine = 0;
        StartRadius = polyline[3];
        break;
      case 3: // ESS
        task_it->AATType = 3; // ESS_CIRCLE;
        break;
      default:
        task_it->AATType = CIRCLE;
        break;
      }
    } else {
      task_it->AATType = CIRCLE;
    }

    task_it->Index = FindOrAddWaypoint(&newPoint, false);
    task_it->AATCircleRadius = polyline[3];

    ++task_it;
  }

  const json::value& sss = task_json.get("s");
  if (sss.is<json::object>()) {
    const json::value& direction = sss.get("d"); // number, required, one of 1 (ENTRY), 2 (EXIT)
    switch(static_cast<int>(direction.get<double>())) {
      default:
      case 1:
        Task[0].OutCircle = false;
        PGStartOut = true;
        break;
      case 2:
        Task[0].OutCircle = true;
        PGStartOut = false;
        break;
    }

    const json::value& type = sss.get("t"); // number, required, one of 1 (RACE), 2 (ELAPSED-TIME)
    bool elapsed_time = (type.is<double>() && static_cast<int>(type.get<double>()) == 2);
    if(!elapsed_time) { // ignore timesgates in case of ELAPSED-TIME ( unmanaged in )
      ParseTimeGates(sss.get("g")); // array of times, required - Time gates, start open time
    }
  }

  if(task_it != std::begin(Task)) { // Empty task ??
    auto goal_tp = std::prev(task_it);
    FinishRadius = goal_tp->AATCircleRadius;
    FinishLine = 0;

    const json::value& goal = task_json.get("g");
    if (goal.is<json::object>()) {
      
      const json::value& type = goal.get("t"); // number, optional one of 1 (LINE), 2 (CYLINDER) (default 2)
      if(type.is<double>()) {
        if (static_cast<int>(type.get<double>()) == 1) {
          FinishLine = 1; // LINE
        }
      }

#if __TODO__
      // TODO : deadline is not managed in LK8000
      const json::value& deadline = goal.get("d"); // time, optional - Deadline (default 23:00 local time UTC equivalent 
#endif
    }
  }

#ifdef _WGS84
  const json::value& earthModel = task_json.get("e"); // number, optional, 0 (wgs84, default), 1(fai sphere)
  earth_model_wgs84 = !(earthModel.is<double>() && static_cast<int>(earthModel.get<double>()) == 1);
#endif

  return true;
}

/**
 * require valid json object
 */
bool LoadXctrackTask(const json::value& task_json) {
  ClearTask();

  LockTaskData();
  AtScopeExit() {
    UnlockTaskData();
  };

  gTaskType=TSK_GP;
  TskOptimizeRoute=true;

  if (task_json.get("taskType").get<std::string>() == "CLASSIC") {
    switch(static_cast<int>(task_json.get("version").get<double>())) {
      case 1: // application/xctsk
        return LoadXctrackTask_V1(task_json);
      case 2: // from QR codes
        return LoadXctrackTask_V2(task_json);
    }
  }
  return false;
}

} // namespace

bool LoadXctrackTaskFile(const TCHAR* szFilePath) {
    StartupStore(_T(". Load XCTrack Task : <%s>"), szFilePath);

    try {
      // check file contents
      zzip_stream file(szFilePath, "rb");
      if (file) {
        std::istream stream(&file);
        json::value task_json;
        std::string error = json::parse(task_json, stream);
        if(error.empty()) {
          if(task_json.is<json::object>()) {
              return LoadXctrackTask(task_json);
          }
        }
      }
    } catch (std::exception& e) {
      StartupStore(_T(".... Invalid XCTrack task : %s"), to_tstring(e.what()).c_str());
    }
    return false;
}

bool LoadXctrackTaskString(const char* begin, const char* end) {
  try {
    json::value task_json;
    std::string error = json::parse(task_json, begin, end);
    if(error.empty()) {
      if(task_json.is<json::object>()) {
        return LoadXctrackTask(task_json);
      }
    }
  } catch (std::exception& e) {
    StartupStore(_T(".... Invalid XCTrack task : %s"), to_tstring(e.what()).c_str());
  }
  return false;
}
