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
// TODO: Add DDB tests to run-all-tests.sh .  Requires anonymized
// version of a DDB.
//

#include <algorithm>
#include <exception>
#include <iostream>
#include <iterator>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>

#include <cassert>

#include "cpp-lib/sys/network.h"
#include "cpp-lib/command_line.h"
#include "cpp-lib/container-util.h"
#include "cpp-lib/gnss.h"
#include "cpp-lib/ogn.h"
#include "cpp-lib/util.h"


using namespace cpl::util::network ;


const std::string DEFAULT_HOST    = "aprs.glidernet.org";
const std::string DEFAULT_SERVICE = "14580"             ;

// Keepalive message for OGN server, sent each time we receive one
const std::string KEEPALIVE_MESSAGE = "We are alive";



const cpl::util::opm_entry options[] = {
  cpl::util::opm_entry("anonymize", cpl::util::opp(true, 'a')),
  cpl::util::opm_entry("file"    , cpl::util::opp(true , 'f')),
  cpl::util::opm_entry("raw"     , cpl::util::opp(false, 'r')),
  cpl::util::opm_entry("help"    , cpl::util::opp(false, 'h')),
  cpl::util::opm_entry("center"  , cpl::util::opp(true , 'c')),
  cpl::util::opm_entry("radius"  , cpl::util::opp(true      )),
  cpl::util::opm_entry("thermals", cpl::util::opp(true      )),
  cpl::util::opm_entry("utc"     , cpl::util::opp(true , 'u')),
  cpl::util::opm_entry("unittests", cpl::util::opp(false    )),
  cpl::util::opm_entry("minalt"  , cpl::util::opp(true , 'm')),
  cpl::util::opm_entry("ddb_stats", cpl::util::opp(false    )),
  cpl::util::opm_entry("ddb_query_interval", cpl::util::opp(true , 'q')),
  cpl::util::opm_entry("ddb_source", cpl::util::opp(true , 's'))
};


void usage( std::string const& name ) {

  std::cerr << 
"Receives data from Open Glider Network servers, parses and outputs it.\n"
"Usage: " << name << " [ --file <source> ] [ --raw ]\n"
"--file <source>:    Read packets from file.  If not given, connects to\n"
"                    " << cpl::ogn::default_host() << ":" 
                       << cpl::ogn::default_service() << ".\n"
"--raw:               Do not parse packets, output raw APRS data instead.\n"
"--center <lat,lon>   Filter AIRCRAFT packets around center [deg].\n"
"--radius <rad>       Radius for AIRCRAFT filtering [m].\n"
"--thermals           Detect thermals.\n"
"--utc <seconds>      Use UTC in seconds since epoch instead of current time.\n"
"                     -1: Use current time\n"
"                     -2: Parse from file, default for --file\n"
"--minalt <wgs84_alt> Minimum altitude for AIRCRAFT filtering [m].\n"
"--ddb_query_interval <seconds>\n"
"                     Query interval for DDB, -1 for no queries.\n"
"--ddb_source <s>     Read DDB from the given URL (if http://...) or file.\n"
"--unittests:         Run unit tests.\n"
"--anonymize <key>:   Scramble IDs in input stream by <key> and output again.\n"
"--ddb_stats:         Download DDB and display statistics\n"
"--help:              Display this message.\n"
  ;

}

struct filter {
  filter() : radius(-1) {}

  filter(cpl::util::command_line const& cl);

  // Center, alt serves as minimum altitude.
  cpl::gnss::position_time pt;

  // Radius [m], -1 == no filtering
  double radius;

  // Apply the filter
  bool apply(cpl::gnss::position_time const& pt) const;
};

