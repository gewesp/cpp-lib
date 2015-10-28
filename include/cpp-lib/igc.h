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
// Component: AERONAUTICS
//


#ifndef CPP_LIB_IGC_H
#define CPP_LIB_IGC_H

#include <iterator>
#include <memory>
#include <streambuf>

#include <cstdlib>
#include <cmath>
#include <cassert>

#include "cpp-lib/gnss.h"

//
// IGC file parsing.
//
// Reference: 
// FAI/IGC, TECHNICAL SPECIFICATION FOR GNSS FLIGHT RECORDERS
// http://www.ukiws.demon.co.uk/GFAC/documents/tech_spec_gnss.pdf
//

namespace cpl {

namespace igc {

// Parses a B record, returns the respective fix.  Throws
// on parse errors, including the record not being a B record.
gnss::fix parse_b_record(const std::string& line);

} // end namespace igc

} // end namespace cpl


#endif // CPP_LIB_IGC_H
