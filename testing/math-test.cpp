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
#include "cpp-lib/math-util.h"
#include "cpp-lib/matrix-wrapper.h"

#include <iostream>


using namespace cpl::math;


void test_arc(std::ostream& os, double const k) {
  os << "arc: k = " << k << std::endl;
  // 7/8 of a circle
  for (double t = 0; t < 1.75 * 3.1415926 / k; t += .05) {
    os << cpl::matrix::transpose(cpl::math::arc(k, t)) << std::endl;
  }
}

void test_sinc(std::ostream& os) {
  double const eps2 = std::pow(std::numeric_limits<double>::epsilon(), .5);
  os << "sinc(x) - 1 around zero" << std::endl;
  for (int i = -10; i < 10; ++i) {
    double const x = i / 5.0 * eps2;
    os << x << ' ' << sinc(x) - 1 << std::endl;
  }
}

void test_cosc(std::ostream& os) {
  double const eps3 = std::pow(std::numeric_limits<double>::epsilon(), 1.0/3.0);
  os << "cosc(x) around zero" << std::endl;
  for (int i = -10; i < 10; ++i) {
    double const x = i / 5.0 * eps3;
    os << x << ' ' << cosc(x) << std::endl;
  }
}



void test_modulo(std::ostream& os, double const x, double const M) {
  os << x << " % " << M << " = " << modulo(x, M) << std::endl;
}

void test_modulo(std::ostream& os) {
  test_modulo(os, 1.5, 1);
  test_modulo(os, -.8, 1);
  test_modulo(os, 3000, 1);
  test_modulo(os, -1, 360);
  test_modulo(os, 180, 360);
  test_modulo(os, -180, 360);
  test_modulo(os, 720, 360);
  test_modulo(os, -720, 360);
  test_modulo(os, 0, 360);
  test_modulo(os, 370, 360);
  test_modulo(os, -10, 360);
}


template<typename T>
void test_avg(std::ostream& os, double& x, double const& u, T const averager) {
  os << x << " -> " << u << ": ";
  averager.update_discrete_states(x, u);
  os << x << std::endl;
}



void test_modulo_exp_avg(std::ostream& os) {
  auto const invalid = [](double const x) { return x > 1799; };

  modulo_exponential_moving_average<double, decltype(invalid)> const 
      mema(.3, 360, invalid);

  double x = 1;
  test_avg(os, x, 180, mema);
  test_avg(os, x, 180, mema);
  test_avg(os, x, 180, mema);

  x = -1;
  test_avg(os, x, 180, mema);
  test_avg(os, x, 180, mema);
  test_avg(os, x, 180, mema);


  x = 270;
  test_avg(os, x, 280, mema);
  test_avg(os, x, 280, mema);
  test_avg(os, x, 280, mema);

  test_avg(os, x, 260, mema);
  test_avg(os, x, 260, mema);
  test_avg(os, x, 260, mema);
  test_avg(os, x, 260, mema);
  test_avg(os, x, 260, mema);

  // The following three are the same
  x = 1790;
  test_avg(os, x, 260, mema);
  x = -10;
  test_avg(os, x, 260, mema);
  x = 350;
  test_avg(os, x, 260, mema);

  x = 90;
  test_avg(os, x, 260, mema);

  // Invalid initial states, should snap to 260 immediately
  x = 1810;
  test_avg(os, x, 260, mema);
}

void test_wip_1(std::ostream& os, 
    cpl::math::weighted_inner_product<2> const& dot,
    double const x1, double const x2, double const y1, double const y2) {
  std::cout << dot(cpl::matrix::column_vector(x1, x2),
                   cpl::matrix::column_vector(y1, y2)) << std::endl;
}

void test_wip(std::ostream& os) {
  std::vector<double> const w{1.0, 4.0};

  cpl::math::weighted_inner_product<2> const dot(w);

  test_wip_1(os, dot, 1, 1, 0, 0);
  test_wip_1(os, dot, 1, 1, 1, 1);

  os << "symm" << std::endl;
  test_wip_1(os, dot, 1, 2, 3, 4);
  test_wip_1(os, dot, 3, 4, 1, 2);

  os << "unit vectors" << std::endl;
  test_wip_1(os, dot, 1, 0, 1, 0);
  test_wip_1(os, dot, 0, 1, 0, 1);
}


int main() {
  test_modulo(std::cout);
  test_modulo_exp_avg(std::cout);
  test_sinc(std::cout);
  test_cosc(std::cout);
  test_arc(std::cout, 2);
  test_arc(std::cout, .5);
  test_wip(std::cout);
}
