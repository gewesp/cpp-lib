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


#ifndef CPP_LIB_GEODB_H
#define CPP_LIB_GEODB_H

#include <iostream>
#include <string>

#include "cpp-lib/bg-typedefs.h"
#include "cpp-lib/gnss.h"
#include "cpp-lib/math-util.h"
#include "cpp-lib/matrix-wrapper.h"
#include "cpp-lib/registry.h"
#include "cpp-lib/units.h"

#include <boost/geometry.hpp>
#include <boost/geometry/index/rtree.hpp>


namespace cpl {

namespace gnss {

//
// A lat/lon based query structure for nearest-point queries.
//
// Internally, it uses a 3D ECEF (Earth-centered, Earth fixed coordinate
// system) based tree.
//
// The first template argument is the type for values, lat/lon is
// considered the key.
//
template <typename T,
         typename STRAT = boost::geometry::index::quadratic<4> >
struct geodb {

  typedef STRAT strategy;

  typedef T value_type;
  typedef std::pair<value_type, double> value_and_distance;
  typedef std::vector<value_and_distance> value_and_distance_vector;

  typedef std::pair<cpl::math::point_3_t, value_type> tree_element;

  // The tree type used for the spatial index
  typedef boost::geometry::index::rtree<tree_element, strategy> tree_type;

  // Creates a database with a given name and radius (default to Earth).
  // The DB name should reflect the contents or purpose of the DB.
  geodb(std::string const& name = "(unnamed)",
        double const& R = cpl::units::earth_radius())
  : dbname_(name),
    radius_(R)
  {}

  // Returns planet radius [m]
  double radius() const { return radius_; }

  // Returns database name
  double name  () const { return dbname_; }

  // Returns number of stored DB elements
  long size() const { return tr.size(); }

  // Adds a the given element
  // CAUTION: Being ECEF-based, the DB *does* take altitude into account.
  // If this is not required or wanted, just set all altitudes to zero
  // (including queries).
  void add_element(cpl::gnss::lat_lon_alt const&, value_type const&);

  // Finds max_results nearest element(s) to lla and returns the associated 
  // value(s) together with the respective 3D distance.
  value_and_distance_vector nearest(
      cpl::gnss::lat_lon_alt const& lla, 
      int max_results = 1) const;

private:
  std::string dbname_;
  double radius_;

  tree_type tr;
};

// Example value type: Airport data
unsigned constexpr AIRPORT_TYPE_SMALL = 0x01;
unsigned constexpr AIRPORT_TYPE_LARGE = 0x02;
unsigned constexpr AIRPORT_TYPE_HELI  = 0x08;

struct airport_data {
  // Airport name (UTF-8)
  std::string name;

  // ICAO ID (e.g., LSZH)
  std::string icao;

  // Exactly one of AIRPORT_TYPE_...
  unsigned type;

  // TODO: Runway direction(s)
};

typedef geodb<airport_data> airport_db;

// Read an airport DB from a CSV file.
// Format:
// icao,type,latitude_deg,longitude_deg,elevation_ft
// E.g.
// LSZH,large_airport,47.4646987915,8.5491695404,1416
// Logs if log != NULL.
airport_db airport_db_from_csv(
    std::string const& filename,
    std::ostream* log = NULL);

// Reads an airport DB from an openAIP file, adding to adb.
// capitalize: Capitalize names (ICAO codes are always
// capitalized)
void airport_db_from_openaip(
    airport_db& adb,
    std::string const& filename,
    bool capitalize = true,
    std::ostream* log = NULL);

// Parses multiple openAIP XML files given base directory and
// list of country codes.
void airport_db_from_openaip(
    airport_db& adb,
    std::string const& directory,
    std::vector<std::string> const& country_list,
    bool capitalize = true,
    std::ostream* log = NULL);

// Parses registry to read multiple .aip files:
// airport_db_directory = <directory>
// airport_db_country_list = {de, gb, it, ...}
// Calls airport_db_from_openaip()
void airport_db_from_registry(
    airport_db& adb,
    cpl::util::registry const& reg,
    std::ostream* log = NULL);


} // namespace gnss

} // namespace cpl

// Template definitions

template <typename T, typename STRAT>
void cpl::gnss::geodb<T, STRAT>::add_element(
    cpl::gnss::lat_lon_alt const& lla,
    value_type const& v) {

  auto const x = cpl::gnss::lla_to_ecef(lla, radius());
  auto const p = cpl::math::from_vector(x);

  tr.insert(std::make_pair(p, v));
}

template <typename T, typename STRAT>
typename cpl::gnss::geodb<T, STRAT>::value_and_distance_vector
cpl::gnss::geodb<T, STRAT>::nearest(
    cpl::gnss::lat_lon_alt const& lla,
    int const max_results) const {

  value_and_distance_vector ret;

  auto const x = cpl::gnss::lla_to_ecef(lla, radius());
  auto const p = cpl::math::from_vector(x);

  auto it = boost::geometry::index::qbegin(
              tr, boost::geometry::index::nearest(p, max_results));
  while (it != boost::geometry::index::qend(tr)) {
    double const dist = boost::geometry::distance(p, it->first);
    ret.push_back(std::make_pair(it->second, dist));
    ++it;
  }
  return ret;
}


#endif // CPP_LIB_GEODB_H
