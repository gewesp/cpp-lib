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
// Component: REGISTRY
//

#ifndef CPP_LIB_REGISTRY_H
#define CPP_LIB_REGISTRY_H

#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <limits>
#include <stdexcept>
#include <typeinfo>
#include <iterator>

#include <cassert>
#include <cstdlib>

#include "boost/any.hpp"

#include "cpp-lib/util.h"


namespace cpl {

namespace detail_ {

//
// Throw std::runtime_error with "should have " + n + " " + thing
//

void throw_should_have( long const n , std::string const& thing ) ;

}

namespace util {

// Registry and related functions.

//
// Lexer tokens.
//
// Closing symbols must immediately follow their opening counterparts,
// e.g. RB to LB.
//
// When changing this table, be sure also to change token_name_table
// and possibly char_token() in registry.cpp!  Make sure you add handling to
// get_token().
//

enum token {

  // End of input.
  END = 0 ,
  // (
  LP ,
  // )
  RP ,
  // {
  LB ,
  // }
  RB ,
  // [
  LBRACKET ,
  // ]
  RBRACKET ,
  // *
  ASTERISKS ,
  // /
  SLASH ,
  // #
  HASH ,
  // %
  PERCENT ,
  // =
  ASSIGN ,
  // ,
  COMMA ,
  // "
  DOUBLE_QUOTE ,
  // '
  SINGLE_QUOTE ,
  // Quoted string.
  STRING ,
  // Identifier (string of [a-z0-9_] starting with a letter).
  IDENT ,
  // Floating point value.
  NUM ,
  // Last enum value marker.
  NO_TOKEN

} ;


//
// Closing token for a given opening token.  Must immediately follow in the
// enum, cf. note above.
//

inline token closing_token( token t )
{ return static_cast< token > ( static_cast< int >( t ) + 1 ) ; }


//
// Comment styles for the lexer.
//

enum comment_style_t {

  // C style (// till end of line or /* ... */ multiline)
  c_comments ,

  // Shell/Perl style (# till end of line)
  hash_comments ,

  // Matlab/LaTeX style (% till end of line)
  percent_comments

} ;


//
// Lexer style to determine if strings are delimited by single (') or 
// double (") quotes.
//

enum string_quote_style_t {

  single_quote , 
  double_quote

} ;


//
// List style for parser.  comma_required means that list
// elements must be separated by commas.  For comma_optional, the comma
// is optional.
//

enum comma_style_t {

  comma_required ,
  comma_optional

} ;


//
// A struct defining how a lexer will behave.
//

struct lexer_style_t {

  lexer_style_t() 
  :      comment_style( hash_comments ) , 
    string_quote_style( double_quote  ) 
  {}

  lexer_style_t( comment_style_t const cs )
  : comment_style( cs ) , string_quote_style( double_quote ) {}

  lexer_style_t( comment_style_t const cs , string_quote_style_t const sq )
  :      comment_style( cs ) , 
    string_quote_style( sq ) 
  {}

       comment_style_t      comment_style ;
  string_quote_style_t string_quote_style ;

} ;


//
// A struct defining how a parser will behave.
//

struct parser_style_t {

  parser_style_t()
  : comma_style( comma_required ) ,
    list_delimiter( LB ) ,
    expression_delimiter( LP )
  {}

  parser_style_t( comma_style_t const cs )
  : comma_style( cs ) , list_delimiter( LB ) , expression_delimiter( LP ) {}

  parser_style_t( 
    comma_style_t const  cs ,
    token         const  ld ,
    token         const  ed
  )
  : comma_style   ( cs ) ,
    list_delimiter( ld ) ,
    expression_delimiter( ed )
  {}

  comma_style_t comma_style ;
  // Lists are (list_delimiter) (elements) (corresponding closing delimiter)
  token list_delimiter ;
  // Expressions are (expression head) (expression_delimiter) (elements)
  // (corresponding closing delimiter)
  token expression_delimiter ;

} ;

//
// Combination of lexer and parser styles.
//

struct grammar {

  grammar() {}

  grammar( 
    parser_style_t const& parser_style ,
    lexer_style_t  const&  lexer_style
  ) 
  : parser_style( parser_style ) ,
     lexer_style(  lexer_style )
  {}

