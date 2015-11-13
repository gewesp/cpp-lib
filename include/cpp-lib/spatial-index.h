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
// Component: MATH
//

// #define CPP_LIB_LOG_SPATIAL_INDEX_OPS

#ifndef CPP_SPATIAL_INDEX_H
#define CPP_SPATIAL_INDEX_H

#include "cpp-lib/bg-typedefs.h"

#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>

#include <exception>
#include <stdexcept>

#include <cassert>


namespace cpl {

namespace math {


#if 0
template< typename pair >
struct second_equals {
  // Compare position in ID map
  bool operator()(pair const& v1, pair const& v2) const {
    return v1.second == v2.second;
  }
};
#endif

template< typename T >
struct spatial_index_traits {
  // Define the following function:
  // static point point_from_value(T const& t);
};

//
// A combined ID-based and spatial index.
// Supports 'box' queries and lookup by ID.
//
// Implementation: A map and an rtree, where the rtree holds iterators
// into the map.
//
// Operations:  
// * insert/update ID/point
// * Remove by ID
//
// TODO:
// * Define clean iterator interface
// * Iterator based remove
// 
// Improvement potential:
// * Forward reference from map into tree for faster updates instead
//   of the current remove/reinsert strategy.
//
// Tried improvements:
// * Only compare second element in rtree equality comparison (second_equals).
//   It appears that equality is only used at the last step in a query,
//   see also test2.
// * Using linear<4> or rstar<4> doesn't improve over quadratic<4>.
// * Using quadratic<2> doesn't improve either.
//

template<typename ID, typename T, typename TR = spatial_index_traits<T>,
  typename STRAT = boost::geometry::index::quadratic<4> > struct spatial_index {

  // Public typedefs
  typedef ID    id_type    ;
  typedef T     value_type ;
  typedef TR    traits_type;
  typedef STRAT strategy   ;

  // Functions value_type -> bool
  typedef std::function<bool(value_type const&)> value_predicate;

  // Type for the 'primary' index id_type -> value_type
  // Note: Using unordered_map<> doesn't yield a significant speed
  // improvement.
  typedef std::map<id_type, value_type> id_map;

  // Iterator into primary index.  Used for query() results, see below.
  typedef typename id_map::const_iterator primary_iterator;
  typedef typename id_map::const_iterator const_iterator;
  typedef typename id_map::iterator iterator;

  // Value type for the tree below, containing a back reference into
  // the 'primary' index
  typedef std::pair<point, primary_iterator> tree_element;

  // The tree type used for the spatial index
  typedef boost::geometry::index::rtree<tree_element, STRAT> tree_type;

  // Validate structure, currently size only
  void validate(char const* const msg) const {
    if (tr.size() != map.size()) {
      throw std::logic_error("index validation failed: " + std::string(msg) +
            "; tree: " + boost::lexical_cast<std::string>(tr.size())
          + ", map: " + boost::lexical_cast<std::string>(map.size()));
    }
  }

  // Number of items stored
  long size() const {
    // validate("during size() computation");
    return tr.size();
  }

  bool empty() const {
    return map.empty();
  }

  // Support for traversal of the whole primary index
  // TODO: This is a hack as it allows clients to mess up invariants!
  iterator begin() { return map.begin(); }
  iterator end  () { return map.end  (); }

  const_iterator begin() const { return map.begin(); }
  const_iterator end  () const { return map.end  (); }

  // std::map<> proxy interface
        iterator find(id_type const& id)       { return map.find(id); }
  const_iterator find(id_type const& id) const { return map.find(id); }

  // std::map<>::erase plus remove item from the tree.
  iterator erase(iterator const it) {
    validate("before erase");
    if (map.end() == it) { 
      throw std::runtime_error("erase on end()");
    } else {
      auto const pt = traits_type::point_from_value(it->second);
#ifdef CPP_LIB_LOG_SPATIAL_INDEX_OPS
      std::cout << "erase " << it->first 
                << ' ' 
                << boost::geometry::get<0>(pt) << ' '
                << boost::geometry::get<1>(pt) << std::endl;
#endif // CPP_LIB_LOG_SPATIAL_INDEX_OPS
      tr.remove(std::make_pair(pt, const_iterator{it}));
      return map.erase(it);
    }
  }

  inline static bool true_predicate(value_type const&) {
    return true;
  }

#if 0
  // std::map<>::insert() with the restriction that the element
  // isn't already present, plus insert the element into the tree
  std::pair<iterator, bool> insert(id_type const& id, value_type const& p) {
    // TODO ...
  }
#endif

  // Default for upsert(): Always replace existing elements
  struct default_updater {
    default_updater(value_type const& v_new) : v_new(v_new) {}

    bool update_element(id_type const&, value_type& v_ex) const {
      v_ex = v_new;
      return true;
    }

    value_type const& new_element(id_type const&) const {
      return v_new;
    }

  private:
    value_type const& v_new;
  };

  //
  // Insert/update an element.
  //
  // Returns: A pair (iterator it, bool b) where it is the iterator to the 
  //   inserted or updated element and b is true iff the element was new
  //   or updater.update_element() returned true and hence the element 
  //   was updated.
  //
  // Expects an update handler object with two functions:
  // * value_type updater.new_element(id_type const& id) const;
  //   Provides a new element, and executes any actions (e.g. logging) to 
  //   take on insertion of a new element.
  //   May return a value or a const& to the held value.
  // * bool updater.update_element(
  //                    id_type const& id, 
  //                    value_type& v_existing) const;
  //   Compares v_existing with the new value (which is part of updater's
  //   state) and replaces it if deemed necessary (or parts of it).  
  //
  //  updater.update_element() *must* return true the if 
  //   spatial-index-relevant part of v_existing has been updated.  
  //   It *may* return true otherwise as well.
  //
  // new_element() and update_element() must be const functions of
  // updater, meaning that they shouldn't change its internal state.
  //
  // Typically, the updater will be a short-lived object just used
  // for a single upsert() call, so it should have a light-weight
  // constructor.
  //

  template< typename UPDATER >
  std::pair<iterator, bool> upsert(
      id_type const& id, 
      UPDATER const& updater) {
    validate("before upsert");
    auto const it = map.find(id);
    if (map.end() == it) {
      // New element---the rare case

      // Insert new value into the map and obtain position
      value_type const& p = updater.new_element(id);
      iterator const it_inserted = 
          map.insert(std::make_pair(id, p)).first;
      // Insert value and back reference into tree
      insert_tree(p, it_inserted);
      validate("after insert");
      return std::make_pair(it_inserted, true);
    } else {
      // Update element---the common case.

      // TODO: p_old is only necessary if an update is actually done
      // (OK... This *is* the common case).
      point const p_old = traits_type::point_from_value(it->second);
      if (updater.update_element(it->first, it->second)) {
        // Remove old and re-insert new point into tree; primary update 
        // handled by updater.
        tr.remove(std::make_pair(p_old, const_iterator{it}));
        insert_tree(it->second, it);
        // The below definitely *doesn't* work.
        // insert_tree(p, it);
        validate("after update");
        return std::make_pair(it, true );
      } else {
        return std::make_pair(it, false);
      }
    }
  }

  // Remove item with given id, NOOP if the element wasn't present.
  // Returns true iff the element was present.
  // *** Invalidates iterators ***
  bool erase(id_type const& id) {
    validate("before erase");
    auto const it = map.find(id);
    if (map.end() == it) { 
      return false; 
    } else {
      erase(it);
      return true;
    }
  }

  // Finds the max_results points closest to p and writes their positions in the
  // primary index to the output iterator oit.  I must be an iterator to
  // primary_iterator, e.g. std::back_inserter<std::vector<primary_iterator> >
  template<typename I>
  void nearest(point const& p, I oit, 
              long const max_results) {
    validate("before nearest");
    // The query result iterates over tree elements from which we extract
    // the back references to the primary index.
    auto it = boost::geometry::index::qbegin( 
        tr, boost::geometry::index::nearest(p, max_results));
    auto const end = boost::geometry::index::qend(tr);
    while (it != end) {
      *oit = it->second;
      ++it;
      ++oit;
    }
  }

  // Finds the points in the query box b and writes their positions in the
  // primary index to the output iterator oit.  I must be an iterator to
  // primary_iterator, e.g. std::back_inserter<std::vector<primary_iterator> >
  // Optionally, max_results limits the number of query results
  // and pred restricts the result set to values fulfilling
  // the predicate.
  // TODO: Could use boost::geometry::index::satisfies() somehow.
  template<typename I>
  void query(box const& b, I oit, 
             long const max_results = std::numeric_limits<long>::max(),
             value_predicate const& pred = true_predicate) const {
    validate("before query");
    long n_results = 0;

    // The query result iterates over tree elements from which we extract
    // the back references to the primary index.
    auto it = boost::geometry::index::qbegin( 
        tr,   boost::geometry::index::within(b));
    auto const end = boost::geometry::index::qend(tr);
    while (it != end && n_results < max_results) {
      if (pred(it->second->second)) {
        *oit = it->second;
        ++oit;
        ++n_results;
      }
      ++it;
    }
  }

  void clear() {
    tr .clear();
    map.clear();
  }

private:
  // Insert value and back reference into tree
  void insert_tree(value_type const& p,
                   primary_iterator const it) {
    auto const pt = traits_type::point_from_value(p);
#ifdef CPP_LIB_LOG_SPATIAL_INDEX_OPS
    std::cout << "insert " << it->first 
              << ' ' 
              << boost::geometry::get<0>(pt) << ' '
              << boost::geometry::get<1>(pt) << std::endl;
#endif // CPP_LIB_LOG_SPATIAL_INDEX_OPS

    tr.insert(std::make_pair(pt, it));
  }

  tree_type tr;
  id_map map;
};


} // end namespace math

} // end namespace cpl


#endif // CPP_SPATIAL_INDEX_H
