//
// Copyright 2015 KISS Technologies GmbH, Switzerland
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

//
// References
// * http://aprs.gids.nl/nmea/#gga
// * http://aprs.gids.nl/nmea/#rmc
//
// TODO: Add a table for geoid separation (undulation table)
//

#include "cpp-lib/nmea.h"

#include "cpp-lib/gnss.h"
#include "cpp-lib/util.h"

#include <string>

#include <cassert>
#include <cmath>
#include <cstdlib>


inline char EW(double const lon) {
  return lon >= 0 ? 'E' : 'W';
}

inline char NS(double const lat) {
  return lat >= 0 ? 'N' : 'S';
}


// Minutes of positive a positive degree value
inline double minutes(double const x) {
  assert(x >= 0);
  return 60.0 * std::fmod(x, 1.0);
}

#define HHMMSS "%02d%02d%02d"
// Happens to be the same...
#define YYMMDD HHMMSS
// 2 digits degrees, field width 7 and precision 4 for minutes
#define LAT "%02d%07.4f,%c"
// 3 digits degrees, field width 7 and precision 4 for minutes
#define LON "%03d%07.4f,%c"

// 1 after LON: This is a GPS fix (2 for differential GPS)
// The 0.0 between M and M is geoid separation (fixed at 0).
// The two missing values at the end are differential GPS update time
// and station ID.
char const* const FORMAT_GPGGA = 
"$GPGGA," HHMMSS "," LAT "," LON ",1,%02d,%.1f,%.1f,M,0.0,M,,";

// Missing values at end: Magnetic variation + its direction (E/W)
// The 'A' means valid fix ('V' for invalid).
char const* const FORMAT_GPRMC = 
"$GPRMC," HHMMSS ",A," LAT "," LON ",%.1f,%.1f," YYMMDD ",,";

std::string cpl::nmea::gpgga(cpl::gnss::fix const& f) {
  if (!valid(f)) {
    return "$GPGGA,,,,,,0,,,,,,,,*66";
  }

  auto const utc = cpl::util::utc_tm(f.time);

  double const abs_lat = std::fabs(f.lat);
  double const abs_lon = std::fabs(f.lon);

  char ret[100];

  std::sprintf(ret, FORMAT_GPGGA,
      utc.tm_hour, utc.tm_min, utc.tm_sec,

      static_cast<int>(abs_lat),
      minutes(abs_lat),
      NS(f.lat),

      static_cast<int>(abs_lon),
      minutes(abs_lon),
      EW(f.lon),

      f.n_satellites,
      f.horizontal_accuracy,
      f.alt);
  
  return std::string(ret) + '*' + cpl::nmea::checksum(ret);
}


std::string cpl::nmea::gprmc(
    cpl::gnss::fix const& f, 
    cpl::gnss::motion const& m) {
  if (!valid(f)) {
    return "$GPRMC,,V,,,,,,,,,,N*53";
  }

  double const abs_lat = std::fabs(f.lat);
  double const abs_lon = std::fabs(f.lon);

  auto const utc = cpl::util::utc_tm(f.time);

  char ret[120];

  std::sprintf(ret, FORMAT_GPRMC,
      utc.tm_hour, utc.tm_min, utc.tm_sec,

      static_cast<int>(abs_lat),
      minutes(abs_lat),
      NS(f.lat),

      static_cast<int>(abs_lon),
      minutes(abs_lon),
      EW(f.lon),

      m.speed / cpl::units::knot(), m.course,

      utc.tm_mday,
      utc.tm_mon + 1,
      utc.tm_year % 100);

  return std::string(ret) + '*' + cpl::nmea::checksum(ret);
}

// Ignores $ and *.
std::string cpl::nmea::checksum(char const* s) {
  unsigned char sum = 0;

  while (*s) {
    if ('$' != *s && '*' != *s) {
      sum ^= static_cast<unsigned char>(*s);
    }
    ++s;
  }

  char ret[5];
  std::sprintf(ret, "%02X", static_cast<unsigned int>(sum));
  return ret;
}