  parser_style_t parser_style ;
   lexer_style_t  lexer_style ;

} ;

//
// A grammar for simple parsing of matlab style files.
//
// This grammar uses percent style comments, bracket-delimited lists,
// optional commas and single-quoted strings.
//

grammar const matlab_style() ;


//
// A grammar using the following characteristics:
//
// C-style comments, brace-delimited lists, optional commas and 
// double-quoted strings.
//

grammar const config_style() ;


//
// The lexer class.  
//
// A lexer reads characters from an input stream whilst removing comments
// and digesting them into a sequence of tokens with associated values.  It
// keeps track of file name and line number.  Errors occur when the input
// isn't lexically correct and are reported via std::runtime_error
// exceptions containing the location of the error.
//

struct lexer {

  /// Read from stream.  Set file name.

  /// The file name is only used for error reporting.

  lexer( 
    std::istream&                              , 
    std::string   const&    = "(unknown file)" ,
    lexer_style_t const& ls = lexer_style_t()
  ) ;


  // Set comment style.

  void set_style( lexer_style_t const& ls )
  { style = ls ; }


  // Reads and returns next token, consuming it.
  token get_token() ;

  // Reads and returns next token but doesn't consume it.
  token peek_token() ;

  /// \return Current token.  Don't advance on input.

  token current_token() const { return current_token_ ; }

  /// \return String value of current token if it is a quoted string or
  /// identifier.  We assert() that current_token() == cpl::util::STRING.

  std::string const& string_value() const
  { assert( current_token() == STRING || current_token() == IDENT ) ;
    return string_value_ ; }

  /// \return Numeric value of current token if it is a number.

  double num_value() const
  { assert( current_token() == NUM ) ; return num_value_ ; }


  /// Put back the current token into input.

  /// Actually this is implemented by a hold flag which is checked by
  /// get_token() in order to decide whether or not to advance on input.
  /// For the user the putback() function works analogously to the
  /// std::istream function of the same name.
  /// \return *this.

  lexer& putback() { hold = true ; return *this ; }

  /// \return The lexer state.

  /// The state is manipulated by the extraction operators, in analogy
  /// to std::istream.

  operator bool() const { return state ; }

  /// Set state.

  /// \return *this.

  lexer& set_state( bool s ) { state = s ; return *this ; }

  /// \return Current line number.

  std::size_t line_no() const { return line_no_ ; }

  /// \return File name.

  std::string filename() const { return filename_ ; }


  /// \return A string of the form ``<file name>:<line number>: '' indicating
  /// the current location in the source stream.

  std::string location() const ;

private:

  // The source stream.

  std::istream& is ;

  // How to lex...

  lexer_style_t style ;

  char quote_char() const
  { return single_quote == style.string_quote_style ? '\'' : '"' ; }
    
  char comment_char() const
  { return hash_comments == style.comment_style ? '#' : '%' ; }

  // The current token.

  token current_token_ ;


  /// The string value of the current token.

  std::string string_value_ ;

  /// The numeric value of the current token.

  double num_value_ ;


  /// The current line number.

  std::size_t line_no_ ;

  /// The file name.

  std::string filename_ ;

  /// For get_next_token(): Advance on input? (Don't if hold is true.)

  bool hold ;

  /// Lexer state.  True means good.

  bool state ;

  /// Not implemented.  Lexers can't be copied.

  lexer( lexer const& ) ;

  /// Not implemented.  Lexers can't be copied.

  lexer& operator=( lexer const& ) ;

  // Munch a C-style multiline comment.
 
