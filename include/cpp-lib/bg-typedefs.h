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
// Component: MATH
//

#ifndef CPL_BG_TYPEDEFS_H
#define CPL_BG_TYPEDEFS_H

#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

#include "cpp-lib/matrix-wrapper.h"

namespace cpl {

namespace math {

// Point type, used for indexing
typedef boost::geometry::model::point<
    double, 2, boost::geometry::cs::cartesian> point;

typedef boost::geometry::model::point<
    double, 2, boost::geometry::cs::cartesian> point_2_t;

typedef boost::geometry::model::point<
    double, 3, boost::geometry::cs::cartesian> point_3_t;

// http://www.boost.org/doc/libs/1_58_0/boost/geometry/geometries/segment.hpp
// Derives from std::pair<point, point>
typedef boost::geometry::model::segment<point> segment;

// Box type, used for querying.  Constructed with two points:
// box b(point(xl, yl), point(xh, yh))
typedef boost::geometry::model::box<point> box;


// Simple conversion stuff

inline point_3_t from_vector(cpl::matrix::vector_3_t const& x) {
  point_3_t ret;
  ret.set<0>(x(0));
  ret.set<1>(x(1));
  ret.set<2>(x(2));
  return ret;
}

} // namespace math

} // namespace cpl

#endif // CPL_BG_TYPEDEFS_H
