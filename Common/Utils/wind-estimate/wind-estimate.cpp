#include <iostream>
#include <fstream>
#include <filesystem>
#include <numeric>
#include "parse_igc.h"
#include "NMEA/Info.h"
#include "NMEA/Derived.h"
#include "windanalyser.h"

namespace fs = std::filesystem;


// Utility: convert course angle (deg) and speed to a vector
std::pair<double, double> polarToVector(double speed, double angleDeg) {
    double angleRad = angleDeg * M_PI / 180.0;
    return { speed * cos(angleRad), speed * sin(angleRad) };
}


int main(int argc, char* argv[]) {
  if (argc != 2) {
    std::cout << "usage : wind-estimator <source path>" << std::endl;
    return 1;
  }

  const fs::path igc_file = fs::path(argv[1]);

  std::ifstream is(igc_file);
  if (!is) {
    std::cerr << "Couldn't read " + igc_file.string();
  }

  const std::vector<gps_fix> data = parse_igc(is);

  NMEA_INFO Basic = {};
  DERIVED_INFO Calculated = {};

  Calculated.Circling = true;

  WindAnalyser windanalyser;

  windanalyser.slot_newFlightMode();

  bool start = false;
  for (auto& fix : data) {
    Basic.Time = fix.time;
    Basic.Latitude = fix.position.latitude;
    Basic.Longitude = fix.position.longitude;
    Basic.Speed = fix.ground_speed;
    Basic.TrackBearing = fix.track;

    if (!start && Basic.Speed > 3) {
      start = true;
    }
    if (start) {
      windanalyser.slot_newSample(&Basic, &Calculated);
    }
  }

  return 0;
}
