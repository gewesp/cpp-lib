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

#include "cpp-lib/ogn.h"

#include <iostream>
#include <iterator>
#include <string>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <algorithm>
#include <memory>

#include <cassert>

#include "cpp-lib/sys/network.h"
#include "cpp-lib/sys/syslogger.h"
#include "cpp-lib/sys/util.h"

#include "cpp-lib/assert.h"
#include "cpp-lib/http.h"
#include "cpp-lib/math-util.h"
#include "cpp-lib/registry.h"
#include "cpp-lib/units.h"
#include "cpp-lib/util.h"

#include <boost/lexical_cast.hpp>

using namespace cpl::util::log;

namespace {

// Maximum plausible altitude for small aircraft
double constexpr MAX_PLAUSIBLE_ALTITUDE = 20000;

// If utc >= 0, merges the date part from utc and the time part of timeofday.
double adapt_utc(double timeofday, double utc) {
  if (utc < 0) {
    return timeofday;
  }

  // Number of seconds/day
  using cpl::units::day;

  const double date = std::floor(utc / day()) * day();
  const double candidate = date + timeofday;

  // Test whether timeofday is likely to be on date, date - 1day or date + 1day.
  const double delta[3] = { 
    std::fabs(candidate - day() - utc),
    std::fabs(candidate         - utc),
    std::fabs(candidate + day() - utc) 
  };

  auto const mindelta = std::min_element(&delta[0], &delta[0] + 3);

         if (&delta[1] == mindelta) {
    // std::cout << "# timedelta = " << candidate         - utc << std::endl ;
    return candidate        ;
  } else if (&delta[0] == mindelta) {
    // std::cout << "# timedelta = " << candidate - day() - utc << std::endl ;
    return candidate - day();
  } else {
    // std::cout << "# timedelta = " << candidate + day() - utc << std::endl ;
    return candidate + day();
  }
}


// TODO: Error handling for UTC times with leap seconds (ss may be 60)?
long hhmmss_to_seconds(const long hhmmss) {
  if (hhmmss < 0) {
    throw std::runtime_error("negative HHMMSS time");
  }
  const long ss =  hhmmss        % 100;
  const long mm = (hhmmss / 100) % 100;
  const long hh =  hhmmss / 10000     ;

  if (ss >= 60 || mm >= 60 || hh >= 24) {
    throw std::runtime_error("invalid HHMMSS time");
  }

  return ss + 60 * mm + 3600 * hh;
}

bool my_double_cast(const char* const s, double& x) {
  try {
    x = boost::lexical_cast<double>(std::string(s));
    return true;
  } catch (const std::exception& e) {
    // std::cerr << e.what() << "; val = " << s << std::endl;
    return false;
  }
}

// Convert DDDDDmm.mmmm format to decimal degrees.  E.g. 45 degrees 40 minutes,
// 30 seconds would be represented as 4540.5 and the function would return
// 45.675.
double ddmm2deg(const double x) {
  assert(x >= 0);
  const double minutes = std::fmod(x, 100);
  const double degrees_100 = x - minutes;
  // assert(degrees_100 >= 0); // may fail numerically?
  return 1e-2 * degrees_100 + minutes / 60;
}

void set_latlon(
  const char* const NS, const char* const EW,
  double& lat, double& lon) {

  lat = ddmm2deg(lat);
  lon = ddmm2deg(lon);

  if ('S' == NS[0]) { lat = -lat; }
  if ('W' == EW[0]) { lon = -lon; }
}

// Support for higher precision minute values.  Stock APRS has minutes
// with two decimal places, amounting to 18.5m precision @ equator,
// cf. ddmm2deg().  This extension allows up to a factor of 91 higher
// precision: http://www.aprs.org/datum.txt
// Notice that the spec seems to contain an error.  The character
// '{' (ASCII 123) is the last one in base-91 and corresponds to 90, whereas
// datum.txt erroneously lists '}'.

// The respective 'dao' parsing code in libfap (fapint_parse_dao()):
#if 0
        else if ( 'a' <= input[0] && input[0] <= 'z' &&
                  0x21 <= input[1] && input[1] <= 0x7b &&
                  0x21 <= input[2] && input[2] <= 0x7b )
        {
                // Base-91.
                packet->dao_datum_byte = toupper(input[0]); //
                if ( packet->pos_resolution == NULL )
                {
                        packet->pos_resolution = malloc(sizeof(double));
                        if ( !packet->pos_resolution ) return 0;
                }
                *packet->pos_resolution = fapint_get_pos_resolution(4);
                // Scale base-91.
                lat_off = (input[1]-33.0)/91.0 * 0.01 / 60.0;
                lon_off = (input[2]-33.0)/91.0 * 0.01 / 60.0;
#endif

// Is this a base-91 character?
// In ASCII, this is: '!' ... '{'
inline bool isbase91(char const c) {
  return 33 <= c && c < 33 + 91;
}

bool set_latlon_dao(char const* const dao,
    double& lat,
    double& lon) {

  if (    5  != std::strlen(dao)
      || '!' != dao[0]
      || '!' != dao[4]) {
    return false;
  }

  assert(5 == std::strlen(dao));
  assert('!' == dao[0]);
  assert('!' == dao[4]);

  double dlat, dlon;

  if ('W' == dao[1]) {
    if (!(std::isdigit(dao[2]) && std::isdigit(dao[3]))) {
      return false;
    }
    dlat = (dao[2] - '0') * 1e-3 / cpl::units::minute();
    dlon = (dao[3] - '0') * 1e-3 / cpl::units::minute();
  } else if ('w' == dao[1]) {
    if (!(isbase91(dao[2]) && isbase91(dao[3]))) {
      return false;
    }
    dlat = (dao[2] - 33) * 1e-2 / 91 / cpl::units::minute();
    dlon = (dao[3] - 33) * 1e-2 / 91 / cpl::units::minute();
  } else {
    return false;
  }

  lat += (lat >= 0 ? dlat : -dlat);
  lon += (lon >= 0 ? dlon : -dlon);
  return true;
}

} // end anonymous namespace


std::unique_ptr<cpl::util::network::connection> cpl::ogn::connect(
    std::ostream& log,
    const std::string& host,
    const std::string& service) {

  log << prio::NOTICE << "Connecting to " << host << ":" << service << std::endl;
  std::unique_ptr<cpl::util::network::connection> ret(
      new cpl::util::network::connection(host, service));

  log << prio::NOTICE << "Local address: " << ret->local() << std::endl ;
  log << prio::NOTICE << "Peer address: "  << ret->peer () << std::endl ;

  return std::move(ret);
}

void cpl::ogn::login(
    std::ostream& log,
    std::ostream& os,
    std::istream& is,
    const std::string& version,
    const std::string& filter,
    const std::string& username) {

  std::string login_string = "user " + username + " pass -1 vers " + version;
  if ("" != filter) {
    login_string += " filter " + filter;
  }

  log << prio::NOTICE << "OGN login string: " << login_string << std::endl;
  os << login_string << std::endl;

  if (!os) {
    log << prio::ERR << "OGN login: connection died" << std::endl;
    throw std::runtime_error("Login failure");
  }

  std::string reply;
  for (int i = 0; i < 2; ++i) {
    cpl::util::getline(is, reply, 200);
    log << prio::NOTICE << "Login result: " << reply << std::endl;
  }

  if (std::string::npos == reply.find("server")) {
    log << prio::ERR << "OGN login: denied" << std::endl;
    throw std::runtime_error("Login failure");
  } else {
    log << prio::NOTICE << "OGN login: OK" << std::endl;
  }

}

cpl::ogn::aprs_parser::~aprs_parser() {
  if (query_thread_active) {
    cpl::util::log::syslogger log;
    // TODO: Proper synchronization
    query_thread_active = false;
    log << cpl::util::log::prio::NOTICE
        << "OGN: Waiting for DDB query thread to finish..."
        << std::endl;
    query_thread.join();
  }
}

void cpl::ogn::aprs_parser::set_vdb(cpl::ogn::vehicle_db&& new_vdb) {
  if (new_vdb.size() > 0) {
    std::lock_guard<std::mutex> vdb_lock(vdb_mutex);
    vdb = std::move(new_vdb);
    has_nontrivial_vdb = true;
  }
}

void cpl::ogn::aprs_parser::query_thread_function() {
  cpl::util::log::syslogger log;
  log << cpl::util::log::prio::NOTICE
      << "OGN: DDB query thread started, interval: "
      << query_interval
      << " seconds" << std::endl;

  while (query_thread_active) {
    set_vdb(get_vehicle_database_ddb(log));

    if (!query_thread_active) { return; }
    cpl::util::sleep(query_interval);
  }
}

cpl::ogn::aprs_parser::aprs_parser(
    std::ostream& log,
    double const query_interval,
    std::string const& initial_vdb)
: query_interval(query_interval),
  query_thread_active(query_interval > 0),
  has_nontrivial_vdb(false) {
  log << prio::NOTICE
      << "OGN: APRS parser instantiated "
      << (query_thread_active ? "with" : "without")
      << " background DDB querying"
      << std::endl;

  if ("" != initial_vdb) {
    log << prio::NOTICE << "OGN: Reading DDB from "
        << initial_vdb
        << std::endl;
    set_vdb(cpl::ogn::get_vehicle_database_ddb(log, initial_vdb));
  }

  if (query_thread_active) {
    query_thread = std::thread{&aprs_parser::query_thread_function, this};
  }
}

bool cpl::ogn::parse_aprs_station(
    const std::string& line, 
    cpl::ogn::station_info_and_name& stat,
    const double utc) {

  char station_v [41] = "";
  char lon_v[30] = "";
  double alt_ft = 0;

  char network_v [41] = "";

  long hhmmss = 0;

  char NS[2] = "";
  char EW[2] = "";

  // Normal, special conversions
  int constexpr n_normal  = 8;
  int constexpr n_special = 5;
  char special[n_special][31];

  const char* const format = 
      "%40[^>]" // station name
      ">APRS,TCPIP*,qAC,"
      "%40[^:]" // network? (seen: GLIDERN1, GLIDERN2)
      ":/"
      "%ld"     // HHMMSSh
      "h"       // zulu time
      "%lf"     // latitude
      "%1[NS]"  // north/south
      "I"       // It's there, why?
      "%20[0-9.]"  // lon as string
      "%1[EW]"     // east/west
      "%*[^A]"  // cse/spd, not parsed for stations that are normally, well,
                // stationary
      "A=%lf "  // altitude [ft]
      "%30s "   // Specials: CPU, RAM, vx.y.z, NTP, each up to 20 characters
      "%30s "
      "%30s "
      "%30s "
      "%30s"
      // TODO: etc: RF:+40+2.7ppm/+0.8dB
  ;

  const int conversions = std::sscanf(
      line.c_str(), format, 
      station_v,
      network_v,
      &hhmmss,
      &stat.second.pt.lat,
      NS,
      lon_v,
      EW,
      &alt_ft,
      special[0],
      special[1],
      special[2],
      special[3],
      special[4]);

  if (n_normal + 4 <= conversions) {
    if (!my_double_cast(lon_v, stat.second.pt.lon)) {
      return false;
    }

    stat.first = station_v;
    stat.second.network = network_v;
    stat.second.pt.time = adapt_utc(hhmmss_to_seconds(hhmmss), utc);

    set_latlon(NS, EW, stat.second.pt.lat, stat.second.pt.lon);
    stat.second.pt.alt = alt_ft * cpl::units::foot();

    // Assert that conversions <= n_normal + n_special
    // hence i < conversions - n_normal <= n_special
    if (!(conversions <= n_normal + n_special)) {
      return false;
    }

    for (int i = 0; i < conversions - n_normal; ++i) {
      switch (special[i][0]) {
        case 'C': 
          if (1 != std::sscanf(special[i], "CPU:%lf", 
                               &stat.second.cpu)) {
            return false;
          }
          break;

        case 'v':
          stat.second.version = special[i];
          break;

        case 'R':
          if ('A' == special[i][1]) {
            if (2 != std::sscanf(special[i], "RAM:%lf/%lfMB",
                                 &stat.second.ram_used,
                                 &stat.second.ram_max)) {
              return false;
            }
          } else if ('F' == special[i][1]) {
            // TODO
          } else {
            return false;
          }
          break;

        case 'N':
          if (2 != std::sscanf(special[i], "NTP:%lfms/%lfppm",
                               &stat.second.ntp_difference,
                               &stat.second.ntp_ppm)) {
            return false;
          }
          break;

        default:
          if (1 != std::sscanf(special[i], "%lfC",
                               &stat.second.temperature)) {
            return false;
          }
          break;
      } // switch
    } // for special[]...

    if (stat.second.version.empty()) {
      stat.second.version = "v0.0.0";
    }

    return true;
  } else {
    return false;
  }
}

std::string cpl::ogn::qualified_id(std::string const& id, short id_type) {
  switch (id_type) {
    case cpl::ogn::ID_TYPE_RANDOM: return "random:"  + id;
    case cpl::ogn::ID_TYPE_FLARM : return "flarm:"   + id;
    case cpl::ogn::ID_TYPE_ICAO  : return "icao:"    + id;
    case cpl::ogn::ID_TYPE_OGN   : return "ogn:"     + id;
    default:                       return "unknown:" + id;
  }
}

std::string cpl::ogn::unqualified_id(std::string const& id) {
  auto const colon = id.find(':');
  if (std::string::npos == colon) {
    return id;
  } else {
    return id.substr(colon + 1, std::string::npos);
  }
}

std::ostream& cpl::ogn::operator<<(
    std::ostream& os, cpl::ogn::station_info const& stat) {
  os <<        stat.version
     << " " << stat.network
     << " " << stat.pt
     << " " << stat.cpu
     << " " << stat.ram_used
     << " " << stat.ram_max
     << " " << stat.ntp_difference
     << " " << stat.ntp_ppm
     << " " << stat.temperature
  ;
  return os;
}

std::ostream& cpl::ogn::operator<<(
    std::ostream& os, cpl::ogn::aircraft_rx_info const& acft) {
  os <<        acft.id_type
     << " " << acft.vehicle_type
     << " " << acft.stealth
     // << " " << acft.data.process // always 1
     << " " << acft.data.tracking
     << " " << acft.data.identify
     << " " << acft.data.name1
     << " " << acft.ver.hardware
     << " " << acft.ver.software
     << " " << static_cast<cpl::gnss::position_time const&>(acft.pta)
     << " " << acft.pta.horizontal_accuracy
     << " " << acft.mot.course
     << " " << acft.mot.speed
     << " " << acft.mot.vertical_speed
     << " " << acft.mot.turnrate
     << " " << acft.baro_alt
     << " " << acft.rx
  ;
  return os;
}

std::ostream& cpl::ogn::operator<<(
    std::ostream& os, cpl::ogn::thermal const& th) {
  os << "THERMAL "
     << th.pt
     << ' ' << th.climbrate
  ;
  return os;
}

char const* cpl::ogn::thermal_format_comment() {
  return "# THERMAL time lat lon alt climbrate";
}

std::ostream& cpl::ogn::operator<<(
    std::ostream& os, cpl::ogn::rx_info const& rx) {
  os <<        rx.received_by
     << " " << rx.is_relayed
     << " " << rx.rssi
     << " " << rx.frequency_deviation
     << " " << rx.errors
  ;
  return os;
}
 
// Parses APRS lines containing OGN targets.
// If a vehicle_db is installed, fills in data from there.
// See http://wiki.glidernet.org/wiki:subscribe-to-ogn-data
bool cpl::ogn::aprs_parser::parse_aprs_aircraft(
    const std::string& line, 
    cpl::ogn::aircraft_rx_info_and_name& acft,
    double const utc) {
  unsigned id_and_type = 0;
  char id_v[8] = "";

  long hhmmss = 0;

  double climb_rate_fpm = 0, turn_rate_rot = 0;
  double alt_ft = 0;
  double baro_alt_fl = 0;

  char cse_spd[11] = "";

  char callsign_v[41] = "";
  char station_v [41] = "";
  char lon_v[21] = "";
  char NS[2] = "";
  char EW[2] = "";
  char relay_v[9] = "";

  // Normal, special conversions
  int constexpr n_normal  = 10;
  int constexpr n_special = 11;
  char special[n_special][31];

  // qAR / qAS: See 'q Construct', http://www.aprs-is.net/q.aspx
  // It's either qAS,<relay> or qAR for directly received packets
  const char* const format = 
      "%40[^>]"
      ">APRS%8[RELAY*,]qAS,"
      "%40[^:]"
      ":/%ldh"
      "%lf"
      "%1[NS]"  // north/south
      "%*[/\\]" // separator, may be slash or backslash (!)
      "%20[0-9.]"
      "%1[EW]"  // east/west
      "%*c"      // z, ', ... (movement indicator?)
      "%10[^A]"  // course/speed, either "CCC/SSS/" or just "/" if not moving
      "A=%lf "   // altitude [ft]
      "%30s "
      "%30s "
      "%30s "
      "%30s "
      "%30s "
      "%30s "
      "%30s "
      "%30s "
      "%30s "
      "%30s "
      "%30s"
  ;

  const int conversions = std::sscanf(
      line.c_str(), format, 
      callsign_v,
      relay_v,
      station_v,
      &hhmmss,
      &acft.second.pta.lat,
      NS,
      lon_v,
      EW,
      cse_spd,
      &alt_ft,
      special[0],
      special[1],
      special[2],
      special[3],
      special[4],
      special[5],
      special[6],
      special[7],
      special[8],
      special[9],
      special[10]);

  // Want at least 6 'special' conversions.
  // gpsNxM not there for OGN trackers
  int const special_converted = conversions - n_normal;

  // Relayed packets don't have kHz, dB and error count, so only
  // 3 'specials'.  Others should have at least 6.
  acft.second.rx.is_relayed = ',' == relay_v[0] && 'R' == relay_v[1];
  int const min_special_converted = acft.second.rx.is_relayed ? 4 : 6;
  if (special_converted < min_special_converted) {
    return false;
  }
  {
    // "sub-parser" for cse/speed
    int course = 0, speed_kt = 0;

    if ('/' == cse_spd[0] ||
        (   '/' == cse_spd[3]
         && 8   == std::strlen(cse_spd)
         && 2   == std::sscanf(cse_spd, "%d/%d/", &course, &speed_kt))) {
      acft.second.mot.course = course;
      // TODO: Is this really in knots?
      acft.second.mot.speed  = speed_kt * cpl::units::knot();
    } else {
      return false;
    }
  }

  acft.second.rx.received_by = station_v ;
  // As of 2015, no callsign is transmitted on the APRS network.
  // acft.second.data.name1 = callsign_v;
  acft.second.data.name1 = "-";
  acft.second.pta.time = adapt_utc(hhmmss_to_seconds(hhmmss), utc);

  if (!my_double_cast(lon_v, acft.second.pta.lon)) {
    return false;
  }

  set_latlon(NS, EW, acft.second.pta.lat, acft.second.pta.lon);
  acft.second.pta.alt = alt_ft * cpl::units::foot();

  if (acft.second.pta.alt > MAX_PLAUSIBLE_ALTITUDE) {
    return false;
  }

  int shift = 0;

  if ('!' == special[0][0]) {
    if (!set_latlon_dao(special[0], acft.second.pta.lat, acft.second.pta.lon)) {
      return false;
    }
    ++shift;
  }

  if (shift >= special_converted) { goto postprocess; }
  assert(shift < special_converted);
  if (2 != std::sscanf(special[shift], "id%2x%7s", &id_and_type, id_v)) {
    return false;
  }
  if (6 != std::strlen(id_v)) {
    return false;
  }
  ++shift;

  if (shift >= special_converted) { goto postprocess; }
  assert(shift < special_converted);
  if (1 != std::sscanf(special[shift], "%lffpm", &climb_rate_fpm)) {
    return false;
  }
  ++shift;

  if (shift >= special_converted) { goto postprocess; }
  assert(shift < special_converted);
  if (1 != std::sscanf(special[shift], "%lfrot", &turn_rate_rot)) {
    return false;
  }
  ++shift;

  // Value may be missing on FLARMs
  if (shift >= special_converted) { goto postprocess; }
  assert(shift < special_converted);
  if (1 == std::sscanf(special[shift], "FL%lf", &baro_alt_fl)) {
    ++shift;
    acft.second.baro_alt = baro_alt_fl * cpl::units::flight_level();
  }

  if (shift >= special_converted) { goto postprocess; }
  assert(shift < special_converted);
  if (1 != std::sscanf(special[shift], "%lfdB", &acft.second.rx.rssi)) {
    return false;
  } else {
    ++shift;
    if (acft.second.rx.is_relayed) { return false; }
  }

  // 0e, 1e, 2e etc. (errors)
  if (shift >= special_converted) { goto postprocess; }
  assert(shift < special_converted);
  if (1 != std::sscanf(special[shift], "%hde", &acft.second.rx.errors)) {
    return false;
  } else {
    ++shift;
    if (acft.second.rx.is_relayed) { return false; }
  }

  if (shift >= special_converted) { goto postprocess; }
  assert(shift < special_converted);
  if (1 != std::sscanf(special[shift], "%lfkHz", 
                       &acft.second.rx.frequency_deviation)) {
    return false;
  } else {
    ++shift;
    if (acft.second.rx.is_relayed) { return false; }
  }

  if (shift >= special_converted) { goto postprocess; }
  assert(shift < special_converted);
  if (1 == std::sscanf(special[shift], "gps%lfx%*d", 
                       &acft.second.pta.horizontal_accuracy)) {
    // horizontal/vertical accuracy (we only have horizontal,
    // they correlate closely)
    // GPS accuracy may be missing for OGN trackers
    ++shift;
  }

  if (shift >= special_converted) { goto postprocess; }
  assert(shift < special_converted);
  if ('s' == special[shift][0]) {
    acft.second.ver.software = special[shift] + 1;
    ++shift;
  }

  if (shift >= special_converted) { goto postprocess; }
  assert(shift < special_converted);
  // Parse h6.xx, but not hearXXXX (old versions had that)
  if ('h' == special[shift][0] && 'e' != special[shift][1]) {
    acft.second.ver.hardware = special[shift] + 1;
    ++shift;
  }


  // Post processing of values (units etc.)
postprocess:
  // STttttaa
  // stealth mode S, no-tracking flag T, aircraft type tttt, address type aa
  acft.second.id_type      =   id_and_type       & 0x3  ;
  acft.second.vehicle_type =  (id_and_type >> 2) & 0xf  ;
  acft.second.stealth =   id_and_type       & 0x80 ;
  acft.second.process = !(id_and_type       & 0x40);
  // acft.second.data.track and acft.second.data.identify set in
  // by caller.

  // Primary key ID: 'first' element of the pair.
  acft.first = qualified_id(id_v, acft.second.id_type);

  acft.second.mot.vertical_speed =
    climb_rate_fpm * cpl::units::foot() / cpl::units::minute();

  // http://wiki.glidernet.org/wiki:subscribe-to-ogn-data
  // OGN doc is unclear here:
  // "1rot is the standard aircraft rotation rate of 1 half-turn per two
  // minutes.".  We assume 1rot indicates a standard procedure turn rate
  // which is 3 degrees/second.
  // Pawel June 22, 2015: Standard turn 180deg/min == 3deg/sec.
  acft.second.mot.turnrate = 3 * turn_rate_rot;

  if (has_nontrivial_vdb) {
    std::lock_guard<std::mutex> lock(vdb_mutex);
    auto const it = vdb.find(unqualified_id(acft.first));
    if (vdb.end() != it) {
      acft.second.data = it->second;
    }
  }

  return true;
}

// DDB functions

bool parse_bool(std::string const& s, std::string const& loc) {
  cpl::util::verify("Y" == s || "N" == s,
                    loc + "invalid flag (must be 'Y' or 'N')");
  return "Y" == s;
}

cpl::ogn::vehicle_data_and_name parse_ddb_entry(cpl::util::lexer& lex) {
  cpl::util::expect(lex, cpl::util::STRING);
  auto const id_type_string = lex.string_value();
  cpl::util::verify(1 == id_type_string.size(), 
                    lex.location() + "invalid ID type");

  short id_type;
  switch (id_type_string[0]) {
    case 'F': id_type = cpl::ogn::ID_TYPE_FLARM; break;
    case 'I': id_type = cpl::ogn::ID_TYPE_ICAO ; break;
    case 'O': id_type = cpl::ogn::ID_TYPE_OGN  ; break;
    default : id_type = 0;
              cpl::util::verify(false,
                                lex.location() + "ID type must be O, I or F");
  }

  cpl::util::expect(lex, cpl::util::COMMA);

  cpl::util::expect(lex, cpl::util::STRING);
  auto const id = lex.string_value();
  cpl::util::verify(6 == id.size(), 
                    lex.location() + "invalid ID size (must be 6 digits)");

  cpl::util::expect(lex, cpl::util::COMMA);

  cpl::util::expect(lex, cpl::util::STRING);
  auto const type = lex.string_value();
  cpl::util::verify(type.size() <= 40, 
                    lex.location() + "invalid type (must be <= 40 characters)");

  cpl::util::expect(lex, cpl::util::COMMA);

  cpl::util::expect(lex, cpl::util::STRING);
  auto callsign = lex.string_value();
  cpl::util::verify(callsign.size() <= 10, 
                    lex.location() 
                    + "invalid callsign (must be <= 10 characters)");

  cpl::util::expect(lex, cpl::util::COMMA);

  cpl::util::expect(lex, cpl::util::STRING);
  auto cn = lex.string_value();
  cpl::util::verify(cn.size() <= 4,
                    lex.location() 
                    + "invalid competition number (must be <= 4 characters)");

  cpl::util::expect(lex, cpl::util::COMMA);

  cpl::util::expect(lex, cpl::util::STRING);
  auto const tracking = parse_bool(lex.string_value(), lex.location());

  cpl::util::expect(lex, cpl::util::COMMA);

  cpl::util::expect(lex, cpl::util::STRING);
  auto const identify = parse_bool(lex.string_value(), lex.location());

  // Parsing OK, now post-process data
  std::replace(callsign.begin(), callsign.end(), ' ', '_');
  cpl::util::toupper(callsign);
  cpl::util::verify_alnum(callsign, "-_");

  std::replace(cn      .begin(), cn      .end(), ' ', '_');
  cpl::util::toupper(cn);
  cpl::util::verify_alnum(cn, "-_");

  if (!identify || 0 == callsign.size()) {
    callsign = "(hidden)";
  }

  if (!identify || 0 == cn.size()) {
    cn = "-";
  }

  // Use unqualified ID.  This is a primary key in the DDB
  // and users regularly get the ID type wrong (called
  // 'Device type' in the UI as of July, 2015).
  static_cast<void>(id_type);
  return std::make_pair(
      // cpl::ogn::qualified_id(id, id_type),
      id,
      cpl::ogn::vehicle_data{callsign, cn, type, tracking, identify});
}

cpl::ogn::vehicle_db
cpl::ogn::get_vehicle_database_ddb(std::ostream& sl, std::string const& url) {
  cpl::ogn::vehicle_db ret;
  
  try {
    std::ifstream ifs;
    std::istringstream iss;
    std::istream* is = NULL;

    if (0 == url.find("http://")) {
      std::ostringstream oss;
      cpl::http::wget(sl, oss, url);
      iss = std::istringstream{oss.str()};
      is = &iss;
    } else {
      ifs.open(url);
      if (!ifs) {
        throw std::runtime_error("couldn't open " + url);
      }
      is = &ifs;
    }
    always_assert(is != NULL);

    cpl::util::lexer_style_t const ddb_style{ 
      cpl::util::hash_comments, cpl::util::single_quote};

    cpl::util::lexer lex{*is, url, ddb_style};

    while (cpl::util::END != lex.peek_token()) {
      cpl::ogn::vehicle_data_and_name ent;
      try {
        ent = parse_ddb_entry(lex);
      } catch (std::exception const& e) {
        sl << prio::WARNING
           << "Couldn't parse DDB entry: " << e.what() << std::endl;
      }
      ret.insert(ent);
    }
    sl << prio::INFO 
       << "Parsed " << ret.size() << " DDB record(s) from " << url
       << std::endl;
  } catch (std::exception const& e) {
    sl << prio::ERR
       << "Failed to parse DDB from " << url << ": "
       << e.what()
       << std::endl;
  }
  return ret;
}

cpl::ogn::thermal_detector_params::thermal_detector_params(
    int const method)
: method(method) {
  validate();
}


void cpl::ogn::thermal_detector_params::validate() {
  cpl::util::verify(0 <= method && method <= 2,
      "thermal detector method must be 0, 1 or 2");
}

cpl::ogn::thermal_detector_params::thermal_detector_params()
: thermal_detector_params(2) {}

cpl::ogn::thermal_detector_params
cpl::ogn::thermal_detector_params_from_registry(
    cpl::util::registry const& reg,
    thermal_detector_params const& defaults) {
  thermal_detector_params ret;

  ret.method = 
      reg.get_default("method", static_cast<long>(defaults.method));
  ret.dot_size = 
      reg.get_default("dot_size", static_cast<long>(defaults.dot_size));
  ret.max_time_delta = 
      reg.get_default("max_time_delta", defaults.max_time_delta);
  ret.max_speed = 
      reg.get_default("max_speed", defaults.max_speed);
  ret.min_turnrate_glider = 
      reg.get_default("min_turnrate_glider", defaults.min_turnrate_glider);
  ret.min_climbrate = 
       reg.get_default("min_climbrate", defaults.min_climbrate);

  ret.validate();
  return ret;
}


cpl::ogn::thermal cpl::ogn::detect_thermal(
    cpl::ogn::thermal_detector_params const& params,
    cpl::ogn::aircraft_rx_info const& rx) {
  cpl::ogn::thermal ret;

  if (!valid(rx.pta)) {
    return ret;
  }

  if (cpl::ogn::VEHICLE_TYPE_GLIDER == rx.vehicle_type) {
    if (   rx.mot.speed <= params.max_speed
        && rx.mot.turnrate >= params.min_turnrate_glider
        && rx.mot.vertical_speed >= params.min_climbrate) {
      ret.pt        = rx.pta;
      ret.climbrate = rx.mot.vertical_speed;
    }
    return ret;
  }

  if (   cpl::ogn::VEHICLE_TYPE_DELTA == rx.vehicle_type
      || cpl::ogn::VEHICLE_TYPE_PARAGLIDER == rx.vehicle_type) {
    // Don't consider turn rate for PG and deltas
    if (   rx.mot.speed <= params.max_speed
        && rx.mot.vertical_speed >= params.min_climbrate) {
      ret.pt        = rx.pta;
      ret.climbrate = rx.mot.vertical_speed;
    }
    return ret;
  }

  // None of the above, no thermal
  return ret;
}

cpl::ogn::thermal cpl::ogn::detect_thermal(
    cpl::ogn::thermal_detector_params const& params,
    cpl::ogn::aircraft_rx_info const& current,
    cpl::ogn::aircraft_rx_info const* const previous) {

  if (1 == params.method || NULL == previous) {
    return cpl::ogn::detect_thermal(params, current);
  }

  cpl::ogn::thermal ret;
  if (2 != params.method) {
    return ret;
  }

  if (!(   cpl::ogn::VEHICLE_TYPE_GLIDER     == current.vehicle_type
        || cpl::ogn::VEHICLE_TYPE_DELTA      == current.vehicle_type
        || cpl::ogn::VEHICLE_TYPE_PARAGLIDER == current.vehicle_type)) {
    return ret;
  }

  if (!(valid(current.pta) && valid(previous->pta))) {
    return ret;
  }

  double const dt = current.pta.time - previous->pta.time;
  // std::cout << "dt: " << dt << std::endl;
  if (dt <= 0.1 || dt >= params.max_time_delta) {
    return ret;
  }
  
  double const pa1 = potential_altitude(previous->pta.alt, previous->mot);
  double const pa2 = potential_altitude(current .pta.alt, current .mot);

  // std::cout << "pa1: " << pa1 << std::endl;
  // std::cout << "pa2: " << pa2 << std::endl;
  double const climbrate = (pa2 - pa1) / dt;
  // std::cout << "cr: " << climbrate << std::endl;
  if (climbrate >= params.min_climbrate) {
    // TODO: Use in-between position?
    ret.pt        = current.pta;
    ret.climbrate = climbrate;
  }
  return ret;
}

void cpl::ogn::update(cpl::ogn::thermal_detector_params const& params,
                      cpl::ogn::thermal_tileset& tts, 
                      cpl::ogn::thermal const& th) {
  if (!valid(th.pt) || !tts.inside(th.pt)) {
    return;
  }

  // Add another dot-size to make the dots rounder... :)
  auto const r2 = cpl::math::square(params.dot_size - 1) + params.dot_size;

  for (int z = tts.minzoom(); z <= tts.maxzoom(); ++z) {
    // TODO: Optimize this!  Currently, for each zoom level the
    // Mercator projection is recomputed
    auto const fc = tts.mapper().get_full_coordinates(z, th.pt);
    auto& tile = tts.tile_at_create(z, fc.tile);

    for (int y =  fc.pixel.y - params.dot_size + 1; 
             y <= fc.pixel.y + params.dot_size - 1; ++y) {
    for (int x =  fc.pixel.x - params.dot_size + 1; 
             x <= fc.pixel.x + params.dot_size - 1; ++x) {

      // round dots
      if (     cpl::math::square(x - fc.pixel.x)
           +   cpl::math::square(y - fc.pixel.y) <= r2
          && 0 <= x && x < tts.tilesize() && 0 <= y && y < tts.tilesize()) {
        cpl::ogn::update_thermal_aggregator(tile[x][y], th);
      }
    }}
  }
}

void cpl::ogn::unittests(std::ostream& os) {
double lat1 = 1, lon1 = 2;
  double lat2 = -1, lon2 = -2;

  os << "OGN unit tests" << std::endl;
  always_assert(set_latlon_dao("!W55!", lat1, lon1));
  always_assert(set_latlon_dao("!W55!", lat2, lon2));

  os << std::setprecision(8) << lat1 << ' ' << lon1 << std::endl;
  os << std::setprecision(8) << lat2 << ' ' << lon2 << std::endl;

  double lat3 = 3, lon3 = 4;
  always_assert(set_latlon_dao("!w&(!", lat3, lon3));
  os << std::setprecision(8) << lat3 << ' ' << lon3 << std::endl;
}
