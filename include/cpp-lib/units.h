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
// Component: UTIL
//
// TODO:
// - C++11: Use constexpr once supported on all compilers.
// - Polish and use the physical_quantity<> template, see Bjarne Stroustrup's
//   2012 GoNative keynote about new C++11 features
//


#ifndef CPP_LIB_UNITS_H
#define CPP_LIB_UNITS_H

#include "cpp-lib/math-util.h"


namespace cpl {

namespace units {

//
// Values of some exotic units in SI.
//
// Sources:
//
// - http://www.chemie.fu-berlin.de/chemistry/general/units_en.html
// - http://www.sengpielaudio.com/Rechner-druckeinh.htm
// - http://www.wbuf.noaa.gov/tempfc.htm
// - http://www.wikipedia.org/
// - http://www.airbp.com/usga/index2b438.html?page=products&sub=avgas
//

inline double constexpr slug () { return 14.5939    ; } // kg
inline double constexpr pound() { return  0.4535924 ; } // kg

// Length units [m].
inline double constexpr foot         () { return     .3048 ; }
inline double constexpr inch         () { return     .0254 ; }
inline double constexpr nautical_mile() { return 1852.     ; }
inline double constexpr statute_mile () { return 1609.344  ; }
inline double constexpr kilometer    () { return 1000.     ; }
inline double constexpr flight_level () { return foot() * 100 ; }

// Volume units [m^3].
inline double constexpr us_gallon() { return 3.785412e-3 ; }

// Time [second].
inline double constexpr day   () { return 86400. ; } 
inline double constexpr hour  () { return 3600.  ; }
inline double constexpr minute() { return 60.    ; } 

inline double constexpr year  () { return 365 * day() ; }

inline double constexpr millisecond() { return 1e-3; }

// Angle [radians].
inline double constexpr degree    () { return cpl::math::pi / 180. ; }
inline double constexpr arc_minute() { return degree() / 60 ; }
inline double constexpr arc_second() { return degree() / 3600 ; }

// Velocity in [meter/second].
inline double constexpr knot                () { return nautical_mile() / hour  () ; }
inline double constexpr mile_per_hour       () { return statute_mile () / hour  () ; }
inline double constexpr kilometer_per_hour  () { return kilometer    () / hour  () ; } 
inline double constexpr foot_per_minute     () { return foot         () / minute() ; } 
inline double constexpr kph                 () { return kilometer_per_hour()       ; }

// Angular velocity [radians/second].
inline double constexpr rotation()
{ return 2 * cpl::math::pi ; }

inline double constexpr rotation_per_minute()
{ return rotation() / minute() ; }


// Temperature [K].
// Zero degrees Celsius.
inline double constexpr zero_celsius   () { return 273.15                     ; }
// Zero degrees Fahrenheit.
inline double constexpr zero_fahrenheit() { return 255.3722222222222222222222 ; }

inline double constexpr fahrenheit() { return .555555555555555555555555555555 ; }


// Weight force of a mass of one pound at average earth gravitation.
inline double constexpr lbf() { return 4.448222 ; } // N

// Pressure [N/m^2].
// @ 32 deg Fahrenheit, according to
// http://www.sengpielaudio.com/Rechner-druckeinh.htm
inline double constexpr inch_hg() { return 3386.388706 ; }

inline double constexpr pounds_per_square_inch()
{ return lbf() / cpl::math::square( inch() ) ; }

inline double constexpr european_horse_power() { return 735.4987 ; } // Watt

// According to Bart Oolbekkink, American horses are stronger than their
// Eurpoean brothers.

inline double constexpr american_horse_power() { return 745.7    ; } // Watt

// Derived units...
inline double constexpr us_gallon_per_hour() { return us_gallon() / hour() ; }


// The following aren't really units, but well...  

// The somehow ``average'' gravitation on our earth.
inline double constexpr gravitation() { return 9.80665 ; } // m/s^2

// Also not really...  The earth's ``average'' radius according to the
// IUGG [m].
inline double constexpr earth_radius() { return 6378137 ; }

// The density of Avgas 100LL fuel at some ``standard'' temperature [kg/m^3].
inline double constexpr density_avgas() { return 700 ; }
// The density of Jet A-1 fuel at some ``standard'' temperature [kg/m^3].
inline double constexpr density_jet_a1() { return 800 ; }


// The mass of a US gallon of Avgas 100LL at some ``standard''
// temperature [kg].
inline double constexpr us_gallon_avgas() { return us_gallon() * density_avgas() ; }

// The mass of a US gallon of Jet A-1 at some ``standard''
// temperature [kg].
inline double constexpr us_gallon_jet_a1() { return us_gallon() * density_jet_a1() ; }

// US gallon Avgas per hour [kg/s].
inline double constexpr us_gallon_avgas_per_hour()
{ return us_gallon_avgas() / hour() ; }

// US gallon Jet A-1 per hour [kg/s].
inline double constexpr us_gallon_jet_a1_per_hour()
{ return us_gallon_jet_a1() / hour() ; }

template< int kg = 0 , int m = 0 , int s = 0 , int K = 0 , typename T = double >
struct physical_quantity {

  explicit physical_quantity( T const& value ) : value( value ) {}
  operator T() const { return value ; } 

private:

  T value ; 

} ;



template< int kg , int m , int s , int K , typename T >
physical_quantity< kg , m , s , K , T >
operator+( physical_quantity< kg , m , s , K , T > const& a ,
           physical_quantity< kg , m , s , K , T > const& b
	 )
{ return a + b ; }


template< int kg1 , int m1 , int s1 , int K1 , 
          int kg2 , int m2 , int s2 , int K2 , 
	  typename T
	>
physical_quantity< kg1 + kg2 , m1 + m2 , s1 + s2 , K1 + K2 >
operator*( 
  physical_quantity< kg1 , m1 , s1 , K1 , T > const& a ,
  physical_quantity< kg2 , m2 , s2 , K2 , T > const& b
)
{ return a * b ; }


typedef physical_quantity< 0 , 1 , -1 , 0 , double > velocity ;
typedef physical_quantity< 0 , 1 ,  0 , 0 , double > length   ;
typedef physical_quantity< 0 , 0 ,  1 , 0 , double > time     ;

// time t ; speed v ;
// t * v ; // length

} // namespace units

} // namespace cpl


#endif // CPP_LIB_UNITS_H
