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

#include "cpp-lib/gnss.h"

#include <iostream>
#include <numeric>

#include <cmath>

#include "cpp-lib/geodb.h"
#include "cpp-lib/matrix-wrapper.h"
#include "cpp-lib/nmea.h"

using namespace cpl::gnss;
using namespace cpl::matrix;

void operators_test_inner(lat_lon_alt const& lla, vector_3_t const& delta_ned) {
  lat_lon_alt const shifted = lla + delta_ned;
  vector_3_t  const delta_computed = relative_position(lla, shifted);

  std::cout << "Orig: " 
            << lla.lat << ' ' << lla.lon << ' ' << lla.alt << '\n';

  std::cout << "Delta (provided): " << delta_ned      << std::endl;
  std::cout << "Delta (computed): " << delta_computed << std::endl;
}

void operators_test() {
  lat_lon_alt const orig{47, 8, 1234.5};
  operators_test_inner(orig, vector_3_t(column_vector(10000.0, 0.0, 100.0)));
  operators_test_inner(orig, vector_3_t(column_vector(0.0, -10000.0, -100.0)));
  operators_test_inner(orig, vector_3_t(column_vector(12345.0, -123434.0, -4711.0)));
}

void bearing_distance_test_inner(const position_time& pt1) {
  // Go through 9 relative positions
  for (int dlat = -1; dlat <= 1; ++dlat) {
  for (int dlon = -1; dlon <= 1; ++dlon) {
    position_time pt2 = pt1;
    pt2.lat += dlat;
    pt2.lon += dlon;

    std::string dir;
    if ( 1 == dlat) { dir += "N"; pt2.alt += 100; }
    if (-1 == dlat) { dir += "S";                 }
    if ( 1 == dlon) { dir += "E";                 }
    if (-1 == dlon) { dir += "W";                 }

    std::cout << "Bearing, distance from pt1 " << pt1.lat << "/" << pt1.lon
              << " to pt2 "                    << pt2.lat << "/" << pt2.lon
              << " (" << dir << ")"
              << ": " << cpl::gnss::bearing        (pt1, pt2)
              << ", " << cpl::gnss::threed_distance(pt1, pt2)
              << "; pt2 - pt1 = " << relative_position(pt1, pt2) << std::endl
              ;
  }
  }
}

void bearing_distance_test() {
  std::cout << "Equator" << std::endl;
  bearing_distance_test_inner(position_time{0,  34, 0, 0});

  std::cout << "Central Europe" << std::endl;
  bearing_distance_test_inner(position_time{45.5,  7.8, 0, 0});

  std::cout << "West of Greenwich" << std::endl;
  bearing_distance_test_inner(position_time{0, -120, 0, 0});

  std::cout << "South America" << std::endl;
  bearing_distance_test_inner(position_time{-33, -120, 0, 0});

  std::cout << "400km up " << std::endl;
  bearing_distance_test_inner(position_time{45, 7, 400e3, 0});

  std::cout << "Very north" << std::endl;
  bearing_distance_test_inner(position_time{88.9, 30, 0, 0});
}

void v_ned_test_inner(
    double const speed, double const course, double const vs) {

  auto const vned = v_ned(speed, course, vs);
  auto const sc = cpl::gnss::to_polar_deg(vned);

  std::cout << "speed = "    << speed  << " = " << sc.first
            << "; course = " << course << " = " << sc.second
            << "; vs = "     << vs
            << "; v_ned = "  << vned << std::endl;

}

void nmea_test_1(std::ostream& os,
    cpl::gnss::fix const& f, cpl::gnss::motion const& m) {
  os << cpl::nmea::gprmc(f, m) << std::endl;
  os << cpl::nmea::gpgga(f   ) << std::endl;
}

void nmea_test(std::ostream& os) {
  using namespace cpl::gnss;
  // Feb 12, 2015 at 18:03Z
  position_time const pt1{47.5, 8.333333333333, 420   , 1423764201};
  position_time const pt2{47.5, -121, 0               , 1423764201};
  // Negative altitudes aren't really defined in NMEA, are they?
  position_time const pt3{-33.3333333333, 0, -10      , 1423764201};
  position_time const pt4{-47.38572938, 8.856382485, 0, 1423764201};

  position_time const pt_invalid;

  satinfo const si1{8, 4.7};

  // 20 knots NE
  motion const m1{10.2888889, 45, 1};
  
  nmea_test_1(os, fix{pt1, si1}, m1);
  nmea_test_1(os, fix{pt2, si1}, m1);
  nmea_test_1(os, fix{pt3, si1}, m1);
  nmea_test_1(os, fix{pt4, si1}, m1);

  nmea_test_1(os, fix{pt_invalid, si1}, m1);
}

void course_test_1(std::ostream& os, double const& c) {
  os << "normalized course for " << c 
     << ": " << cpl::math::angle_m180_180(c)
     << std::endl;
}

void course_test(std::ostream& os) {
  course_test_1(os, 0);
  course_test_1(os, 180);
  course_test_1(os, 270);
  course_test_1(os, 540);
  course_test_1(os, -3);
  course_test_1(os, -180.001);
  course_test_1(os, -360);
}

