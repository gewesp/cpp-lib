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

#include <iostream>
#include <random>

#include <cmath>

#include "cpp-lib/map.h"

#include "cpp-lib/gnss.h"


void tile_mapper_test1
(std::ostream& os, cpl::map::tile_mapper const& tm,
 cpl::gnss::lat_lon const& ll, int const zoom) {

  auto const gc = tm.get_global_coordinates(zoom, ll);
  auto const fc = tm.get_full_coordinates(zoom, ll);

  os << ll
     << ": zoom " << zoom
     << ": global = " << gc
     << "; tile = "   << fc
     << std::endl;
}

void tile_mapper_test(std::ostream& os) {

  cpl::map::tile_mapper const tm;

  cpl::gnss::lat_lon const p1{0, 0};
  cpl::gnss::lat_lon const p2{47, 8};
  cpl::gnss::lat_lon const p3{-30, -133};
  cpl::gnss::lat_lon const p4{-85.0511, -179.999999};
  cpl::gnss::lat_lon const p5{85.0511, 179.99999999};

  // lat out of range
  cpl::gnss::lat_lon const p6{89, 0};
  // lon out of range
  cpl::gnss::lat_lon const p7{0, -190};

  tile_mapper_test1(os, tm, p1, 0);
  tile_mapper_test1(os, tm, p1, 1);
  tile_mapper_test1(os, tm, p1, 3);

  tile_mapper_test1(os, tm, p2, 0);
  tile_mapper_test1(os, tm, p2, 1);
  tile_mapper_test1(os, tm, p2, 3);
  tile_mapper_test1(os, tm, p2, 10);

  tile_mapper_test1(os, tm, p3, 0);
  tile_mapper_test1(os, tm, p3, 1);
  tile_mapper_test1(os, tm, p3, 3);
  tile_mapper_test1(os, tm, p3, 10);

  tile_mapper_test1(os, tm, p4, 0);
  tile_mapper_test1(os, tm, p4, 1);
  tile_mapper_test1(os, tm, p4, 3);
  tile_mapper_test1(os, tm, p4, 10);

  tile_mapper_test1(os, tm, p5, 0);
  tile_mapper_test1(os, tm, p5, 1);
  tile_mapper_test1(os, tm, p5, 3);
  tile_mapper_test1(os, tm, p5, 10);

  tile_mapper_test1(os, tm, p6, 0);
  tile_mapper_test1(os, tm, p6, 1);
  tile_mapper_test1(os, tm, p6, 3);
  tile_mapper_test1(os, tm, p6, 10);

  tile_mapper_test1(os, tm, p7, 0);
  tile_mapper_test1(os, tm, p7, 1);
  tile_mapper_test1(os, tm, p7, 3);
  tile_mapper_test1(os, tm, p7, 10);
}

struct element {
  char foo = 123;
  short bar = 234;
};

void tileset_test(std::ostream& os) {
  // A rectangle comprising the Western Alps
  cpl::gnss::lat_lon const nw{47.8, 4.8};
  cpl::gnss::lat_lon const se{43.7, 12};

  auto const minzoom = 1;
  auto const maxzoom = 10;

  std::uniform_real_distribution<> lat(se.lat, nw.lat);
  std::uniform_real_distribution<> lon(nw.lon, se.lon);

  std::mt19937 rng;

  cpl::map::tileset_parameters tsp{nw, se, minzoom, maxzoom};
  tsp.tileset_name = "Test";
  cpl::map::tileset<element> ts{tsp};
  write_static_info(os, ts);

  for (int i = 0; i < 15; ++i) {
    for (int j = 0; j < 100; ++j) {
      for (int z = ts.minzoom(); z <= ts.maxzoom(); ++z) {
        // Access element at given coordinates
        ts.value_at_create(z, cpl::gnss::lat_lon{lat(rng), lon(rng)});
      }
    }
    write_dynamic_info(os, ts);
  }
}

int main() {
  tile_mapper_test(std::cout);
  tileset_test(std::cout);
}
