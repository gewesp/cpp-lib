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

//
// Finds the (unique?) nontrivial minimum or the trivial global
// minimum of the d-dimensional Rosenbrock function by optimizing 
// with random starting points.
//
// Allows instrumentation to count the number of function evaluations
// and possible duplicate evaluations.
//
// In 4 dimensions, this should find:
//
//   (-0.7757, 0.613, 0.382, 0.146).
//
// Instrumentating shows that sometimes the minimizer has a slow start, 
// evaluating 6 or 7 very close arguments.
//
// Uses a high-quality random number generator to stay clear of
// high-dimensional correlation issues.
//
//

#include <iostream>
#include <random>
#include <vector>

#include <boost/lexical_cast.hpp>

#include "cpp-lib/command_line.h"
#include "cpp-lib/optimization.h"
#include "cpp-lib/minimize.h"
#include "cpp-lib/math-util.h"
#include "cpp-lib/matrix-wrapper.h"

using namespace cpl::math ;
using namespace cpl::util ;


namespace {

// Minimum distance from global minimum to consider 'success'.
double const d1_threshold = 1e-5 ;

double const norm_grad_threshold = 9e-7 ;
long const linesearch_threshold = 500 ;
long const maxit = 1500 ;

double const sigma = 10 ;


// Command line.

const opm_entry options[] = {

  opm_entry( "arguments" , opp( false , 'a' ) ) ,
  opm_entry( "repeat"    , opp( true  , 'r' ) )

} ;


void search( int const d , long const repeat , bool const arguments ) {

  std::cout << "Searching non-trivial minima of the "
            << d
            << "-dimensional "
            << "Rosenbrock function.\n" ;

  std::mt19937 mt ;
  std::normal_distribution<double> N( 0.0 , sigma ) ;

  // The global minimum at (1, ..., 1).
  cpl::matrix::vector_t x_global( d ) ;
  x_global.fill( 1 ) ;

  for ( int rr = 0 ; rr < repeat ; ++rr ) {

    // Determine starting point around global minimum
    cpl::matrix::vector_t x0( d ) ;
    for( int j = 0 ; j < d ; ++j ) {
      x0( j ) = 1 + N( mt ) ;
    }

    std::cout << "Starting at x0 = " << cpl::matrix::transpose(x0) << std::endl;

    long n_ls = 0 ;

    rosenbrock_f rf( arguments ) ;
    cpl::matrix::vector_t const x_min = minimize( x0 , rf , maxit , 1. , &n_ls ) ;

    if( arguments ) {
      std::cout << "Evaluated " << rf.n_eval << " arguments\n" ;
      std::cout
        << "sorted vectors, # occurrences, last time evaluated, "
           "delta to next\n" ;
      for( rosenbrock_f::argcnt_t::const_iterator i  = rf.evals.begin() ;
                                                  i != rf.evals.end  () ;
                                                  ++i ) {
        // i->second.first ... number of evaluations for that argument.
        // i->second.second ... last time (eval counter) this argument was
        // evaluated.
        always_assert( i->second.first >= 1 ) ;
        if( i->second.first >= 2 ) {
          std::cout << "*** WARNING: multiple evaluation! ***\n" ;
        }
        std::cout << cpl::matrix::transpose( i->first ) 
                  << std::endl
                  << i->second.first << ' ' << i->second.second ;
        rosenbrock_f::argcnt_t::const_iterator j = i ;
        ++j ;
        if( j != rf.evals.end() ) {
          std::cout << ' ' << cpl::matrix::norm_2( i->first - j->first ) ;
        }
        std::cout << std::endl ;
      }
      std::cout << "end arguments\n" ;
    }

    always_assert( x_min.rows() == d ) ;

    double const d1 = cpl::matrix::norm_2( x_min - x_global ) ;
    if( d1 > d1_threshold ) {

      double const norm_grad =
          cpl::matrix::norm_2( rosenbrock_gradient( x_min ) ) ;

      if( norm_grad >= norm_grad_threshold || n_ls >= linesearch_threshold ) {
        std::cout << "x0 = "
                  << cpl::matrix::transpose( x0 )
                  << "line searches: "
                  << n_ls
                  << "\n|grad f(x_min)| = "
                  << norm_grad
                  << std::endl
        ;
      }
    }
    std::cout << "Found minimum at: " << cpl::matrix::transpose( x_min ) 
              << std::endl;

  }

}

void usage( std::string const& name ) {

  std::cerr << name
            << " [ --repeat <n> --arguments | -a ] [d]\n"
            << "Search n times for nontrivial minima of the d-dimensional "
            << "Rosenbrock function.  If not given, d = 4.\n"
            << "If --arguments is given, display the points of function "
            << "evaluation together with the frequency.\n"
  ;

}


} // anonymous namespace

int main( int , char const * const * const argv ) {

  try {

  long d = 4 ;

  command_line cl( options , options + size( options ) , argv ) ;

  bool const arguments = cl.is_set( "arguments" ) ;
  long const repeat = cl.is_set( "repeat" ) ?
    boost::lexical_cast<long>(cl.get_arg("repeat")) : 1;

  std::string ds ;
  if( cl >> ds ) {
    try {
      d = boost::lexical_cast< long >( ds ) ;
    } catch( std::exception const& ) {
      usage( argv[ 0 ] ) ;
      return 1 ;
    }
  }
  // Check that no more arguments are present.
  if( cl >> ds ) {
    usage( argv[ 0 ] ) ;
    return 1 ;
  }

  search( d , repeat , arguments ) ;

  } catch( std::runtime_error const& e ) {
    std::cerr << e.what() << std::endl ;
    return 1 ;
  }

}
