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
// Component: GNSS
//


#ifndef CPP_LIB_GNSS_H
#define CPP_LIB_GNSS_H

#include <iostream>

#include "cpp-lib/bg-typedefs.h"
#include "cpp-lib/math-util.h"
#include "cpp-lib/matrix-wrapper.h"
#include "cpp-lib/registry.h"
#include "cpp-lib/spatial-index.h"
#include "cpp-lib/units.h"


namespace cpl {

namespace gnss {

struct satinfo {
  satinfo()
    : n_satellites(0), horizontal_accuracy(0) {}

  satinfo(int const n_satellites, double const horizontal_accuracy)
    : n_satellites(n_satellites), horizontal_accuracy(horizontal_accuracy) {}

  // Number of satellites used to compute the fix, -1 for an invalid fix
  int n_satellites;

  // Horizontal accuracy [m].  The smaller, the better.
  double horizontal_accuracy;
};

struct lat_lon {
  lat_lon() : lat(0), lon(0) {}

  lat_lon(
      double const lat,
      double const lon)
    : lat(lat), lon(lon) {}

  // Geographic latitude [deg], North = positive
  double lat;

  // Geographic longitude [deg], East = positive
  double lon;
};

struct lat_lon_bounding_box {
  lat_lon north_west;
  lat_lon south_east;
};

// Returns true iff ll lies inside bb.
// TODO: Date line...
bool inside(lat_lon const& ll, lat_lon_bounding_box const& bb);

struct motion {
  motion() : speed(0), course(0), vertical_speed(0) {}

  motion(double const speed,
      double const course,
      double const vertical_speed)
    : speed(speed), course(course), vertical_speed(vertical_speed) {}

  // Horizontal speed [m/s]
  double speed;

  // Course over ground [degree]
  double course;

  // Vertical speed [m/s], positive up
  double vertical_speed;
};

struct motion_and_turnrate : motion {
  motion_and_turnrate() :
    motion{}, turnrate{0}
  {}

  motion_and_turnrate(
      double const& speed,
      double const& course,
      double const& vertical_speed,
      double const& turnrate) :
    motion(speed, course, vertical_speed),
    turnrate(turnrate) 
  {}

  // Turn rate [deg/s], right = positive
  double turnrate;
};

struct lat_lon_alt : lat_lon {
  lat_lon_alt()
    : lat_lon{} , alt{0}
  {}

  lat_lon_alt(lat_lon const& ll)
    : lat_lon{ll}, alt{0}
  {}

  lat_lon_alt(
      double const lat,
      double const lon,
      double const alt)
    : lat_lon{lat, lon},
      alt{alt}
  {}

  // Altitude [m] over reference sphere, ellipsoid (e.g., WGS84) or geoid
  double alt;
};

struct position_time : lat_lon_alt {
  position_time() :
    lat_lon_alt() ,
    time(0) {}

  position_time(
      double const lat,
      double const lon,
      double const alt,
      double const time) :
    lat_lon_alt(lat, lon, alt) ,
    time(time) {}

  position_time(
      lat_lon_alt const& lla,
      double const time) :
    lat_lon_alt(lla) , 
    time(time) {}

  // Time [s], reference to be determined by application
  double time;
};

// Position/time and satellite info, but no motion
struct fix : position_time, satinfo {
  fix() : position_time(), satinfo() {}

