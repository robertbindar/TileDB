/**
 * @file tiledb/sm/subarray/unit_range_manager.cc
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
 * This file defines unit tests for the RangeManager classes.
 */

#include <catch.hpp>
#include "../range_manager.h"
#include "tiledb/sm/misc/types.h"

using namespace tiledb;
using namespace tiledb::common;
using namespace tiledb::sm;

// TODO: Generalize for all types
TEST_CASE("CreateDefaultDimensionRangeManager") {
  uint64_t bounds[2] = {0, 10};
  Range range{bounds, 2 * sizeof(uint64_t)};
  DimensionRangeManager<uint64_t, false> range_manager{range};
  CHECK(range_manager.num_ranges() == 1);
  Range default_range = range_manager.get_range(0);
  CHECK(!default_range.empty());
  const uint64_t* start = (uint64_t*)default_range.start();
  const uint64_t* end = (uint64_t*)default_range.end();
  CHECK(*start == 0);
  CHECK(*end == 10);
}

// TODO: Generalize for all coalescing types
TEST_CASE("DimensionRangeManager::DimensionRangeManager") {
  uint64_t bounds[2] = {0, 10};
  Range range{bounds, 2 * sizeof(uint64_t)};
  DimensionRangeManager<uint64_t, true> range_manager{range, true};
  CHECK(range_manager.num_ranges() == 0);
  SECTION("Add 2 Overlapping Ranges") {
    uint64_t data1[2] = {1, 3};
    uint64_t data2[2] = {4, 5};
    Range r1{data1, 2 * sizeof(uint64_t)};
    Range r2{data2, 2 * sizeof(uint64_t)};
    range_manager.add_range_unsafe(r1);
    range_manager.add_range_unsafe(r2);
    CHECK(range_manager.num_ranges() == 1);
    auto combined_range = range_manager.get_range(0);
    const uint64_t* start = (uint64_t*)combined_range.start();
    const uint64_t* end = (uint64_t*)combined_range.end();
    CHECK(*start == 1);
    CHECK(*end == 5);
  }
}

// TODO: Generalize for all non-coalescing types
TEST_CASE("DimensionRangeManager::AddRange - coalesce float") {
  float bounds[2] = {-1.0, 1.0};
  Range range{bounds, 2 * sizeof(float)};
  DimensionRangeManager<float, true> range_manager{range, true};
  CHECK(range_manager.num_ranges() == 0);
  SECTION("Add 2 Overlapping Ranges") {
    float data1[2] = {-0.5, 0.5};
    float data2[2] = {0.5, 0.75};
    Range r1{data1, 2 * sizeof(float)};
    Range r2{data2, 2 * sizeof(float)};
    range_manager.add_range_unsafe(r1);
    range_manager.add_range_unsafe(r2);
    CHECK(range_manager.num_ranges() == 2);
  }
}
