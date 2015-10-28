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
// Component: GNSS
//
// References
// [1] http://aprs.gids.nl/nmea/#gga
//     http://aprs.gids.nl/nmea/#rmc
// [2] http://edu-observatory.org/gps/gps_accuracy.html
//
// Examples from:
// * http://nmea.sourceforge.net/
//
// TODO: We currently equate HDOP with the accuracy value.  Check what
// receivers make from that value, they might just as well make the
// same mistake.  See [2].
//


#ifndef CPP_LIB_NMEA_H
#define CPP_LIB_NMEA_H

#include "cpp-lib/gnss.h"

#include <string>

namespace cpl {

namespace nmea {


//
// Returns a $GPGGA NMEA sentence from the given fix.
//

std::string gpgga(cpl::gnss::fix const&);


//
// Returns a $GPRMC NMEA sentence from the given fix and motion.
//

std::string gprmc(cpl::gnss::fix const&, cpl::gnss::motion const&);


//
// Returns the NMEA checksum of the argument, e.g. "*A6"
//

std::string checksum(char const*);


} // namespace nmea

} // namespace cpl


#endif // CPP_LIB_NMEA_H