  fix(position_time const& pt, satinfo const& si)
    : position_time(pt), satinfo(si) {}
};

// Validates lat/lon pair: [-90, 90] x [-180, 180]
// Throws on errors.
void validate_lat_lon(lat_lon const& ll);
void validate_lat_lon_bounding_box(lat_lon_bounding_box const&);

inline bool valid(position_time const& pt) {
  return pt.time > 0;
}

// Gets from registry, returns def if not defined
lat_lon lat_lon_from_registry(
    cpl::util::registry const&, std::string const& key,
    lat_lon const& def = lat_lon{});


// Write data, format: n_satellites horizontal_accuracy
std::ostream& operator<<(std::ostream&, satinfo const&);

// Write data, format: time lat lon alt
std::ostream& operator<<(std::ostream&, position_time const&);

// Write data, format: lat lon
std::ostream& operator<<(std::ostream&, lat_lon const&);

// Write data, format: lat lon alt
std::ostream& operator<<(std::ostream&, lat_lon_alt const&);

// Write data, format: north_west: lat lon; south_east: lat lon
std::ostream& operator<<(std::ostream&, lat_lon_bounding_box const&);

// Write data, format: position_time satinfo
std::ostream& operator<<(std::ostream&, fix const&);


// Read data, format: time lat lon alt
std::istream& operator>>(std::istream&, position_time&);

// Returns 3D distance [m] between pt1 and pt2 with a circular approximation 
// of the Earth, disregarding time.
double threed_distance(position_time const& pt1, position_time const& pt2);

// Same as above, but ignore altitude, so this effectively makes
// it rather a 2D distance.
double twod_pseudo_distance(position_time const& pt1, position_time const& pt2);

// Returns bearing [degree] from pt1 to pt2, disregarding time and altitude.
// E.g., returns 0 if pt2 is north of pt1, 90 if pt2 is East of pt1, etc.
// Uses NED frame of pt1 as reference.
double bearing(position_time const& pt1, position_time const& pt2);

// Converts the given lat/lon/alt to ECEF (Earth-centered, Earth-fixed)
// coordinate frames.
cpl::matrix::vector_3_t lla_to_ecef
(lat_lon_alt const&, double const& radius);

// Inverse of the above
lat_lon_alt ecef_to_lla
(cpl::matrix::vector_3_t const&, double const& radius);

// Approximately 'add' the given delta to lat_lon_alt.
// Down component: down *positive*!!
lat_lon_alt operator+(
    lat_lon_alt const& lla, cpl::matrix::vector_3_t const& delta_ned);

inline lat_lon_alt const& operator+=(
    lat_lon_alt& lla, cpl::matrix::vector_3_t const& delta_ned) {
  return lla = lla + delta_ned;
}

// If |now - pt.time| <= max_predict_time, adds dt * v_ned to pt, updates
// time and return result.  Otherwise, returns pt unmodified.
position_time predict(
    position_time const& pt, cpl::matrix::vector_3_t const& v_ned,
    double const& now, double const& max_predict_time);

// Returns the relative position of pt2 w.r.t. a NED system around pt1.  
// Coordinates are positive iff pt2 is North/East/Down of pt1.
// The N/E distances are computed as if on the Earth's surface.
cpl::matrix::vector_3_t relative_position(
    lat_lon_alt const& pt1, 
    lat_lon_alt const& pt2);

#if 0
// TODO: Operator overload considered dangerous here since
// altitude part is negated...
// Computes relative position such that pt1 + (pt2 - pt1) ~= pt2
inline cpl::matrix::vector_3_t operator-(
    lat_lon_alt const& pt1, 
    lat_lon_alt const& pt2) {
  return relative_position(pt2, pt1);
}
#endif

// Returns North/East/Down speed for given speed [m/s], course [degrees]
// and vertical speed.  Notice that vertical speed is up/positive,
// but in NED the sign is reversed!
cpl::matrix::vector_3_t v_ned(
    double speed,
    double course_deg,
    double vertical_speed);

inline cpl::matrix::vector_3_t v_ned(motion const& m) {
  return v_ned(m.speed, m.course, m.vertical_speed);
}

// Returns the sum of altitude and the potential altitude gain
// in case m were be upwards directed, i.e.
// altitude + v^2/2g where v is the vector sum of horizontal and
// vertical speed.
double potential_altitude(double const& altitude, motion const& m);


// Returns distance and angle [degrees] for the given
// North/East displacement vector.  Rows with index >= 3 are ignored.
// Can be used to obtain speed and course for a North/East/Down velocity 
// vector.
template< typename M >
std::pair<double, double> to_polar_deg(
    M const& dnorth_deast,
    double const eps = std::numeric_limits<double>::epsilon()) {

  assert(cpl::matrix::n_rows   (dnorth_deast) >= 2);
  assert(cpl::matrix::n_columns(dnorth_deast) == 1);

  double const dnorth = dnorth_deast(0);
  double const deast  = dnorth_deast(1);
  double const d      = std::sqrt(
      cpl::math::square(dnorth) + cpl::math::square(deast));

  if (d < eps) {
    return std::make_pair(d, 0.0);
  } else {
    return   std::make_pair(d,
                   cpl::math::angle_0_2pi(std::atan2(deast, dnorth)) 
                 / cpl::units::degree());
  }

}

inline motion to_motion(cpl::matrix::vector_3_t const& v_ned) {
  // speed/course
  auto const sc = cpl::gnss::to_polar_deg(v_ned);
  return motion(sc.first, sc.second, -v_ned(2));
}

// Reads KML from the given file and returns the coordinates found in
// tag.
std::vector<lat_lon_alt> coordinates_from_kml(
    std::string const& filename,
    std::string const& tag);

// Reads lon,lat,alt pairs (CAUTION: lon and lat swapped as customary 
// in KML) and returns coordinates.
std::vector<lat_lon_alt> coordinates_from_lon_lat_alt(
    std::istream& iss);

////////////////////////////////////////////////////////////////////////
// A simple geoid model
////////////////////////////////////////////////////////////////////////

//
// Initializes the (static) geoid data from given file.
// Currently supported: WW15MGH.GRD
// Data download: 
// http://earth-info.nga.mil/GandG/wgs84/gravitymod/egm96/egm96.html
// Throws on errors.
// Logs to sl if not NULL.
// Skips to 1, 2, 4 or 8 data points (8 data points: 2 degree resolution,
// enough for aviation applications)
//

void geoid_init(std::ostream* sl, std::string const& filename, int skip = 1);

//
// Gets geoid height [m] (i.e., geoid height above WGS84 ellipsoid) at
// given point.
// Wraps negative longitude values to positive and limits them to 360.
// No interpolation is done.
// Conversion:
//
//   AMSL(ll) = WGS84(ll) - geoid_height(ll)
//
// NOTE: If geoid_init() hasn't been called or failed, this
// always returns zero.
//

double geoid_height(lat_lon const&);

// Converts WGS84 altitude [m] to Mean Sea Level (MSL).
inline double msl_from_wgs84(lat_lon_alt const& lla) {
  return lla.alt - geoid_height(lla);
}

// Converts Mean Sea Level (MSL) altitude [m] to WGS84.
inline double wgs84_from_msl(lat_lon_alt const& lla) {
  return lla.alt + geoid_height(lla);
}

inline long memory_consumption(const lat_lon_alt& lla) {
  return sizeof(lla);
}

inline long memory_consumption(const position_time& pt) {
  return sizeof(pt);
}

} // namespace gnss

namespace math {

//
// Traits to use spatial_index with cpl::gnss::lat_lon
//

template<> struct spatial_index_traits<cpl::gnss::lat_lon> {
  static cpl::math::point point_from_value(
      cpl::gnss::lat_lon const& ll) {
    return point(ll.lat, ll.lon);
  }
};


//
// Returns a square box with width and height 2*d [m]. LL must have 
// lat and lon members.
//

template<typename LL>
cpl::math::box query_box(LL const& center, double const d) {
  cpl::gnss::lat_lon_alt const c{center};
  cpl::matrix::vector_3_t const delta = cpl::matrix::column_vector(d, d, 0.0);

  return cpl::math::box{
    spatial_index_traits<cpl::gnss::lat_lon>::point_from_value(
        c + (-delta)),
    spatial_index_traits<cpl::gnss::lat_lon>::point_from_value(
        c + delta)};
}


} // namespace math

} // namespace cpl


#endif // CPP_LIB_GNSS_H
