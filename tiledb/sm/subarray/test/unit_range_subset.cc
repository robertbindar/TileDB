/**
 * @file tiledb/sm/subarray/unit_range_subset.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2021 TileDB, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * @section DESCRIPTION
 *
 * This file defines unit tests for the RangeSubset classes.
 */

#include <catch.hpp>
#include "../range_subset.h"
#include "tiledb/common/thread_pool.h"
#include "tiledb/sm/misc/types.h"

using namespace tiledb;
using namespace tiledb::common;
using namespace tiledb::sm;

// TODO: Generalize for all types
TEST_CASE("CreateDefaultRangeSubset") {
  uint64_t bounds[2] = {0, 10};
  Range range{bounds, 2 * sizeof(uint64_t)};
  RangeSubset<uint64_t, Datatype::UINT64, false> range_subset{range};
  CHECK(range_subset.num_ranges() == 1);
  Range default_range = range_subset.get_range(0);
  CHECK(!default_range.empty());
  const uint64_t* start = (uint64_t*)default_range.start();
  const uint64_t* end = (uint64_t*)default_range.end();
  CHECK(*start == 0);
  CHECK(*end == 10);
}

// TODO: Generalize for all coalescing types
TEST_CASE("RangeSubset::RangeSubset") {
  uint64_t bounds[2] = {0, 10};
  Range range{bounds, 2 * sizeof(uint64_t)};
  RangeSubset<uint64_t, Datatype::UINT64, true> range_subset{range, true};
  CHECK(range_subset.num_ranges() == 0);
  SECTION("Add 2 Overlapping Ranges") {
    uint64_t data1[2] = {1, 3};
    uint64_t data2[2] = {4, 5};
    Range r1{data1, 2 * sizeof(uint64_t)};
    Range r2{data2, 2 * sizeof(uint64_t)};
    range_subset.add_range_unsafe(r1);
    range_subset.add_range_unsafe(r2);
    CHECK(range_subset.num_ranges() == 1);
    auto combined_range = range_subset.get_range(0);
    const uint64_t* start = (uint64_t*)combined_range.start();
    const uint64_t* end = (uint64_t*)combined_range.end();
    CHECK(*start == 1);
    CHECK(*end == 5);
  }
}

// TODO: Generalize for all non-coalescing types
TEST_CASE("RangeSubset::add_range - coalesce float") {
  float bounds[2] = {-1.0, 1.0};
  Range range{bounds, 2 * sizeof(float)};
  RangeSubset<float, Datatype::FLOAT32, true> range_subset{range, true};
  CHECK(range_subset.num_ranges() == 0);
  SECTION("Add 2 Overlapping Ranges") {
    float data1[2] = {-0.5, 0.5};
    float data2[2] = {0.5, 0.75};
    Range r1{data1, 2 * sizeof(float)};
    Range r2{data2, 2 * sizeof(float)};
    range_subset.add_range_unsafe(r1);
    range_subset.add_range_unsafe(r2);
    CHECK(range_subset.num_ranges() == 2);
  }
}

// TODO: Generalize for all non-sortable types.
TEST_CASE("RangeSubset: Test unsortable", "[range-manager][threadpool]") {
  char bound_data[2] = {'a', 'c'};
  Range range{bound_data, 2 * sizeof(char)};
  RangeSubset<char, Datatype::CHAR, true> range_subset{range};
  REQUIRE(range_subset.num_ranges() == 1);
  ThreadPool pool;
  REQUIRE(pool.init(2).ok());
  auto sort_status = range_subset.sort_ranges(&pool);  // TODO quiet error.
  CHECK(!sort_status.ok());
}

// TODO: Generatize for all numeric sortable types.
TEST_CASE("RangeSubset: Test numeric sort", "[range-manager][threadpool]") {
  uint64_t bounds[2] = {0, 10};
  Range range{bounds, 2 * sizeof(uint64_t)};
  RangeSubset<uint64_t, Datatype::UINT64, true> range_subset{range, true};
  SECTION("Sort 2 reverse ordered ranges") {
    // Add ranges.
    uint64_t data1[2] = {4, 5};
    uint64_t data2[2] = {1, 2};
    Range r1{data1, 2 * sizeof(uint64_t)};
    Range r2{data2, 2 * sizeof(uint64_t)};
    range_subset.add_range_unsafe(r1);
    range_subset.add_range_unsafe(r2);
    CHECK(range_subset.num_ranges() == 2);
    // Create ThreadPool.
    ThreadPool pool;
    REQUIRE(pool.init(2).ok());
    //  Sort ranges.
    auto sort_status = range_subset.sort_ranges(&pool);
    CHECK(sort_status.ok());
    // Check the numner of ranges.
    CHECK(range_subset.num_ranges() == 2);
    // Check the first range.
    auto result_range = range_subset.get_range(0);
    const uint64_t* start = (uint64_t*)result_range.start();
    const uint64_t* end = (uint64_t*)result_range.end();
    CHECK(*start == 1);
    CHECK(*end == 2);
    // Check the second range.
    result_range = range_subset.get_range(1);
    start = (uint64_t*)result_range.start();
    end = (uint64_t*)result_range.end();
    CHECK(*start == 4);
    CHECK(*end == 5);
  }
}

TEST_CASE("RangeSubset::sort - STRING_ASCII") {
  Range range{};
  RangeSubset<uint64_t, Datatype::STRING_ASCII, false> range_subset{
      range, true};
  SECTION("Sort 2 reverse ordered non-overlapping ranges") {
    // Set ranges.
    const std::string d1{"cat"};
    const std::string d2{"dog"};
    const std::string d3{"ax"};
    const std::string d4{"bird"};
    Range r1{};
    r1.set_str_range(d1, d2);
    range_subset.add_range_unsafe(r1);
    Range r2{};
    r2.set_str_range(d3, d4);
    range_subset.add_range_unsafe(r2);
    CHECK(range_subset.num_ranges() == 2);
    // Create ThreadPool.
    ThreadPool pool;
    REQUIRE(pool.init(2).ok());
    // Sort ranges.
    auto sort_status = range_subset.sort_ranges(&pool);
    CHECK(sort_status.ok());
    // Check the number of ranges.
    CHECK(range_subset.num_ranges() == 2);
    // Check the first range.
    auto result_range = range_subset.get_range(0);
    auto start = result_range.start_str();
    auto end = result_range.end_str();
    CHECK(start == d3);
    CHECK(end == d4);
    // Check the second range.
    result_range = range_subset.get_range(1);
  }
}
