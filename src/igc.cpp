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
// Component: AERONAUTICS
//

#include "cpp-lib/igc.h"
#include "cpp-lib/gnss.h"

#include <cstdio>

#include <boost/lexical_cast.hpp>

namespace {

void throw_igc_error(std::string const& msg) {
  throw std::runtime_error("IGC parser: " + msg);
}

int int_cast(const std::string::const_iterator begin, 
             const std::string::const_iterator end) {
  try {
    return boost::lexical_cast<int>(std::string(begin, end));
  } catch (std::exception&) {}
  throw_igc_error("Couldn't convert to integer: " + 
      std::string(begin, end));
  // Silence compiler warning---throw_igc_error always throws of course...
  return 0;
}

inline double deg_minthousand2deg(const int deg, const int minutes_1000) {
  return deg + minutes_1000 / 60000.;
}

} // end anonymous namespace


cpl::gnss::fix cpl::igc::parse_b_record(std::string const& line) {

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

  cpl::gnss::fix ret;

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
  const int acc     = int_cast(beg + 35, beg + 38);
  const int num_sat = int_cast(beg + 38, beg + 40);

  ret.lat = deg_minthousand2deg(deg_lat, min_lat);
  ret.lon = deg_minthousand2deg(deg_lon, min_lon);

  if ('S' == line[14]) {
    ret.lat = -ret.lat;
  } else if ('N' == line[14]) {
  } else {
    throw_igc_error("Invalid North/South specifier");
  }

  if ('W' == line[23]) {
    ret.lon = -ret.lon;
  } else if ('E' == line[23]) {
  } else {
    throw_igc_error("Invalid East/West specifier");
  }

  ret.time                = 3600.0 * hh + 60.0 * mm + ss;
  ret.alt                 = alt;
  ret.n_satellites        = num_sat;
  ret.horizontal_accuracy = acc;

  return ret;
}
