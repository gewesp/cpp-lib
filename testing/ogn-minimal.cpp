//
// Minimal OGN client example
//

#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

#include <cassert>

#include "cpp-lib/sys/network.h"
#include "cpp-lib/ogn.h"

using namespace cpl::util::network;
using namespace cpl::ogn;

void process(aprs_parser& parser, std::istream& is, std::ostream& os) {
  std::string line;

  aircraft_rx_info_and_name acft;
  station_info_and_name     stat;

  while (std::getline(is, line)) {
    if (line.empty() || '#' == line[0]) { continue; }

    if (parser.parse_aprs_aircraft(line, acft)) {
      os << acft.first << ' ' << acft.second << std::endl;
    } else if (parse_aprs_station(line, stat)) {
      os << stat.first << ' ' << stat.second << std::endl;
    } else {
      os << "parse error: " << line << std::endl;
    }
  }
}

int main() {
  try {
    // Instantiate parser, query DDB once per hour
    aprs_parser parser(std::cerr, 3600);

    // Connect to OGN and log in
    auto c = connect(std::cerr);
    instream is(*c);
    onstream os(*c);

    login(std::clog, os, is, "test-client v1.00");

    // Process data stream, write to stdout
    process(parser, is, std::cout);
  } catch (std::exception const& e) { 
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  }
}
