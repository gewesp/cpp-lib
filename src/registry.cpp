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


#include <cctype>
#include <fstream>
#include <sstream>
#include <sstream>
#include <string>

#include <utility>
#include <memory>

#include <cassert>

#include "cpp-lib/registry.h"
#include "cpp-lib/util.h"


using namespace cpl::util    ;
using namespace cpl::detail_ ;


namespace {

//
// According to Josuttis, 13.9.1, when opening a file in text mode
// (i.e., no std::ios_base::binary), cr/nl characters are replaced by \n
// and vice versa.  Windows compilers indeed seem to do that.
//

inline bool isnewline( char const c ) { return '\n' == c ; }


char const* const token_name_table[] = {

  "end of file"         ,
  "left parenthesis"    ,
  "right parenthesis"   ,
  "left brace"          ,
  "right brace"         ,
  "left bracket"        ,
  "right bracket"       ,
  "`*'"                 ,
  "`/'"                 ,
  "`#'"                 ,
  "`%'"                 ,
  "`='"                 ,
  "comma"               ,
  "double quote"        ,
  "single quote"        ,
  "character string"    ,
  "identifier"          ,
  "number"

} ;


token char_token( char const c ) {

  switch( c ) {

  case '(' : return LP         ;
  case ')' : return RP         ;
  case '{' : return LB         ;
  case '}' : return RB         ;
  case '[' : return LBRACKET   ;
  case ']' : return RBRACKET   ;
  case '/' : return SLASH      ;
  case '#' : return HASH       ;
  case '%' : return PERCENT    ;
  case ',' : return COMMA      ;
  case '=' : return ASSIGN     ;
  case '*' : return ASTERISKS  ;

  // TODO:  make this a lexer member and add line number to error msg.
  default :
    throw
      std::runtime_error( std::string( "unknown input character: " ) + c ) ;

  }

}


std::string token_name( token const t ) {

  always_assert
  ( size( token_name_table ) == static_cast< unsigned long >( NO_TOKEN ) ) ;

  always_assert( t < NO_TOKEN ) ;

  return token_name_table[ static_cast< int >( t ) ] ;

}


std::string defined_at_msg(
  std::size_t line_no ,
  std::string const& filename
) {

  std::ostringstream os ;
  os << "(line " << line_no << " in " << filename << ")" ;
  return os.str() ;

}

} // end anon namespace


void cpl::detail_::throw_should_have
( long const n , std::string const& thing ) {

  throw std::runtime_error
  ( "should have " + string_cast( n ) + " " + thing ) ;

}


void cpl::util::registry::check_key( const key_type& key ) {

  if( kv_map.count( key ) )
  { throw std::runtime_error
    ( key + " redefined " + defined_at( key ) ) ; }

}


std::any const& 
cpl::util::registry::get_any( key_type const& key ) const {

  map_type::const_iterator i = kv_map.find( key ) ;

  if( i == kv_map.end() )
  { throw std::runtime_error( "key " + key + ": not defined" ) ; }

  return i->second.value ;

}


std::string const& 
cpl::util::registry::defined_at( key_type const& key ) const {

  map_type::const_iterator i = kv_map.find( key ) ;

  if( i == kv_map.end() )
  { throw std::runtime_error( "key " + key + ": not defined" ) ; }

  return i->second.defined_at ;

}


std::string const
cpl::util::registry::key_defined_at( key_type const& key ) const 
{ return "key " + key + " " + defined_at( key ) ; }




void cpl::util::registry::add_any(
  key_type const& key ,
  std::any   const& value ,
  std::string  const& defined_at ,
  bool throw_on_redefinition
) {

  if( throw_on_redefinition ) { check_key( key ) ; }
  kv_map[ key ].value      = value      ;
  kv_map[ key ].defined_at = defined_at ;

}



double const& 
cpl::util::registry::check_positive( key_type const& key ) const {

  double const& ret = get< double >( key ) ;

  if( ret <= 0 )
  { throw std::runtime_error( key_defined_at( key ) + ": should be > 0" ) ; }

  return ret ;

}


double const& 
cpl::util::registry::check_nonneg( key_type const& key ) const {

  double const& ret = get< double >( key ) ;

  if( ret < 0 )
  { throw std::runtime_error( key_defined_at( key ) + ": should be >= 0" ) ; }

  return ret ;

}


long 
cpl::util::registry::check_long( key_type const& key , double const& min , double const& max ) const {

  double const d = get< double >( key ) ;

  try { return ::cpl::util::check_long( d , min , max ) ; }
  catch( std::runtime_error const& e ) {

    throw std::runtime_error( 
        key 
      + " " 
      + defined_at( key ) 
      + ": "
      + e.what() 
	) ;

  }

}