  void munch_till_asterisks_slash() ;

} ;

/// \defgroup lexer_functions Lexer related functions.
///
//@{

/// Get next token using cpl::util::expect() and try to extract a double.

/// A std::runtime_error is thrown if the operation fails.
/// \return The extracted value.

double get_double( lexer& ) ;

/// Call cpl::util::get_double() and check that the value is nonnegative.

/// An exception is thrown if the operation fails or
/// if the extracted value is negative.  This
/// function can be used for preliminary checking of values at parsing
/// time.
/// \return The extracted value.

double get_nonneg( lexer& ) ;

/// Call cpl::util::get_double() and check that the value is positive.

/// An exception is thrown if the operation fails or
/// if the extracted value is <= 0.  This
/// function can be used for preliminary checking of values at parsing
/// time.
/// \return The extracted value.

double get_positive( lexer& ) ;


/// Check that the next token is the given one.

/// \param t The expected token.
/// \param get_next If true, check against get_token().  Otherwise,
/// check against current_token().
/// If the read token is unequal to t, throw a std::runtime_error
/// containing an appropriate message.

void expect( lexer& , token t , bool get_next = true ) ;


/// Check that the next token is the given identifier.

/// \param s The expected identifier.
/// \param get_next If true, check against get_token().  Otherwise,
/// check against current_token().
/// If the token read is unequal to cpl::util::IDENT or the identifier is
/// unequal to the given one, throw a std::runtime_error
/// containing an appropriate message.

void expect( lexer& , std::string const& s , bool get_next = true ) ;


/// Read sequence of whitespace-delimited numerical values into given range.

/// \a for_it must be a forward iterator.  Values are read and \a begin is
/// increased until \a end is reached.

template< typename for_it >
void read_numbers( lexer& , for_it begin , for_it end ) ;

//@}


/// Key type used in registry.

typedef std::string key_type ;


//
// Expression of the form head( tail_0, ..., tail_{n - 1}), with the
// appropriate delimiters in place of the parentheses above.
//

struct expression {

  std::string head ;
  std::vector< boost::any > tail ;

} ;

// 
// A parser used for reading key/value pairs. 
//

struct parser {

  parser( lexer& lex , parser_style_t const& ps = parser_style_t() )
  : lex  ( lex  ) , 
    style( ps   ) ,
    state( true ) 
  {}

  // Parse a key/expression pair.  Expression is returned in boost::any,
  // definition location in line_no and filename.
  parser& parse_pair(
    key_type& ,
    boost::any& ,
    std::size_t& line_no ,
    std::string& filename
  ) ;

  // Parse a term.
  parser& parse_term( boost::any& ) ;

  // Parse a list until the given closing token.
  std::vector< boost::any > parse_list( token close ) ;

  operator bool() const { return state ; }

private:

  token list_open_token () const
  { return style.list_delimiter ; }
  token list_close_token() const
  { return closing_token( style.list_delimiter ) ; }

  token expression_open_token () const
  { return style.expression_delimiter ; }
  token expression_close_token() const
  { return closing_token( style.expression_delimiter ) ; }

  lexer& lex ;

  parser_style_t style ;

  bool state ;

} ;


//
// A registry is a store of key/value pairs.  It provides access via
// getter and checker / functions, addition of pairs, and read of a
// complete registry from / an ASCII file or a lexer.  Keys are arbitrary
// std::string's.  Values may have the types
//
//  - Number
//  - String
//  - Identifier
//  - List
//  - Expression
//
// A list is an arbitraryly nested list of terms which, in turn,
// may be doubles, strings, identifiers, lists, or expressions.
//
// Depending on the list_style parameter in the constructor, list
// and expression elements must be separated by commas (comma_required) or not
// (comma_optional).
//

struct registry {

  virtual ~registry() {}

  /// Constructors

  /// Creates an empty registy.  Usually followed by read_from() calls.
  registry() {}

  /// Reads from named file with given (default) grammar.
  registry(std::string const& filename            ,
           grammar const& g = grammar()           ,
           bool const throw_on_redefinition = true) {
    read_from( filename , g , throw_on_redefinition ) ;
  }

  /// Reads from an istream
  registry(std::istream& is                       ,
           grammar const& g = grammar()           ,
           bool const throw_on_redefinition = true) {
    lexer lex( is , "(unknown file)", g.lexer_style ) ;
    read_from( lex , g , throw_on_redefinition ) ;
  }

  /// Read from a lexer.

  /// Scans for key/value pairs using parser::parse_pair().  If no key is
  /// found, the last token is kept in the lexer using putback() so that
  /// it can be reused in subsequent parsing of the same lexer.
  ///
  /// This is useful if, for example, the key/value pairs form only a
  /// section of a file.
  /// \param throw_on_redefinition If true, throw a std::runtime_error
  /// containing an appropriate message in case an already existing key
  /// is redefined.

