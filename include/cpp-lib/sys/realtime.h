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
// Component: REALTIME
//

#ifndef CPP_LIB_REALTIME_H
#define CPP_LIB_REALTIME_H

// This is .h, not .hpp like everything else in boost.  WTF.
#include "boost/predef.h"

// This is only supported on Linux.
#if (BOOST_OS_LINUX)
#  include "cpp-lib/linux/realtime.h"
#else
#  error "Realtime functions not supported on this operating system platform"
#endif

#endif // CPP_LIB_REALTIME_H
