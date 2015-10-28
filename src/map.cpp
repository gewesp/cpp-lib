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

#include "cpp-lib/map.h"

#include "cpp-lib/gnss.h"
#include "cpp-lib/util.h"
#include "cpp-lib/units.h"

#include <cmath>


cpl::map::tile_mapper::tile_mapper(int const tilesize)
  : tilesize_{tilesize} {
  cpl::util::verify(
          256 == tilesize
      ||  512 == tilesize
      || 1024 == tilesize
      || 2048 == tilesize,
      "tile size must be 256, 512, 1024 or 2048");
}


cpl::map::global_coordinates cpl::map::tile_mapper::get_global_coordinates(
    int const zoom,
    cpl::gnss::lat_lon const& ll) const {
  assert(zoom       >= 0);
  assert(tilesize() >= 256);
  global_coordinates ret;

  // Out of range for the 85.051129 value; poles are *not* covered
  if (ll.lat > 86 || ll.lat < -86) {
    return ret;
  }
  if (ll.lon > 180 || ll.lon < -180) {
    return ret;
  }

  double const lat_rad = ll.lat * cpl::units::degree();
  double const lon_rad = ll.lon * cpl::units::degree();

  double const x = 0.5 * tilesize() / M_PI * (1L << zoom) 
                 * (lon_rad + M_PI);
  double const y = 0.5 * tilesize() / M_PI * (1L << zoom) 
                 * (M_PI - std::log(std::tan(M_PI / 4 + lat_rad / 2)));

  // Oops
  if (x < 0 || y < 0) {
    return ret;
  }
  if (x >= (1L << zoom) * tilesize() || y >= (1L << zoom) * tilesize()) {
    return ret;
  }

  ret.x = x;
  ret.y = y;
  return ret;
}

cpl::map::tile_coordinates
cpl::map::tile_mapper::get_tile_coordinates(
    cpl::map::global_coordinates const& gc) const {
  cpl::map::tile_coordinates ret;

  ret.x = gc.x / tilesize();
  ret.y = gc.y / tilesize();

  assert(ret.x >= 0);
  assert(ret.y >= 0);
  return ret;
}

cpl::map::pixel_coordinates
cpl::map::tile_mapper::get_pixel_coordinates(
    cpl::map::global_coordinates const& gc,
    cpl::map::tile_coordinates const& tc) const {

  cpl::map::pixel_coordinates ret;

  ret.x = gc.x - static_cast<double>(tilesize()) * tc.x;
  ret.y = gc.y - static_cast<double>(tilesize()) * tc.y;
  cpl::math::clamp(ret.x, 0, tilesize() - 1);
  cpl::math::clamp(ret.y, 0, tilesize() - 1);

  return ret;
}


std::ostream& cpl::map::operator<<
(std::ostream& os, cpl::map::global_coordinates const& gc) {
  return os << gc.x << ' ' << gc.y;
}

std::ostream& cpl::map::operator<<
(std::ostream& os, cpl::map::tile_coordinates const& tc) {
  return os << tc.x << ' ' << tc.y;
}

std::ostream& cpl::map::operator<<
(std::ostream& os, cpl::map::pixel_coordinates const& pc) {
  return os << pc.x << ' ' << pc.y;
}

std::ostream& cpl::map::operator<<
(std::ostream& os, cpl::map::full_coordinates const& fc) {
  return os << fc.tile << ' ' << fc.pixel;
}

cpl::map::tileset_parameters 
cpl::map::tileset_parameters_from_registry(
    cpl::util::registry const& reg,
    cpl::map::tileset_parameters const& defaults) {

  cpl::map::tileset_parameters ret;

  ret.minzoom = 
      reg.get_default("minzoom", static_cast<long>(defaults.minzoom));
  ret.maxzoom = 
      reg.get_default("maxzoom", static_cast<long>(defaults.maxzoom));

  ret.tilesize = 
      reg.get_default("tilesize", static_cast<long>(defaults.tilesize));

  ret.north_west = 
      lat_lon_from_registry(reg, "north_west", defaults.north_west);

  ret.south_east = 
      lat_lon_from_registry(reg, "south_east", defaults.south_east);

  ret.tileset_name = reg.get_default("tileset_name", defaults.tileset_name);

  ret.empty_tile_name=
      reg.get_default("empty_tile_name", defaults.empty_tile_name);

  // Mandatory!
  ret.tile_directory = reg.check_string("tile_directory");

  ret.validate();

  return ret;
}