long
cpl::util::registry::check_port( key_type const& key ) const {

  double const& d = get< double >( key ) ;

  try { return ::cpl::util::check_long( d , 0 , 65535 ) ; } 
  catch( std::runtime_error const& e ) {

    throw std::runtime_error( 
        key 
      + " " 
      + defined_at( key ) 
      + ": not a valid TCP/UDP port value: " 
      + e.what() 

    ) ;

  }

}


std::vector< std::vector< double > > const
cpl::util::registry::check_vector_vector_double(
  key_type const& key ,
  long const rows ,
  long const columns
) const {

  std::vector< std::vector< double > > ret ;

  std::any const& a = get_any( key ) ;

  try { convert( a , ret , rows , columns ) ; }
  catch( std::runtime_error const& e )
  { throw std::runtime_error( key_defined_at( key ) + ": " + e.what() ) ; }

  return ret ;

}


void cpl::util::convert(
  std::any const& a ,
  std::vector< std::vector< double > >& ret ,
  long const rows ,
  long const columns 
) {

  assert( rows >= -1 ) ;
  assert( columns >= -2 ) ;

  std::vector< std::any > const& v =
    convert< std::vector< std::any > >( a ) ;

  if( rows >= 0 && v.size() != static_cast< unsigned long >( rows ) ) 
  { throw_should_have( rows , "row(s)" ) ; }

  ret.resize( v.size() ) ;

  for( unsigned long i = 0 ; i < ret.size() ; ++i ) {

    try {

      convert( v[ i ] , ret[ i ] , -1 ) ;

      if(
	     ( columns >= 0
	       && ret[ i ].size() != static_cast< unsigned long >( columns ) )
	  ||
	     ( columns == -2
	       && i >= 1 && ret[ i ].size() != ret.front().size() )
	) {

	long const n =
	  ( columns == -2 ) ? ret.front().size() : columns ;

	throw_should_have( n , "column(s)" ) ;

      }

    } catch( std::runtime_error const& e ) {

      std::ostringstream os ;
      os << "row " << i + 1 << ": " << e.what() ;
      throw std::runtime_error( os.str() ) ;

    }

    if( columns >= 0 ) 
    { assert( ret[ i ].size() == static_cast< unsigned long >( columns ) ) ; }

    if( columns == -2 && i >= 1 )
    { assert( ret[ i ].size() == ret[ 0 ].size() ) ; }

  }

  if( rows >= 0 )
  { assert( ret.size() == static_cast< unsigned long >( rows ) ) ; }

}


std::vector< std::string > const
cpl::util::registry::check_vector_string
( key_type const& key , long n ) const {

  std::any const& a = get_any( key ) ;
  
  std::vector< std::string > ret ;

  try { convert( a , ret , n ) ; }
  catch( std::runtime_error const& e ) 
  { throw std::runtime_error( key_defined_at( key ) + ": " + e.what() ) ; }

  return ret ;

}


std::vector< double > const
cpl::util::registry::check_vector_double
( key_type const& key , long n ) const {

  std::any const& a = get_any( key ) ;
  
  std::vector< double > ret ;

  try { convert( a , ret , n ) ; }
  catch( std::runtime_error const& e ) 
  { throw std::runtime_error( key_defined_at( key ) + ": " + e.what() ) ; }

  return ret ;

}

std::vector< std::any > const&
cpl::util::registry::check_vector_any
( key_type const& key , long const n ) const {

  assert( n >= -1 ) ;

  try {

    std::vector< std::any > const& ret = 
      convert< std::vector< std::any > >( get_any( key ) ) ;

    if( n >= 0 && ret.size() != static_cast< unsigned long >( n ) ) 
    { throw_should_have( n , "element(s)" ) ; }

    return ret ;

  } catch( std::runtime_error const& e ) 
  { throw std::runtime_error( key_defined_at( key ) + ": " + e.what() ) ; }

  // unreachable, hence no return.

}


bool cpl::util::registry::check_bool( key_type const& key ) const {

  std::string const& s = get< std::string >( key ) ;

  if( s == "true"  ) return true ;
  if( s == "false" ) return false ;

  throw std::runtime_error
    ( key_defined_at( key ) + ": should be true or false" ) ;

}

bool cpl::util::registry::get_default( key_type const& key , 
                                       bool const default_value ) const {
  if( !is_defined( key ) ) {
    return default_value ;
  } else {
    return check_bool( key ) ;
  }
}

long cpl::util::registry::get_default( 
    key_type const& key , 
    long const default_value ,
    double const& min , 
    double const& max ) const {
  if ( !is_defined( key ) ) {
    return default_value ; 
  } else {
    return check_long( key , min , max ) ;
  }
}

