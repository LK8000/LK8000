#ifndef _parse_igc_h_
#define _parse_igc_h_

#include <string>
#include <vector>
#include "Geographic/GeoPoint.h"

struct gps_fix {
  double time = 0;
  AGeoPoint position = {};
  double ground_speed = 0.;
  double track = 0.;
};

gps_fix parse_b_record(const std::string& line);

std::vector<gps_fix> parse_igc(std::istream& is);

#endif  //  _parse_igc_h_