  void read_from( 
    lexer& source                                         ,
     lexer_style_t const&  lexer_style =  lexer_style_t() ,
    parser_style_t const& parser_style = parser_style_t() ,
    bool throw_on_redefinition         = true 
  ) ;


  // Read from a named file.

  // Scans for key/value pairs using parser::parse_pair().  If no key is
  // found, the token must be END, i.e., we parse the complete file.
  //
  // This function opens the file, creates a lexer with the
  // filename argument and then delegates to
  // read_from( lexer& ).
  //

  void read_from(
    std::string const& name                               ,
     lexer_style_t const&  lexer_style =  lexer_style_t() ,
    parser_style_t const& parser_style = parser_style_t() ,
    bool throw_on_redefinition         = true 
  ) ;

  //
  // Convenience read_from() functions with combined lexer and parser style,
  // forwarding to the above.
  //

  void read_from( 
    lexer&         source ,
    grammar const& g      ,
    bool const throw_on_redefinition = true
  )
  { read_from
    ( source , g.lexer_style , g.parser_style , throw_on_redefinition ) ; }
  
  void read_from( 
    std::string const& name ,
    grammar     const& g    ,
    bool const throw_on_redefinition = true
  )
  { read_from
    ( name , g.lexer_style , g.parser_style , throw_on_redefinition ) ; }



  /// Add key/wrapped value/defined_at triple.

  /// \param key The key.
  /// \param val The value.
  /// \param defined_at A string indicating where the value was defined.
  /// \param throw_on_redefinition If true, throw a std::runtime_error
  /// containing an appropriate message in case an already existing key
  /// is redefined.

  void add_any(
    key_type const& key ,
    boost::any const& val ,
    std::string const& defined_at ,
    bool throw_on_redefinition = true
  ) ;


  //
  // Lookup a value of type T.  If key is not defined, return
  // default_value.
  // If the stored value has a type differing from T,
  // throw a std::runtime_error containing an appropriate message.
  //
  // Forwards to a private template, TODO: Maybe a solution with
  // type inference is possible?
  //

  std::string const& get_default( 
      key_type const& key , std::string const& default_value ) const 
  { return get_default_internal( key , default_value ) ; }

  // Need to catch const char* here, otherwise it gets converted
  // to bool.
  // Also, needs to return by value to avoid returning
  // a reference to temporary!
  std::string get_default( 
      key_type const& key , char const* const default_value ) const {
    if( !is_defined( key ) ) {
      return default_value ;
    } else {
      return get< std::string >( key ) ;
    }
  }

  double     const& get_default( 
      key_type const& key , double      const& default_value ) const 
  { return get_default_internal( key , default_value ) ; }

  std::vector< double > const& get_default( 
      key_type const& key , std::vector< double > const& default_value ) const 
  { return get_default_internal( key , default_value ) ; }

  long get_default( 
      key_type const& key , long default_value ,
      double const& min = std::numeric_limits< long >::min() ,
      double const& max = std::numeric_limits< long >::max() ) const ;

  std::vector< std::string > const& get_default( 
      key_type const& key , std::vector< std::string > const& default_value ) const 
  { return get_default_internal( key , default_value ) ; }

  bool get_default( 
      key_type const& key , bool default_value ) const ;


  //
  // Lookup a value of type T.
  //
  // If the given key doesn't exist or the stored value
  // has a type differing from T,
  // throw a std::runtime_error containing an appropriate message.
  // \return The stored value.
  // \param key The key.
  //
  // NOTE.  In templates it may be necessary to call this function with
  // the object.template f<>() syntax!
  //

  template< typename T > T const& get( key_type const& key ) const ;


  /// Lookup by key and return the boost::any.

  boost::any const& get_any( key_type const& key ) const ;


  /// Look up and return a positive numerical value.

  /// If the given key doesn't exist or the stored value has
  /// a type differing from double or the stored double value is
  /// less than or equal to zero,
  /// throw a std::runtime_error containing an appropriate message.
  /// \return The stored value.
  /// \param key The key.

