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


#ifndef CPP_LIB_GEOMETRY_H
#define CPP_LIB_GEOMETRY_H

#include <exception>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <limits>

#include <cassert>
#include <cmath>

#include "cpp-lib/matrix-wrapper.h"

namespace cpl {

namespace math     {

//
// Transformation spherical <-> cartesian coordinates.
//
// r     ... Radius.
// theta ... Azimuthal angle in the xy-plane from the x-axis (-pi .. pi).
//           Corresponds to geographic longitude.
// phi   ... Polar angle from the z-axis (0 .. pi).  Corresponds to 
//           geographic latitude, but measured from the North Pole.
//
// Cf. http://mathworld.wolfram.com/SphericalCoordinates.html
//

cpl::matrix::vector_3_t spherical_to_cartesian( 
  double const& theta ,
  double const& phi
) ;

void cartesian_to_spherical( 
  cpl::matrix::vector_3_t const& x ,
  double& r           ,
  double& theta       ,
  double& phi
) ;


// 
// Let S be a sphere with center 0 and x on it's surface.  Return a
// local orthogonal coordinate frame in S with the following properties:
// 
// 1. Its x axis has nonnegative inner product with the z axis of the
//   reference frame.
// 2. Its y axis lies in the xy plane of the reference frame.
// 3. Its z axis points towards 0.
//

cpl::matrix::matrix_3_t
sphere_surface_frame( cpl::matrix::vector_3_t const& x ) ;

//
// Returns the signed angle between v1 and v2, returning
// a positive value if v2 can be obtained by rotating v1 to the left,
// in [rad].
//
// Returns 0 if either v1 or v2 has norm smaller than epsilon().
// 

double signed_angle( cpl::matrix::vector_2_t const& v1 ,
                     cpl::matrix::vector_2_t const& v2 ) ;


//
// Returns a point on an arc of curvature k (= 1/R, where R is the 
// curvature radius), parametrized by arc length s.  Curvature 
// may be zero.
//
// Initial direction is the X axis, positive curvature turns towards Y.
//

cpl::matrix::vector_2_t arc(double k, double s);


} // namespace math

} // namespace cpl


#endif // CPP_LIB_GEOMETRY_H
