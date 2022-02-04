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

#include <vector>

using namespace tiledb;
using namespace tiledb::common;
using namespace tiledb::sm;

TEST_CASE("DimensionRangeManager::DimensionRangeManager") {
  std::vector<std::vector<Range>> ranges(1);
  uint64_t bounds[2] = {0, 10};
  Range range{bounds, 2 * sizeof(uint64_t)};
  DimensionRangeManager<uint64_t> range_manager{0, range, ranges};
}