  double const& check_positive( key_type const& key ) const ;


  /// Look up and return a nonnegative numerical value.

  /// If the given key doesn't exist or the stored value has
  /// a type differing from double or the stored double value is
  /// less than zero,
  /// throw a std::runtime_error containing an appropriate message.
  /// \return The stored value.
  /// \param key The key.

  double const& check_nonneg( key_type const& key ) const ;

  //
  // Call get< double >() and check that the value is an integer
  // between min and max.
  //

  long check_long( 
    key_type const& ,
    double const& min = std::numeric_limits< long >::min() ,
    double const& max = std::numeric_limits< long >::max() 
  ) const ;
    
  //
  // Call get< double >() and check that the value is a valid
  // TCP port value, i.e. it is an integer between 0 and 2^16 - 1.
  //

  long check_port( key_type const& ) const ;

  std::string const& check_string( key_type const& key ) const {
    return get<std::string>(key);
  }

  /// Use the respective convert() function to check that
  /// the stored value is a list of lists containing doubles.

  std::vector< std::vector< double > > const
  check_vector_vector_double
  ( key_type const& key , long const rows = -1 , long const columns = -1 )
  const ;


  // Use the respective convert() function to check that the
  // stored value is a list.
  //
  // If n >= 0, check that the list has n elements.

  std::vector< boost::any > const& 
  check_vector_any( key_type const& key , long const n = -1 ) const ;


  // Use the respective convert() function to check that
  // the stored value is a list containing doubles.

  std::vector< double > const
  check_vector_double( key_type const& key , long n = -1 ) const ;


  // Use the respective convert() function to check that
  // the stored value is a list containing strings.

  std::vector< std::string > const
  check_vector_string( key_type const& key , long n = -1 ) const ;


  /// Look up and return a boolean value.

  /// If the given key doesn't exist or the stored value
  /// is different from true or false,
  /// throw a std::runtime_error containing an appropriate message.
  /// \return The stored value.
  /// \param key The key.

  bool check_bool( key_type const& key ) const ;


  /// Throw if key already exists.

  /// If the given key already exists in the registry, throw a
  /// std::runtime_error containing the message "redefinition of <key>".
  /// \param key The key.

  void check_key( key_type const& key ) ;

  /// Check if key exists.

  /// \return True if the given key exists in the registry.
  /// \param key The key.

  bool is_set( key_type const& key ) const { return kv_map.count( key ) != 0 ; }
  bool is_defined( key_type const& key ) const { return is_set( key ) ; }


  // Check if key exists and is set to the boolean value true.

  bool is_set_and_true( key_type const& key ) const
  { return is_set( key ) && check_bool( key ) ; }


  /// Return a message of the form "(line n of <file>)" indicating where
  /// \a key was defined.  If \a key is not defined, throw
  /// an exception.

  std::string const& defined_at( key_type const& key ) const ;


  /// Return a message of the form "key <key> (line n of <file>)"
  /// indicating where \a key was defined.  Calls
  /// defined_at().

  std::string const key_defined_at( key_type const& key ) const ;   

  /// Write in the same format read_from() expects.

  /// \sa read_from().

  friend std::ostream& operator<<( std::ostream& , registry const& ) ;

  //
  // Erase all entries.
  //

  void clear() { kv_map.clear() ; }


  // Return file name of last read_from call with file name.
  std::string const last_filename() const { return filename ; }


private:

  struct mapped { boost::any value ; std::string defined_at ; } ;

  // The key/value map type.  Stores key, value, and location of
  // (for error messages).

  typedef std::map< key_type , mapped > map_type ;


  // The key/value map.

  map_type kv_map ;

  // Last read from file (if known).
  std::string filename ;

