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

#include "cpp-lib/geometry.h"
#include "cpp-lib/gnss.h"
#include "cpp-lib/matrix-wrapper.h"
#include "cpp-lib/units.h"
#include "cpp-lib/util.h"
#include "cpp-lib/sys/syslogger.h"

#include "boost/lexical_cast.hpp"
#include "boost/property_tree/ptree.hpp"
#include "boost/property_tree/xml_parser.hpp"

#include <cmath>

using namespace cpl::matrix;

cpl::gnss::lat_lon
cpl::gnss::lat_lon_from_registry(
    cpl::util::registry const& reg,
    std::string const& key,
    cpl::gnss::lat_lon const& def) {
  if (!reg.is_defined(key)) {
    return def;
  } else {
    auto const& v = reg.check_vector_double(key, 2);
    try {
      cpl::gnss::lat_lon ret{v[0], v[1]};
      validate_lat_lon(ret);
      return ret;
    } catch (std::exception const& e) {
      throw std::runtime_error(reg.defined_at(key) + ":" + e.what());
    }
  }
}

// TODO: Precision, sentry, save state etc.
std::ostream& cpl::gnss::operator<<(
    std::ostream& os, cpl::gnss::lat_lon_alt const& lla) {
  os.precision(10);
  os << lla.lat  << " "
     << lla.lon  << " "
     << lla.alt
  ;
  return os;
}

std::ostream& cpl::gnss::operator<<(
    std::ostream& os, cpl::gnss::position_time const& pt) {
  os.precision(10);
  os << pt.time << " "
     << pt.lat  << " "
     << pt.lon  << " "
     << pt.alt
  ;
  return os;
}

std::ostream& cpl::gnss::operator<<(
    std::ostream& os, cpl::gnss::lat_lon const& ll) {
  os.precision(10);
  os <<        ll.lat 
     << ' ' << ll.lon
  ;
  return os;
}

std::istream& cpl::gnss::operator>>(
    std::istream& is, cpl::gnss::position_time& pt) {
  is >> pt.time
     >> pt.lat
     >> pt.lon
     >> pt.alt
  ;
  return is;
}

std::ostream& cpl::gnss::operator<<(
    std::ostream& os, cpl::gnss::satinfo const& si) {
  os << si.n_satellites << " "
     << si.horizontal_accuracy 
  ;
  return os;
}

std::ostream& cpl::gnss::operator<<(
    std::ostream& os, cpl::gnss::fix const& fix) {
  os << static_cast<cpl::gnss::position_time>(fix) << ' '
     << static_cast<cpl::gnss::satinfo>      (fix)
  ;
  return os;
}

double cpl::gnss::threed_distance(
    cpl::gnss::position_time const& pt1,
    cpl::gnss::position_time const& pt2) {

  using cpl::math::pi;

  using cpl::matrix::vector_3_t;

  using cpl::units::degree;
  using cpl::units::earth_radius;

  // Convert lat/lon into azimuth (theta)/polar (phi)
  const double theta1 = pt1.lon * degree();
  const double theta2 = pt2.lon * degree();

  const double phi1 = pi / 2 - pt1.lat * degree();
  const double phi2 = pi / 2 - pt2.lat * degree();

  // Add altitude to earth radius.
  const double r1 = earth_radius() + pt1.alt;
  const double r2 = earth_radius() + pt2.alt;

  // Convert to ECEF system.
  // Do *not* use auto here, due to expression templates!
  const vector_3_t p1 = r1 * cpl::math::spherical_to_cartesian(theta1, phi1);
  const vector_3_t p2 = r2 * cpl::math::spherical_to_cartesian(theta2, phi2);
  const vector_3_t d = p2 - p1;

  return cpl::matrix::norm_2(d);
}

double cpl::gnss::twod_pseudo_distance(
    cpl::gnss::position_time const& pt1,
    cpl::gnss::position_time const& pt2) {

  cpl::gnss::position_time pt1_0 = pt1;
  cpl::gnss::position_time pt2_0 = pt2;

  pt1_0.alt = 0;
  pt2_0.alt = 0;

  return cpl::gnss::threed_distance(pt1_0, pt2_0);
}

