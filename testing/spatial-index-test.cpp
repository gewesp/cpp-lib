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
#include <random>

#include "cpp-lib/container-util.h"
#include "cpp-lib/gnss.h"
#include "cpp-lib/spatial-index.h"
#include "cpp-lib/util.h"

using namespace cpl::math;

typedef long id_type;

typedef cpl::gnss::position_time value_type;

// A predicate
bool time_even(value_type const& v) {
  return std::fmod(v.time, 2) < .001;
}

// Uses traits on the base type lat_lon for an index on position_time.
typedef spatial_index<id_type, value_type,
        cpl::math::spatial_index_traits<cpl::gnss::lat_lon> > my_index;


// This updater sometimes doesn't update the indexable even
// if it says so.  The index must cope with this.
struct funky_updater {

  funky_updater()
    : mstd{4711},
      U01{0.0, 1.0} {}

  mutable std::minstd_rand mstd;
  mutable std::uniform_real_distribution<double> U01;

  void set_value(value_type const& ll_new) {
    value_to_set = ll_new;
  }

  value_type const& new_element(id_type const&) const {
    return value_to_set;
  }

  bool update_element(id_type const, cpl::gnss::lat_lon& ll_existing) const {
    bool const do_update = U01(mstd) > 0.5;
    if (!do_update) {
      bool const report_correctly = U01(mstd) > 0.3;
      if (report_correctly) {
        return false;
      } else {
        // Report an update even though it didn't happen
        return true;
      }
    } else {
      ll_existing = value_to_set;
      return true;
    }
  }

private:
  value_type value_to_set;
};

void test_funky(std::ostream& os) {

  os << "Frequent updates of few IDs" << std::endl;

  std::minstd_rand mstd(4711);
  std::uniform_int_distribution<int> ID(0, 100);
  std::uniform_int_distribution<int> coord(0, 5);

  my_index idx;

  funky_updater fuf;
  for (int i = 0; i < 1000000; ++i) {
    id_type const id = ID(mstd);
    // cpl::gnss::position_time const pt1{coord(mstd), coord(mstd), 0, 0};
    cpl::gnss::position_time const pt(coord(mstd), coord(mstd), 0, 0);
    fuf.set_value(pt);
    idx.upsert(id, fuf);
  }

  os << "OK" << std::endl;

}

void erase_all(my_index& idx) {
  cpl::util::container::erase_if(
      idx, [](std::pair<id_type, cpl::gnss::position_time> const&) {
        return true;
      });

  always_assert(0 == idx.size());
}

//
// Test massive random index updates and proximity queries, similar to
// ktrax application.
//
// interval: How many update/query pairs before outputting stats
// repeat: How often to repeat the test
// max_ids:  Maximum ID, choose the expected number of clients at a given
//           time, the index will quickly fill up
// max_xy:   Area size, both x and y
// r:        Query rectangle size 
//
// Results 2/2015 on MacBook:
// quadratic<16>:
// * 10'000 clients are OK with an expected 10 element query result
//   (--> ca. 3x realtime)
// * Not very sensitive to query radius, even up to 100 results may
//   be possible
// * Not very sensitive to total client number either, mostly relevant
//   are the incoming updates
//
// quadratic<4>:
// * 100'000 clients are OK with an expected 10 element query result
//   (--> ca. 2x realtime).  Wow!!
//

void test_index(
    std::ostream& os,
    my_index& idx,
    const long repeat,
    const long interval,
    const int max_ids, const double max_xy,
    const double r,
    long const max_results,
    my_index::value_predicate const& pred = my_index::true_predicate) {
  std::minstd_rand mstd(4711);
  std::uniform_real_distribution<double> U(-max_xy, max_xy);
  std::uniform_int_distribution<int> I(0, max_ids - 1);

  std::vector<my_index::primary_iterator> near;
  for (long j = 0; j < repeat; ++j) {
    double size_sum = 0;
    for (long i = 0; i < interval; ++i) {
      id_type const id = I(mstd);
      cpl::gnss::position_time const pt(U(mstd), U(mstd), 0, i);
      box const query_box(point(pt.lat - r, pt.lon - r), 
                          point(pt.lat + r, pt.lon + r));
      idx.upsert(id, my_index::default_updater{pt});
      near.clear();
      idx.query(query_box, std::back_inserter(near), max_results, pred);
      size_sum += near.size();

      // Check result set.  Items are iterators into id_map.
      int self = 0;
      for (auto const& item : near) {
        self += item->first == id;
        cpl::gnss::position_time const& res = item->second;
        double const dlat = res.lat - pt.lat;
        double const dlon = res.lon - pt.lon;
        // Check results are within query box
        cpl::util::verify(
            dlat * dlat + dlon * dlon <= 2 * r * r,
            "result outside query box");
      }
      // We should see ourselves exactly once.
      if (std::numeric_limits<long>::max() == max_results) {
        cpl::util::verify(1 == self, "self not in result set");
      }
    }
    os << interval << " update/query pairs; elements: " << idx.size()
       << "; average result set size: " << size_sum / interval
       << std::endl;
  }

}