void cpl::util::registry::read_from( 
  lexer& lex                 , 
   lexer_style_t const& ls   ,
  parser_style_t const& ps   ,
  bool throw_on_redefinition
) {

  lex.set_style( ls ) ;
  
  parser p( lex , ps ) ;

  key_type key ;
  std::any   val ;

  std::size_t line_no  ;
  std::string filename ;

  while( p.parse_pair( key , val , line_no , filename ) ) {

    try {
      add_any(
	key ,
	val ,
	defined_at_msg( line_no , filename ) ,
	throw_on_redefinition
      ) ;
    }

    catch( std::runtime_error const& e )
    { throw std::runtime_error( lex.location() + e.what() ) ; }

  }

  lex.putback() ;

}


void cpl::util::registry::read_from(
  std::string const& filename_ ,
   lexer_style_t const& ls    ,
  parser_style_t const& ps    ,
  bool throw_on_redefinition
) {

  auto is = cpl::util::file::open_read( filename_.c_str() ) ;
  lexer lex( is , filename_ ) ;
  read_from( lex , ls , ps , throw_on_redefinition ) ;

  expect( lex , END ) ;

  filename = filename_ ;

}


std::ostream& cpl::util::operator<<
( std::ostream& os , registry const& ) {

  os << "*** FIXME: registry output not yet implemented." << std::endl ;
  return os ;

}


cpl::util::grammar const cpl::util::config_style() {

  return grammar( 
	parser_style_t( comma_optional , LB , LP ) ,
	lexer_style_t( c_comments , double_quote )
  ) ;

}


cpl::util::grammar const cpl::util::matlab_style() {

  return grammar( 
	parser_style_t( comma_optional , LBRACKET , LP ) ,
	lexer_style_t( percent_comments , single_quote )
  ) ;

}


cpl::util::lexer::lexer
( std::istream& is , std::string const& filename , lexer_style_t const& s ) :
  is( is ) ,
  style( s ) ,
  current_token_( END ) ,
  num_value_( 0 ) ,
  line_no_( 1 ) ,
  filename_( filename ) ,
  hold( false ) ,
  state( true )
{}

token cpl::util::lexer::peek_token() {
  if (hold) {
    return current_token();
  } else {
    const token ret = get_token();
    putback();
    return ret;
  }
}

token cpl::util::lexer::get_token() {

  if( hold ) {

    hold = false ;
    return current_token() ;

  }


  char c;

restart:

  while( is.get( c ) && std::isspace( c ) )
  { line_no_ += isnewline( c ) ; }

  if( is.eof () ) { return current_token_ = END ; }
  if( is.fail() ) { throw std::runtime_error( location() + "read error" ) ; }

  assert( !std::isspace( c ) ) ;

  switch( c ) {

  case '#' :
  case '%' :

    if( style.comment_style == c_comments || comment_char() != c )
    { return current_token_ = char_token( c ) ; }

    // Munch comment till end of line.
    while( is.get( c ) && !isnewline( c ) )
    {}

    if( is.eof() ) return current_token_ = END ;
    ++line_no_ ;
    goto restart;

  case '/' :

    if( is.peek() == std::char_traits< char >::eof() )
    { return current_token_ = SLASH ; }

    if( is.peek() == '/' ) {

      if( style.comment_style == hash_comments )
      { return current_token_ = SLASH ; }

      // In "//" style comment.

      while( is.get( c ) && !isnewline( c ) )
      {}

      if( is.eof() ) return current_token_ = END ;
      ++line_no_ ;

      goto restart ;

    }

    if( is.peek() == '*' ) {

      if( style.comment_style != c_comments )
      { return current_token_ = ASTERISKS ; }

      // In "/* ... */" style comment.

      is.ignore() ;

      munch_till_asterisks_slash() ;
      goto restart ;

    }


  case '=' :
  case '(' :
  case ')' :
  case '{' :
  case '}' :
  case '[' :
  case ']' :
  case ',' :
    return current_token_ = char_token( c ) ;

  // Quoted string...
  case '"'  :
  case '\'' :

    if( quote_char() != c ) { return current_token_ = char_token( c ) ; }
    string_value_.resize( 0 ) ;

    while( 1 ) {

      is.get( c ) ;
      if( !is || isnewline( c ) )
      { throw std::runtime_error( location() + "unterminated string" ) ; }

      if( c == quote_char() ) { break ; }

      if( c == '\\' ) {

        char cc ;
        is.get( cc ) ;
        if( !is || ( quote_char() != cc && 'n' != cc && '\\' != cc ) )
        { throw std::runtime_error( location() + "bad escape sequence" ) ; }

             if( quote_char() == cc ) { c = quote_char() ; }
        else if( 'n'          == cc ) { c = '\n'         ; }
        else if( '\\'         == cc ) { c = '\\'         ; }

      }

      string_value_.push_back( c ) ;

    }

    return current_token_ = STRING ;

  }

  // read a floating point number
  if( std::isdigit( c ) || c == '.' || c == '+' || c == '-' ) {

    is.putback( c ) ;
    is >> num_value_ ;

    if( is.fail() ) {

      throw std::runtime_error
      ( location() + "error reading floating point value" ) ;

    }

    return current_token_= NUM;

  }


  // Read an identifier (must start with a letter; may also contain
  // digits and underscore)

  if( std::isalpha( c ) ) {
    string_value_ = c ;
    while( is.get( c ) && ( std::isalnum( c ) || c == '_' ) ) {
      string_value_.push_back( c );
    }
    if( is ) {
      is.putback( c ) ;
    }
    return current_token_ = IDENT;
  }

  throw std::runtime_error(
    location() + "bad character in input (" + c + ")"
  ) ;

}