double cpl::gnss::bearing(
    cpl::gnss::position_time const& pt1,
    cpl::gnss::position_time const& pt2) {

  using cpl::units::degree;

  const double dlon = (pt2.lon - pt1.lon) * degree();
  const double dlat = (pt2.lat - pt1.lat) * degree();

  const double coslat = std::cos(pt1.lat * degree());

  // North and South pole: Undefined.
  if (std::abs(coslat) < std::numeric_limits<double>::epsilon()) {
    return 0;
  }

  // Approximate vector from pt1 to pt2
  const auto d = 
    cpl::matrix::column_vector(dlat, dlon * coslat);

  // If distance in radians is too small, return zero.
  // TODO: The limit is somewhat arbitrary... 
  // TODO: Could use to_polar_deg().
  // TODO: Could remove use of sqrt().
  if (cpl::matrix::norm_2(d) < std::numeric_limits<double>::epsilon()) {
    return 0;
  } else {
    return cpl::math::angle_0_2pi(std::atan2(dlon * coslat, dlat)) / degree();
  }
}

vector_3_t cpl::gnss::lla_to_ecef
(cpl::gnss::lat_lon_alt const& lla, double const& radius) {
  double const r = radius + lla.alt;
  double const theta = lla.lon * cpl::units::degree();
  double const phi   = cpl::math::pi / 2.0 - lla.lat * cpl::units::degree();
  return r * cpl::math::spherical_to_cartesian(theta, phi);
}

// Inverse of the above
cpl::gnss::lat_lon_alt cpl::gnss::ecef_to_lla
(vector_3_t const& p, double const& radius) {
  double r;
  double theta, phi;
  cpl::math::cartesian_to_spherical(p, r, theta, phi);
  double const lat_radians = cpl::math::pi / 2.0 - phi;
  double const lon_radians = theta;
  return cpl::gnss::lat_lon_alt{
      lat_radians / cpl::units::degree(),
      lon_radians / cpl::units::degree(),
      r - radius};
}


void cpl::gnss::validate_lat_lon(lat_lon const& ll) {
  if (ll.lat > 90 || ll.lat < -90) {
    throw std::runtime_error(
          "invalid latitude: " 
        + boost::lexical_cast<std::string>(ll.lat));
  }

  if (ll.lon > 180 || ll.lon < -180) {
    throw std::runtime_error(
          "invalid longitude: " 
        + boost::lexical_cast<std::string>(ll.lon));
  }
}

cpl::gnss::lat_lon_alt cpl::gnss::operator+(
    cpl::gnss::lat_lon_alt const& lla,
    cpl::matrix::vector_3_t const& delta_ned) {

  using cpl::units::degree;
  using cpl::units::earth_radius;

  const double coslat = std::cos(lla.lat * degree());
  return cpl::gnss::lat_lon_alt(
      lla.lat + (delta_ned(0) / earth_radius()) / degree(),
      lla.lon + (delta_ned(1) / earth_radius()) / degree() / coslat,
      lla.alt -  delta_ned(2)
  );
}

cpl::gnss::position_time cpl::gnss::predict(
    cpl::gnss::position_time const& pt, 
    cpl::matrix::vector_3_t const& delta_ned,
    double const& now, double const& max_predict_time) {
  double const dt = now - pt.time;
  if (std::fabs(dt) > max_predict_time) {
    return pt;
  }

  cpl::gnss::position_time ret;
  static_cast<cpl::gnss::lat_lon_alt&>(ret) = pt + dt * delta_ned;
  ret.time = pt.time + dt;

  return ret;
}

// TODO: Profiling ktrax suggests cos() uses the major part of
// CPU time.  Can be factored out and passed as a function argument.
// Google research (1/2015) suggests cos() takes about 50 to
// 100 times more than elementary FP operations.
cpl::matrix::vector_3_t cpl::gnss::relative_position(
    cpl::gnss::lat_lon_alt const& pt1,
    cpl::gnss::lat_lon_alt const& pt2) {

  using cpl::units::degree;
  using cpl::units::earth_radius;

  const double dlon = (pt2.lon - pt1.lon) * degree();
  const double dlat = (pt2.lat - pt1.lat) * degree();

  const double coslat = std::cos(pt1.lat * degree());

  // North and South pole: Undefined.
  if (std::abs(coslat) < std::numeric_limits<double>::epsilon()) {
    return cpl::matrix::column_vector(0.0, 0.0, 
        pt1.alt - pt2.alt);
  }

  // Approximate vector from pt1 to pt2
  return cpl::matrix::column_vector(
    earth_radius() * dlat,
    earth_radius() * dlon * coslat,
    pt1.alt - pt2.alt);
}

cpl::matrix::vector_3_t cpl::gnss::v_ned(
    double const speed,
    double const course_deg,
    double const vertical_speed) {
  double const course = course_deg * cpl::units::degree();

  return cpl::matrix::column_vector(
      speed * std::cos(course),
      speed * std::sin(course),
      -vertical_speed);
}