// Create an index of n random points and perform n_queries nearest
// neighbor queries with max_results results.
void test_nearest(
    std::ostream& os,
    const long n, const double max_xy,
    const long n_queries,
    long const max_results) {
  std::minstd_rand mstd(4711);
  std::uniform_real_distribution<double> U(-max_xy, max_xy);

  my_index idx;

  os << "Creating " << n << " random points" << std::endl;

  for (long i = 0; i < n; ++i) {
    cpl::gnss::position_time const pt(U(mstd), U(mstd), 0, 0);
    idx.upsert(i, my_index::default_updater{pt});
  }

  os << "Performing " << n_queries << " k-nearest-neighbor queries with k = "
     << max_results
     << std::endl;

  std::vector<my_index::primary_iterator> near;
  for (long i = 0; i < n_queries; ++i) {
    point const q(U(mstd), U(mstd));
    near.clear();
    idx.nearest(q, std::back_inserter(near), max_results);
  }
}

void test_crash_regression(std::ostream& os) {
  // Add 30, remove all, then add 1 -> BOOM
  os << "Crash regression test" << std::endl;
  my_index idx;
  test_index(os, idx, 1, 30, 100000, 89.0, 1.0, 
             std::numeric_limits<long>::max());
  erase_all(idx);
  test_index(os, idx, 1, 1, 100000, 89.0, 1.0, 
             std::numeric_limits<long>::max());
}

// Index supports duplicate points, as long as the ID is different.
void test_duplicates(my_index& idx) {
  cpl::gnss::position_time const pt1{1, 2, 0, 0};
  cpl::gnss::position_time const pt2{3, 4, 0, 0};
  id_type const id1 = 1;
  id_type const id2 = 2;
  id_type const id3 = 3;

  idx.upsert(id1, my_index::default_updater{pt1});
  idx.upsert(id1, my_index::default_updater{pt2});

  always_assert(1 == idx.size());
  erase_all(idx);
  always_assert(0 == idx.size());

  idx.upsert(id1, my_index::default_updater{pt1});
  idx.upsert(id2, my_index::default_updater{pt1});
  idx.upsert(id3, my_index::default_updater{pt1});

  always_assert(3 == idx.size());
  idx.upsert(id3, my_index::default_updater{pt2});
  always_assert(3 == idx.size());
  idx.erase(cpl::util::container::advanced(idx.begin(), 2));
  always_assert(2 == idx.size());
  idx.erase(cpl::util::container::advanced(idx.begin(), 1));
  always_assert(1 == idx.size());
  idx.erase(cpl::util::container::advanced(idx.begin(), 0));
  always_assert(0 == idx.size());
  erase_all(idx);
}

int main() { 
  try {
    { 
      std::cout << "Testing dupes" << std::endl;
      my_index idx;
      test_duplicates(idx);
      test_duplicates(idx);
      test_duplicates(idx);
      test_duplicates(idx);
    }
    test_crash_regression(std::cout);
    test_funky(std::cout);

    // Repeat 10 * 100'000 upserts and proximity queries,
    // [-89, 89]^2, 1 degree query box -> 13.5 average result set size
    // Timing: About 10 seconds on MacBook Pro as of February 2015
    std::cout << "Arbitrary result set size\n";
    my_index idx;
    test_index(std::cout, idx, 8, 100000, 100000, 89.0, 1.0, 
               std::numeric_limits<long>::max());
    erase_all(idx);
    std::cout << "Result size: Maximum 3\n";
    test_index(std::cout, idx, 2, 100000, 100000, 89.0, 1.0, 3);
    erase_all(idx);

    std::cout << "Avg. result size: 250.5 (only even time values)\n";
    test_index(std::cout, idx, 1, 1000, 1000000000, 89.0, 1000.0, 100000,
               time_even);
    erase_all(idx);

    std::cout << "Avg. result size: ~6 (only even time values)\n";
    test_index(std::cout, idx, 1, 1000, 1000000000, 89.0, 1000.0, 6,
               time_even);
    erase_all(idx);

    // k-NN queries
    test_nearest(std::cout, 100000, 89.0, 10000, 100);
    test_nearest(std::cout, 100000, 89.0, 10000, 200);
    test_nearest(std::cout, 100000, 89.0, 10000, 300);

  } catch (std::exception const& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
  }
}