  template< typename T > T const& get_default_internal(
      key_type const& key ,
      T const& default_value ) const ;

} ;


//
// Get a reference to the value contained in \a a.  Throw an exception
// if the type does not match \a T.
//

template< typename T > T const& convert( boost::any const& a ) ;

//
// Check that \a a is a list of lists containing doubles.
// If \a rows >= 0, the outer list must have \a rows entries.
// If \a columns >= 0, all inner lists must have \a columns entries.
// If \a columns == -2, all inner lists must have the same number of
// entries.
// \retval v Contains the list if conversion was succesful.
//

void convert(
  boost::any const& a ,
  std::vector< std::vector< double > >& v ,
  long const rows = -1 ,
  long const columns = -1
) ;


//
// Check that a is a list containing T's.
// If n >= 0, require that the list have \a n entries.
// The parameter ret will contain the list if conversion was succesful.
// Requires: n >= -1.
//

template< typename T >
void convert(
  boost::any const& a ,
  std::vector< T >& ret ,
  long const n = -1
) {

  always_assert( n >= -1 ) ;

  std::vector< boost::any > const& v =
    convert< std::vector< boost::any > >( a ) ;

  if( n >= 0 && v.size() != static_cast< unsigned long >( n ) )
  { cpl::detail_::throw_should_have( n , "element(s)" ) ; }

  ret.resize( v.size() ) ;

  if( n >= 0 ) {

    assert(   v.size() == static_cast< unsigned long >( n ) ) ;
    assert( ret.size() == static_cast< unsigned long >( n ) ) ;

  }

  for( unsigned long i = 0 ; i < ret.size() ; ++i ) {

    try
    { ret[ i ] = convert< T >( v[ i ] ) ; }
    catch( std::runtime_error const& e ) {

      throw std::runtime_error
      ( "element " + cpl::util::string_cast( i + 1 ) + e.what() ) ;

    }

  }

  if( n >= 0 )
  { assert( ret.size() == static_cast< unsigned long >( n ) ) ; }

}


// Write in the same format read_from() expects.

std::ostream& operator<<( std::ostream& , registry const& reg ) ;

namespace detail_ {

//
// Human readable description of a value type.
//
// TODO: Use some less sophisticated way than function template
// specialization here...
//

template< typename T >
inline std::string const hr_type()
{ always_assert( !"implement this type!" ) ; return std::string() ; }

template<>
inline std::string const hr_type< double >()
{ return "number"     ; }

template<>
inline std::string const hr_type< std::string >()
{ return "string"     ; }

template<>
inline std::string const hr_type< cpl::util::expression >()
{ return "expression" ; }

template<>
inline std::string const hr_type< std::vector< boost::any > >()
{ return "list"       ; }

template<>
inline std::string const hr_type< std::vector< double > >()
{ return "list of double" ; }

template<>
inline std::string const hr_type< std::vector< std::vector< boost::any > > >()
{ return "nested list" ; }

template<>
inline std::string const hr_type< std::vector< std::vector< double > > >()
{ return "list of list of double" ; }

template<>
inline std::string const hr_type< std::vector< std::string > >()
{ return "list of strings" ; }

} // end namespace detail_

} // end namespace util

} // end namespace cpl



template< typename T >
T const& cpl::util::convert( boost::any const& a ) {

  T const* ret = boost::any_cast< T >( &a ) ;

  if( !ret )
  { throw std::runtime_error( "should be a " + cpl::util::detail_::hr_type< T >() ) ; }

  return *ret ;

}

template< typename T >
T const& cpl::util::registry::get_default_internal(
    key_type const& key , T const& default_value ) const {

  if( !is_defined( key ) ) {
    return default_value ;
  } else {
    return get< T >( key ) ;
  }

}


template< typename T >
T const& cpl::util::registry::get( key_type const& key ) const {

  boost::any const& a = get_any( key ) ;

  try {

    T const& ret = convert< T >( a ) ;
    return ret ;

  } catch( std::runtime_error const& e ) {

    throw std::runtime_error
    ( key + " " + defined_at( key ) + ": " + e.what() ) ;

  }

}


template< typename for_it >
void cpl::util::read_numbers
( cpl::util::lexer& lex , for_it begin , for_it end ) {

  typedef typename std::iterator_traits< for_it >::value_type value_type ;

  while( begin != end ) {

    const double x = get_double( lex ) ;
    *begin = static_cast< value_type >( x ) ;
    ++begin ;

  }

}


#endif // CPP_LIB_REGISTRY_H