double cpl::gnss::potential_altitude(
    double const& alt,
    cpl::gnss::motion const& mot) {
  double const v2 = cpl::math::square(mot.speed)
                  + cpl::math::square(mot.vertical_speed);
  return alt + v2 / (2 * cpl::units::gravitation());
}

std::vector<cpl::gnss::lat_lon_alt> cpl::gnss::coordinates_from_kml(
    std::string const& filename,
    std::string const& tag) {
  boost::property_tree::ptree pt;
  boost::property_tree::read_xml(filename, pt);

  std::istringstream iss{pt.get<std::string>(tag)};
  std::vector<cpl::gnss::lat_lon_alt> ret;
  char c1, c2;
  double lat, lon, alt;

  // Attention: KML has lon,lat,alt
  while (iss >> lon >> c1 >> lat >> c2 >> alt) {
    if (c1 != ',' || c2 != ',') {
      throw 
        std::runtime_error("failure reading KML coordinates from " + filename);
    }
    ret.push_back(cpl::gnss::lat_lon_alt{lat, lon, alt});
  }
  return ret;
}

////////////////////////////////////////////////////////////////////////
// Geoid stuff
////////////////////////////////////////////////////////////////////////
namespace {

double constexpr grid = 0.25;

// rows: latitude
int constexpr maxrows = 4 * 180;
// columns: longitude
int constexpr maxcols = 4 * 360;

struct geoid {
  // 1 / (Number of data points per degree)
  double scale = grid;

  // The data
  cpl::matrix::matrix_f_t data;

  geoid(std::ostream* sl, std::string const&, int);

  geoid() {}

  double height(cpl::gnss::lat_lon const&) const;
};

geoid the_geoid;

} // anonymous namespace

::geoid::geoid(
    std::ostream* const sl, std::string const& filename, int const skip) {
  if (sl) {
    *sl << cpl::util::log::prio::NOTICE
        << "Geoid initialization: Reading data from " << filename
        << std::endl;
  }

  cpl::util::verify(1 == skip || 2 == skip || 4 == skip || 8 == skip,
      "Geoid initialization: Skip must be 1, 2, 4 or 8");
  scale = 1.0 / (skip * grid);

  auto is = cpl::util::file::open_read(filename);

  double lat_min, lat_max;
  double lon_min, lon_max;
  double grid_lat, grid_lon;

  is >> lat_min >> lat_max
     >> lon_min >> lon_max
     >> grid_lat >> grid_lon;

  if (  -90.0 != lat_min ||  90.0 != lat_max 
      ||  0.0 != lon_min || 360.0 != lon_max) {
    throw std::runtime_error(
        "Geoid initialization: Wrong lat/lon limits");
  }

  if (grid != grid_lon || grid != grid_lat) {
    throw std::runtime_error(
        "Geoid initialization: Wrong grid increments");
  }

  int const rows = maxrows / skip + 1;
  int const cols = maxcols / skip + 1;
  cpl::matrix::matrix_f_t new_geoid(rows, cols);

  for (int r = 0; r < maxrows + 1; ++r) {
  for (int c = 0; c < maxcols + 1; ++c) {
    float val;
    is >> val;
    if (!is) {
      throw std::runtime_error("Geoid initialization: Read error");
    }
    if (0 == (r % skip) && 0 == (c % skip)) {
      int const rr = r / skip;
      int const cc = c / skip;
      assert(rr < n_rows   (new_geoid));
      assert(cc < n_columns(new_geoid));
      new_geoid(rr, cc) = val;
    }
  } }
  if (!is) {
    throw std::runtime_error("Geoid initialization: Read error");
  }
  double dummy;
  is >> dummy;
  if (is) {
    throw std::runtime_error("Geoid initialization: Extra data at end of file");
  }

  data = new_geoid;
}

double ::geoid::height(cpl::gnss::lat_lon const& lla) const {
  if (0 == n_rows(data)) {
    return 0.0;
  }

  double lat = lla.lat;
  double lon = lla.lon;

  if (lon < 0.0) { lon += 360.0; }

  cpl::math::clamp(lat, -90.0,  90.0);
  cpl::math::clamp(lon,   0.0, 360.0);

  int const row = static_cast<int>((90.0 - lat) * scale + 0.5);
  int const col = static_cast<int>((lon       ) * scale + 0.5);

  assert(0 <= row);
  assert(0 <= col);
  assert(row < n_rows   (data));
  assert(col < n_columns(data));

  return data(row, col);
}

void cpl::gnss::geoid_init(
    std::ostream* const sl, std::string const& filename, int const skip) {
  the_geoid = ::geoid(sl, filename, skip);
}

double cpl::gnss::geoid_height(cpl::gnss::lat_lon const& lla) {
  return the_geoid.height(lla);
}