void cpl::util::lexer::munch_till_asterisks_slash() {

restart:

  char c ;

  while( is.get( c ) && c != '*' )
  { line_no_ += isnewline( c ) ; }

  if( !is || is.peek() == std::char_traits< char >::eof() ) { return ; }

  if( is.peek() != '/' ) { goto restart ; }

  is.ignore() ;

}


std::string cpl::util::lexer::location() const {

  std::ostringstream os ;
  os << filename() << ":" << line_no() << ": " ;

  return os.str() ;

}


parser& cpl::util::parser::parse_pair(
  key_type& key ,
  std::any  & val ,
  std::size_t & line_no ,
  std::string & filename
) {

  if( !state ) { return *this ; }

  if( lex.get_token() != IDENT ) {
    state = false ;
    return *this ;
  }

  line_no  = lex.line_no () ;
  filename = lex.filename() ;


  key = lex.string_value() ;

  expect( lex , ASSIGN ) ;

  return parse_term( val ) ;

}


std::vector< std::any > parser::parse_list( token const close ) {

  std::vector< std::any > v ;

  bool first = true ;

  while( lex.get_token() != close ) {

    if( !first ) {

      // Parse space between list elements.

      if( style.comma_style == comma_required )
      { expect( lex , COMMA , false ) ; }
      else
      { if( lex.current_token() != COMMA ) { lex.putback() ; } }

    }

    else { lex.putback() ; }

    v.push_back( std::any() ) ;
    parse_term( v.back() ) ;

    first = false ;

  }

  return v ;

}


parser& cpl::util::parser::parse_term( std::any& val ) {

  lex.get_token() ;

  if     ( lex.current_token() == NUM    )
  { val = lex.num_value   () ; }
  else if( lex.current_token() == STRING )
  { val = lex.string_value() ; }
  else if( lex.current_token() == IDENT  ) {

    // Expression head or string value.
    std::string const ident = lex.string_value() ;

    if( lex.get_token() == expression_open_token() ) {

      // expression of form foo(x y z)
      expression e ;
      e.head = ident ;
      e.tail = parse_list( expression_close_token() ) ;
      val = e ;

    } else {

      lex.putback() ;
      val = ident ;
    }

  } else if( lex.current_token() == list_open_token() ) {

    val = parse_list( list_close_token() ) ;

  } else {

    throw std::runtime_error
    ( lex.location() + "number, string, identifier or list expected" ) ;

  }

  return *this ;

}



void cpl::util::expect( lexer& lex , token const t , bool read ) {

  token const tt = read ? lex.get_token() : lex.current_token() ;

  if( tt != t ) {

    throw std::runtime_error
    ( lex.location() + token_name( t ) + " expected" ) ;

  }

}


void cpl::util::expect( lexer& lex , std::string const& s , bool read ) {

  token const tt = read ? lex.get_token() : lex.current_token() ;

  if( tt != IDENT || lex.string_value() != s )
    throw std::runtime_error(
      lex.location() + "identifier expected (" + s + ")"
    ) ;

}


double cpl::util::get_double( lexer& lex ) {

  expect( lex , NUM ) ;
  return lex.num_value() ;

}


double cpl::util::get_nonneg( lexer& lex ) {

  double ret = cpl::util::get_double( lex ) ;

  if( ret < 0 ) throw std::runtime_error(
    lex.location() + "nonnegative number expected"
  ) ;

  return ret ;

}


double cpl::util::get_positive( lexer& lex ) {

  double ret = cpl::util::get_double( lex ) ;

  if( ret <= 0 ) throw std::runtime_error(
    lex.location() + "positive number expected"
  ) ;

  return ret ;

}
