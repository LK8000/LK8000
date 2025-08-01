#include "parse_igc.h"
#include <stdexcept>
#include <charconv>
#include <istream>

namespace {

void throw_igc_error(std::string const& msg) {
  throw std::runtime_error("IGC parser: " + msg);
}

int int_cast(const std::string::const_iterator begin, const std::string::const_iterator end) {
  try {
    int value;
    if (std::from_chars(begin.base(), end.base(), value).ec == std::errc()) {
      return value;
    }
  }
  catch (std::exception&) {
  }

  throw_igc_error("Couldn't convert to integer: " + std::string(begin, end));
  return 0; // Silence compiler warning
}

inline double deg_minthousand2deg(int deg, int minutes_1000) {
  return deg + minutes_1000 / 60000.;
}

} // namespace

gps_fix parse_b_record(const std::string& line) {
  gps_fix ret;

  if( !line.size() ) {
    throw_igc_error( "Empty line" ) ;
  }

  // only interested in fix records
  if( line[ 0 ] != 'B' ) {
    throw_igc_error( "Not a B record" );
  }

  if( line.size() < 35 ) {
    throw std::runtime_error( "B record too short" ) ;
  }

  if( line[ 24 ] != 'A' ) {
    return ret;
  }

  const std::string::const_iterator beg = line.begin();
  const int hh = int_cast(beg + 1, beg + 3);
  const int mm = int_cast(beg + 3, beg + 5);
  const int ss = int_cast(beg + 5, beg + 7);

  const int deg_lat = int_cast(beg + 7 , beg + 9 );
  const int min_lat = int_cast(beg + 9 , beg + 14);
  const int deg_lon = int_cast(beg + 15, beg + 18);
  const int min_lon = int_cast(beg + 18, beg + 23);

  const int alt     = int_cast(beg + 30, beg + 35);

  ret.position.latitude = deg_minthousand2deg(deg_lat, min_lat);
  ret.position.longitude = deg_minthousand2deg(deg_lon, min_lon);

  if ('S' == line[14]) {
    ret.position.latitude = -ret.position.latitude;
  }
  else if ('N' == line[14]) {
  }
  else {
    throw_igc_error("Invalid North/South specifier");
  }

  if ('W' == line[23]) {
    ret.position.longitude = -ret.position.longitude;
  }
  else if ('E' == line[23]) {
  }
  else {
    throw_igc_error("Invalid East/West specifier");
  }

  ret.time                = 3600.0 * hh + 60.0 * mm + ss;
  ret.position.altitude   = alt;

  return ret;
}

std::vector<gps_fix> parse_igc(std::istream& is) {
  std::vector<gps_fix> data;
  std::string line;
  while (std::getline(is, line)) {
    try {
      if (0 == line.size()) {
        continue; // empty line!
      }
      if ('B' == line[0]) {
        auto fix = parse_b_record(line);
        if(!data.empty()) {
          auto& prev = data.back();
          prev.position.Reverse(fix.position, fix.track, fix.ground_speed);
          fix.ground_speed /= (fix.time - prev.time);
        }
        data.push_back(std::move(fix));
      }
    }
    catch (std::exception&) {
        // ignore parsing error
    }
  }
  return data;
}
