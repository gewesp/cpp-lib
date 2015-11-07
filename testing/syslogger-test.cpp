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
#include <exception>
#include <stdexcept>

#include "cpp-lib/registry.h"
#include "cpp-lib/util.h"
#include "cpp-lib/sys/syslogger.h"


using namespace cpl::util::log;

double testclock() {
  return cpl::util::parse_datetime("2015-02-14T09:43:48Z");
}

void loop(std::ostream& l, const int n) {
  for (int i = 1; i <= n; ++i) {
    l << i << std::endl;
  }
}

void log_all_prios(std::ostream& sl) {
  sl << prio::DEBUG   << "DEBUG"     << std::endl;
  sl << prio::INFO    << "INFO"      << std::endl;
  sl << prio::NOTICE  << "NOTICE"    << std::endl;
  sl << prio::WARNING << "WARNING"   << std::endl;
  sl << prio::ERR     << "ERROR"     << std::endl;
  sl << prio::CRIT    << "CRITICAL"  << std::endl;
  sl << prio::ALERT   << "ALERT"     << std::endl;
  // sl << prio::EMERG   << "EMERGENCY" << std::endl;
}

void test(std::ostream& sl) {
  sl << "Program initialized" << std::endl;

  sl << "Counting from 1 to 5" << std::endl;
  loop(sl, 5);
  sl << "Done counting" << std::endl;

  sl << prio::NOTICE << "Log priority ..." << std::endl;
  sl << "... auto-resets to INFO" << std::endl;

  sl << setminprio(prio::NOTICE) 
     << prio::NOTICE << "Log everything >= DEBUG"  << std::endl;
  sl << setminprio(prio::DEBUG);
  log_all_prios(sl);

  sl << setminprio(prio::NOTICE) 
     << prio::NOTICE << "Log everything >= NOTICE" << std::endl;
  sl << setminprio(prio::NOTICE);
  log_all_prios(sl);

  sl << setminprio(prio::NOTICE) 
     << prio::NOTICE << "Log everything >= ERROR"  << std::endl;
  sl << setminprio(prio::ERR);
  log_all_prios(sl);

  // std::endl flushes message and resets log level to the default INFO
  sl << setminprio(prio::INFO);

  // The next message should be suppressed
  sl << prio::DEBUG
     << "If this message appears in output, something is WRONG!!!!" 
     << std::endl;

  sl << prio::NOTICE << "Also empty messages are printed" << std::endl;
  sl << prio::NOTICE << "XXX" << std::endl;
  sl << prio::NOTICE << "XX" << std::endl;
  sl << prio::NOTICE << "X" << std::endl;
  sl << prio::NOTICE << "" << std::endl;
  sl << prio::NOTICE << "X" << std::endl;
  sl << prio::NOTICE << "XX" << std::endl;

  // Don't log the XXX to echo
  sl << setminprio(prio::ALERT, ECHO);
  sl << prio::NOTICE << "XXX" << std::endl;
  sl << setminprio(prio::INFO);

  sl << setminprio(prio::EMERG, SYSLOG);
  sl << prio::ALERT
     << "Hi there.  Don't worry, it's just a cpp-lib/syslogger test!"
     << std::endl;

  sl << prio::NOTICE << "Very long lines get split." << std::endl;

  sl << prio::NOTICE << std::string(1100, 'x') << std::endl;

  sl << prio::NOTICE << "End of xxxxxx" ; 

  sl << prio::NOTICE << "Forgot std::endl!" << std::endl;

  sl << prio::NOTICE << "Exiting..." << std::endl;

  // The *last* log priority before the std::endl is relevant,
  // so this is treated as DEBUG.
  sl << prio::NOTICE << "NOTICE message";
  sl << prio::DEBUG  << "DEBUG message" ;
  // autoflush at EOF
}


int main() {

  try {

    // Logs to the LOG_USER facility, prepends the given tag
    // to each message.
    // Logs level INFO or above by default.
    {
      std::cout << "Testing with stdout echo" << std::endl;
      syslogger sl("", &std::cout, testclock);
      test(sl);
      std::cout << "Disabling stdout echo" << std::endl;
      sl.set_echo_stream(NULL);
      test(sl);
      std::cout << "Re-enabling stdout echo" << std::endl;
      sl.set_echo_stream(&std::cout);
      test(sl);
    }

    {
      syslogger sl("syslogger-test", &std::cout, testclock);
      sl << prio::NOTICE << "Testing with a tag" << std::endl;
    }

    {
      syslogger sl("syslogger-test-noecho");
      std::cout << "Testing without stdout echo" << std::endl;
      sl << prio::NOTICE << "This should not appear on stdout" << std::endl;
    }

    std::cout << "Testing direct (unformatted) output to stdout" << std::endl;
    test(std::cout);

  } catch( std::exception const& e ) { 
    std::cerr << e.what() << std::endl;
    return 1;
  }

}
