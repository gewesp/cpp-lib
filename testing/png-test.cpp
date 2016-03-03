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

#include <exception>
#include <iostream>
#include <stdexcept>

#include "png.hpp"

#include "cpp-lib/interpolation.h"
#include "cpp-lib/util.h"
#include "cpp-lib/matrix-wrapper.h"


void test_image1(int const size_x, int const size_y, std::string const& name) {
  png::image< png::rgba_pixel > img(size_x, size_y);

  double const p1x = size_x * .2;
  double const p1y = size_y * .2;
  double const p2x = size_x * .8;
  double const p2y = size_y * .2;
  double const p3x = size_x * .5;
  double const p3y = size_y * .8;

  double const w = .1;

  for (int y = 0; y < size_y; ++y) {
  for (int x = 0; x < size_x; ++x) {
    double const a1 = cpl::matrix::norm_2(
        cpl::matrix::column_vector(x - p1x, y - p1y));
    double const a2 = cpl::matrix::norm_2(
        cpl::matrix::column_vector(x - p2x, y - p2y));
    double const a3 = cpl::matrix::norm_2(
        cpl::matrix::column_vector(x - p3x, y - p3y));

    double const r = std::cos(w * a1);
    double const g = std::cos(w * a2);
    double const b = std::cos(w * a3);
    double const a = r * g * b;

    img[y][x] = png::rgba_pixel((r+1) * 127, (g+1) * 127, (b+1) * 127,
        (a+1) * 127);
  } } // 2dim for

  img.write(name);
}


// int main(int const argc, const char* const* const argv) {
int main() {
  try {

  test_image1(300, 200, "test1.png");


  } catch (std::exception const& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
}