void process(
    cpl::ogn::aprs_parser& parser,
    std::istream& is, 
    std::ostream& os, 
    filter const& filt,
    double const utc,
    const bool raw,
    cpl::ogn::thermal_detector_params const& tparams,
    std::ostream* const keepalive) {
  std::string line;

  if (!raw) {
    if (tparams.method) {
      os << cpl::ogn::thermal_format_comment() << std::endl;
    } else {
      os << "# AIRCRAFT "
         << "id "
         << "id_type "
         << "vehicle_type "
         << "stealth "
         << "tracking "
         << "identify "
         << "callsign "
         << "hwver "
         << "swver "
         << "time "
         << "lat "
         << "lon "
         << "alt "
         << "accuracy "
         << "course "
         << "speed "
         << "vertical_speed "
         << "turn_rate "
         << "baro_alt "
         << "received_by "
         << "tocall "
         << "relayed "
         << "relay "
         << "rssi "
         << "frequency_deviation "
         << "errors "
         << std::endl
      ;
      os << "# STATION "
         << "id "
         << "version "
         << "network "
         << "time "
         << "lat "
         << "lon "
         << "alt "
         << "cpu "
         << "ram_used "
         << "ram_max "
         << "ntp_difference "
         << "ntp_ppm "
         << "temperature"
         << std::endl
      ;
    }
  }

  // A rectangle comprising the Western Alps
  cpl::gnss::lat_lon const nw{47.8, 4.8};
  cpl::gnss::lat_lon const se{43.7, 12};

  auto const minzoom = 1;
  auto const maxzoom = 10;

  cpl::map::tileset_parameters tsp{nw, se, minzoom, maxzoom};
  tsp.tileset_name = "thermals";

  cpl::ogn::thermal_tileset tts{tsp};

  cpl::ogn::aircraft_db acdb;

  double utc_parsed = 0;

  while (std::getline(is, line)) {
    if (line.empty()) {
      continue;
    }
    if ('#' == line[0]) {
      if (raw) {
        os << line << std::endl;
      } else {
        os << "KEEPALIVE " << line.substr(2, std::string::npos) << std::endl;
      }

      if (keepalive) {
        *keepalive << "# " << KEEPALIVE_MESSAGE << std::endl;
      }
      continue;
    }
    if (utc <= -2) {
      if (line.find("TIME ") == 0) {
        utc_parsed = cpl::util::parse_datetime(line.substr(5));
        continue;
      }
    }
    if (raw) {
      os << line << std::endl;
    } else {
      cpl::ogn::aircraft_rx_info_and_name acft;
      const double utc_now = utc > 0 ? utc : 
         (utc <= -2 ? utc_parsed : cpl::util::utc());
      if (parser.parse_aprs_aircraft(line, acft, utc_now)) {
        // Previous aircraft with same info
        auto const it = acdb.find(acft.first);
        cpl::ogn::aircraft_rx_info const* const pprev = acdb.end() == it ?
            NULL : &it->second;

        if (filt.apply(acft.second.pta)) {
          if (tparams.method) {
            auto const th = cpl::ogn::detect_thermal(
                tparams, acft.second, pprev);
#if 0
            if (pprev) {
              std::cout << acft.first << ": dt = "
                        << acft.second.pta.time - pprev->pta.time
                        << std::endl;
            }
#endif

            acdb[acft.first] = acft.second;

            if (valid(th.pt)) {
              os << th << std::endl;
              update(tparams, tts, th);
            }
          } else {
            os << "AIRCRAFT "
               << acft.first << " " << acft.second << std::endl;
          }
        }
      } else {
        cpl::ogn::station_info_and_name stat;
        if (cpl::ogn::parse_aprs_station(line, stat, utc_now)) {
          if (!tparams.method) {
            os << "STATION "
               << stat.first << " " << stat.second << std::endl;
          }
        } else {
          std::string what;
          try {
            parser.parse_aprs_aircraft(line, acft, utc_now, true);
          } catch (const std::exception& e) {
            what = e.what();
          }
          os << "# WARNING: Couldn't parse: " << line;
          if (not what.empty()) {
            os << ": " << what;
          }
          os << std::endl;
        }
      }
    }
  }
  if (tparams.method) {
    write_static_info(os, tts);
    write_dynamic_info(os, tts);

    cpl::gnss::lat_lon const ch{47, 8};
    auto const ch_tile = tts.tile_at(1, ch);
    if (NULL != ch_tile) {
      cpl::util::write_array(os, *ch_tile);
    }
  }
}

bool keepalive_or_empty(std::string const& line) {
  return line.empty() || '#' == line[0];
}

// Parses all *stations* and returns a map thereof.  Values
// are the last info received (in sequential file order).
std::map<std::string, cpl::ogn::station_info>
parse_all_stations(std::string const& filename) {
  auto is = cpl::util::file::open_read(filename);

  std::map<std::string, cpl::ogn::station_info> themap;

  cpl::ogn::station_info_and_name stat;

  std::string line;
  while (std::getline(is, line)) {
    if (keepalive_or_empty(line)) {
      continue;
    }
    if (!cpl::ogn::parse_aprs_station(line, stat)) {
      continue;
    }
    // Upsert operation.
    themap[stat.first] = stat.second;
  }

  return themap;
}



