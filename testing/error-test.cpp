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

#include "cpp-lib/assert.h"
#include "cpp-lib/error.h"

// int main(int argc, const char* const* const argv) {
int main() {
  always_assert(4 == 2 + 2);

  std::cout << "The next assertion should faile and call die():" << std::endl;
  always_assert(5 == 2 + 2);

  /*NOTREACHED*/
  return 0;
}
