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

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>

#include <iostream>
#include <random>


typedef boost::geometry::model::point<
    double, 2, boost::geometry::cs::cartesian> point;
typedef std::pair<point, int> value;
typedef boost::geometry::index::quadratic<4> strategy;
typedef boost::geometry::index::rtree<value, strategy> tree;

value element(int const i) {
  std::minstd_rand mstd(i);
  std::uniform_int_distribution<int> I(0, 999999);

  return std::make_pair(point{i, i + 1}, I(mstd));
}

void crash_regression(std::ostream& os, tree& tr, int n) {
  os << "Inserting/removing " << n << " elements" << std::endl;
  for (int i = 0; i < n; ++i) {
    tr.insert(element(i));
  }
  assert(static_cast<unsigned long>(n) == tr.size());
  for (int i = 0; i < n; ++i) {
    tr.remove(element(i));
  }
  assert(0 == tr.size());
}

int main() {
  tree tr;
  crash_regression(std::cout, tr, 3000);
  crash_regression(std::cout, tr, 1);
}