filter::filter(cpl::util::command_line const& cl) {
  if (!cl.is_set("center")) {
    radius = -1;
    return;
  }

  if (2 != std::sscanf(
      cl.get_arg("center").c_str(), "%lf,%lf", &pt.lat, &pt.lon)) {
    throw std::runtime_error("<lat,lon> in degrees required");
  }
  pt.alt = cl.is_set("minalt") ? boost::lexical_cast<double>(
      cl.get_arg("minalt")) : 0;

  radius = cl.is_set("radius") ? boost::lexical_cast<double>(
      cl.get_arg("radius")) : 5000;
}


bool filter::apply(cpl::gnss::position_time const& query) const {
  if (radius < 0) {
    return true;
  }

  cpl::gnss::position_time q1 = query;

  // Pretend we're at same altitude and use threed_distance
  q1.alt = this->pt.alt;

  const double dist = threed_distance(q1, pt);

  return dist <= radius && query.alt >= pt.alt;
}

int anonymize(std::istream& is, std::ostream& os, std::string const& k) {
  int const key = std::stol(k, 0, 0);

  std::string const id = "([0-9A-F]{6,6})";

  std::regex const re_id1("(ICA|FLR|OGN|RND)" + id,
      std::regex_constants::egrep);
  std::regex const re_id2("(id..)"            + id,
      std::regex_constants::egrep);

  std::string s;
  while (std::getline(is, s)) {
    auto const beg_id1 = std::sregex_iterator(s.begin(), s.end(), re_id1);
    auto const beg_id2 = std::sregex_iterator(s.begin(), s.end(), re_id2);
    auto const end = std::sregex_iterator();

    if (beg_id1 != end && beg_id2 != end) {
      if (   1 != std::distance(beg_id1, end)
          || 1 != std::distance(beg_id2, end)) {
        goto output;
      }
      // http://en.cppreference.com/w/cpp/regex/match_results/str
      auto const id1 = beg_id1->str(2);
      auto const id2 = beg_id2->str(2);
      if (id1 != id2) { goto output; }

      // Convert hex, add l and convert back to string
      long const id_scrambled = std::stol(id1, 0, 16) + key;

      char buf[10];
      std::sprintf(buf, "%06lX", id_scrambled & 0xffffff);

      s = std::regex_replace(s, std::regex(id1), buf);
    }
output:
    os << s << std::endl;
  }

  return 0;
}

// Count occurrences of id_type in vdb
int count_id_type(cpl::ogn::vehicle_db const& vdb, short const id_type) {
  return std::count_if(vdb.begin(), vdb.end(),
      [id_type](cpl::ogn::vehicle_db::value_type const& v) {
          return id_type == v.data.id_type_probably_wrong;
        });
}