void v_ned_test() {
  v_ned_test_inner(0, 0, -10);

  // North
  v_ned_test_inner(10, 0, 1);
  // East
  v_ned_test_inner(10, 90, 1);
  // West
  v_ned_test_inner(10, 270, 1);
}

void potential_altitude_test(std::ostream& os, double const& alt,
    double const& v) {
  cpl::gnss::motion const mot1{v, 0.0, 0.0};
  cpl::gnss::motion const mot2{0.0, 123, v};
  os << "Potential altitude gain with horz. speed " << v << " m/s: "
     << cpl::gnss::potential_altitude(alt, mot1) - alt
     << std::endl;
  os << "Potential altitude gain with vert. speed " << v << " m/s: "
     << cpl::gnss::potential_altitude(alt, mot1) - alt
     << std::endl;
}

void test_kml_reading(
    std::string const& filename,
    std::string const& tag,
    std::ostream& os) {
  auto const& v = coordinates_from_kml(filename, tag);

  for (auto const& lla : v) {
    os << lla << std::endl;
  }
}

void run_queries(
    cpl::gnss::airport_db const& adb,
    std::string const& query_filename,
    std::ostream& os);

void test_airport_db(
    std::string const& filename,
    std::string const& query_filename,
    std::ostream& os) {
  auto const adb = cpl::gnss::airport_db_from_csv(filename, &os);
  run_queries(adb, query_filename, os);
}

void run_queries(
    cpl::gnss::airport_db const& adb,
    std::string const& query_filename,
    std::ostream& os) {
  std::ifstream is(query_filename.c_str());

  cpl::gnss::lat_lon_alt lla;
  while (is >> lla.lat >> lla.lon >> lla.alt) {
    auto const n = adb.nearest(lla);
    always_assert(1 == n.size());
    auto const& entry   = n[0].first;
    auto const distance = n[0].second;

    os << "query: " << lla << "; "
       << "nearest: " << entry.icao << '/' << entry.type
       << "; dist: " << distance
       << std::endl;
  }
}


void test_airport_db_openaip(
    std::string const& filename,
    std::string const& query_filename,
    std::ostream& os) {
  cpl::gnss::airport_db adb;
  cpl::gnss::airport_db_from_openaip(adb, filename, true, &os);
  run_queries(adb, query_filename, os);
}

void test_airport_db_registry(
    std::string const& filename,
    std::ostream& os) {
  cpl::util::registry const reg(filename);
  cpl::gnss::airport_db adb;
  cpl::gnss::airport_db_from_registry(adb, reg, &os);
}

void test_geoid_1(std::ostream& os, cpl::gnss::lat_lon const& ll) {
  os << "geoid @ " << ll << ": " 
     << cpl::gnss::geoid_height(ll)
     << std::endl;
}

void test_geoid(
    std::ostream& os,
    std::string const& filename,
    int const skip) {
  cpl::gnss::geoid_init(&os, filename, skip);

  test_geoid_1(os, cpl::gnss::lat_lon{47, 8});
  test_geoid_1(os, cpl::gnss::lat_lon{52, -3});
  test_geoid_1(os, cpl::gnss::lat_lon{-30, 0});
  test_geoid_1(os, cpl::gnss::lat_lon{-35, -122});
  test_geoid_1(os, cpl::gnss::lat_lon{-34,  142});

  test_geoid_1(os, cpl::gnss::lat_lon{90, 8});
  test_geoid_1(os, cpl::gnss::lat_lon{100, 8});

  test_geoid_1(os, cpl::gnss::lat_lon{52, -3 });
  test_geoid_1(os, cpl::gnss::lat_lon{52, 357});

  test_geoid_1(os, cpl::gnss::lat_lon{52, 360});
  test_geoid_1(os, cpl::gnss::lat_lon{52, 370});
}

int main(int const argc, char const* const* const argv) {
  try {
  cpl::util::verify(2 <= argc, "give at least 2 arguments");
  std::string const command = argv[1];
  if ("kml" == command) {
    cpl::util::verify(4 == argc, "kml <filename> <tag>");
    test_kml_reading(argv[2], argv[3], std::cout);
  } else if ("airport_db_registry" == command) {
    cpl::util::verify(3 == argc, "airport_db_registry <config>");
    test_airport_db_registry(argv[2], std::cout);
  } else if ("airport_db" == command) {
    cpl::util::verify(4 == argc, "airport_db <dbfile> <queries>");
    test_airport_db(argv[2], argv[3], std::cout);
  } else if ("airport_db_openaip" == command) {
    cpl::util::verify(4 == argc, "airport_db_openaip <dbfile> <queries>");
    test_airport_db_openaip(argv[2], argv[3], std::cout);
  } else if ("geoid" == command) {
    cpl::util::verify(3 == argc, "geoid <dbfile>");
    test_geoid(std::cout, argv[2], 8);
  } else if ("unittest" == command) {
    cpl::util::verify(2 == argc, "unittests (no further arguments)");
    operators_test();
    bearing_distance_test();
    v_ned_test();
    course_test(std::cout);
    nmea_test(std::cout);

    potential_altitude_test(std::cout, 1000, 25);
    potential_altitude_test(std::cout, -100, 50);
    potential_altitude_test(std::cout, -100, 79);
  } else {
    throw std::runtime_error("usage: gnss-test kml | airport_db | unittests");
  }
  } catch (std::exception const& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  }
}