void ddb_stats(std::ostream& os, std::string const& source) {
  auto const vdb = cpl::ogn::get_vehicle_database_ddb(os, source);

  // Count occurrences of types
  std::map<std::string, int> types;
  for (auto const& el : vdb) {
    ++types[el.data.type];
  }

  // Sort found types in descending order of frequency
  typedef std::pair<std::string, int> si;
  std::vector<si> freq(types.begin(), types.end());
  std::sort(freq.begin(), freq.end(), 
            [](si const& p1, si const& p2) { return p1.second > p2.second; });

  auto const n_flarm = count_id_type(vdb, cpl::ogn::ID_TYPE_FLARM );
  auto const n_icao  = count_id_type(vdb, cpl::ogn::ID_TYPE_ICAO  );
  auto const n_ogn   = count_id_type(vdb, cpl::ogn::ID_TYPE_OGN   );

  auto const n_flarm_from_id = std::count_if(vdb.begin(), vdb.end(),
      [](cpl::ogn::vehicle_db::value_type const& v) {
          return (   cpl::ogn::ID_TYPE_FLARM == v.data.id_type_probably_wrong
                  || cpl::ogn::ID_TYPE_ICAO  == v.data.id_type_probably_wrong)
                  && (   "DD" == v.id.substr(0, 2)
                      || "DE" == v.id.substr(0, 2)
                      || "DF" == v.id.substr(0, 2));
        });

  auto const n_icao_from_id = std::count_if(vdb.begin(), vdb.end(),
      [](cpl::ogn::vehicle_db::value_type const& v) {
          return (   cpl::ogn::ID_TYPE_FLARM == v.data.id_type_probably_wrong
                  || cpl::ogn::ID_TYPE_ICAO  == v.data.id_type_probably_wrong)
                  && not (   "DD" == v.id.substr(0, 2)
                          || "DE" == v.id.substr(0, 2)
                          || "DF" == v.id.substr(0, 2));
        });

  const auto table_stats = cpl::ogn::get_table_statistics(vdb);

  os << "Open Glider Network DDB statistics" << std::endl;
  os << "Date: " << cpl::util::format_datetime(cpl::util::utc()) << std::endl;
  cpl::db::write(os, table_stats);
  os << "Total number of registered devices: " << vdb.size() << std::endl;
  os << "Number of FLARM IDs (D{DEF}xxxx): " 
     << n_flarm_from_id << std::endl; 
  os << "Number of ICAO IDs (not D{DEF}xxxx): " 
     << n_icao_from_id << std::endl; 
  os << "Number of OGN IDs: " 
     << n_ogn << std::endl; 
  os << "Number of IDs claimed FLARM: " 
     << n_flarm << std::endl; 
  os << "Number of IDs claimed ICAO: " 
     << n_icao << std::endl; 
  os << "Total number of registered aircraft types: "
     << freq.size() << std::endl;
  os << "Aircraft types in decreasing order of frequency: " << std::endl;
  for (auto const& el : freq) {
    os << el.second << ' ' << el.first << std::endl;
  }

  os << "Aircraft with single character competition number (CN):" << std::endl;
  for (char c = 'A'; c <= 'Z'; ++c) {
    auto const v = cpl::ogn::lookup_by_name2(vdb, std::string() + c);
    os << "CN '" << c << "' is used by: ";
    if (0 == v.size()) {
      os << "(none)";
    } else {
      for (auto const& el : v) {
        os << el.data.name1 << ' ';
      }
    }
    os << '\n';
  }

  {
    os << "Duplicate callsigns:\n";
    // Cross-check total number of entries
    unsigned total = 0;
    auto const& idx = by_name1(vdb);
    auto it = idx.begin();
    while (idx.end() != it) {
      auto const cs = it->data.name1;
      std::vector<std::string> ids;
      do {
        ids.push_back(it->id);
        ++it;
      } while (idx.end() != it && cs == it->data.name1);
      total += ids.size();
      always_assert(1 <= ids.size());
      if ("-" != cs && 2 <= ids.size()) {
        os << cs << ": ";
        for (auto const& id : ids) {
          os << id << " ";
        }
        os << '\n';
      }
    }
    always_assert(total == vdb.size());
  }
}


int main( int , char const* const* const argv ) {

  try {

  cpl::util::command_line cl( options , options + size( options ) , argv ) ;

  filter filt(cl);

  if (cl.is_set("help")) {
    usage(argv[0]);
    return 0;
  }

  if (cl.is_set("unittests")) {
    cpl::ogn::unittests(std::cout);
    return 0;
  }

  if (cl.is_set("ddb_stats")) {
    std::string const ddb_source =
          cl.is_set("ddb_source") ? cl.get_arg("ddb_source")
        : cpl::ogn::default_ddb_url();
    ddb_stats(std::cout, ddb_source);
    return 0;
  }

  std::unique_ptr<connection> c;
  std::unique_ptr<std::istream> is;
  std::unique_ptr<std::ostream> keepalive;

  double const ddb_query_interval = 
    cl.is_set("ddb_query_interval") ?
        boost::lexical_cast<double>(cl.get_arg("ddb_query_interval"))
      : cpl::ogn::default_ddb_query_interval();

  double const utc = 
    cl.is_set("utc") ?
        boost::lexical_cast<double>(cl.get_arg("utc"))
      : (cl.is_set("file") ? -2 : -1);

  cpl::ogn::aprs_parser parser(std::clog, ddb_query_interval);

  if (cl.is_set("file")) {
    is.reset(new cpl::util::file::owning_ifstream(
        cpl::util::file::open_read(cl.get_arg("file"))));
  } else {
    c = cpl::ogn::connect(std::clog);
    is.reset       (new instream(*c));
    keepalive.reset(new onstream(*c));
    cpl::ogn::login(
        std::clog, *keepalive, *is, "ogn-test v1.20", "" /* no filter */);
  }

  if (cl.is_set("anonymize")) {
    return anonymize(*is, std::cout, cl.get_arg("anonymize"));
  }

  int thermal_method = 0;
  if (cl.is_set("thermals")) {
    thermal_method = boost::lexical_cast<int>(cl.get_arg("thermals"));
  }

  cpl::ogn::thermal_detector_params const tparams{thermal_method};

  process(parser, *is, std::cout, filt, utc, 
      cl.is_set("raw"), tparams, keepalive.get());

  } // end global try
  catch( std::exception const& e ) { 
    cpl::util::die(std::string("error: ") + e.what()); 
    usage(argv[0]);
  }

}
