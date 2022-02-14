/**
 * @file   range_manager.cc
 *
 * @section LICENSE
 *
 * The MIT License
 *
 * @copyright Copyright (c) 2017-2022 TileDB, Inc.
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
 * This file defines the class RangeManager.
 */

#include "tiledb/sm/subarray/range_manager.h"

#include <iostream>

using namespace tiledb::common;

namespace tiledb {
namespace sm {

tdb_shared_ptr<RangeManager> create_default_range_manager(
    Datatype datatype, const Range& range_bounds) {
  switch (datatype) {
    case Datatype::INT8:
      return make_shared<DimensionRangeManager<int8_t, false>>(
          HERE(), range_bounds);
    case Datatype::UINT8:
      return make_shared<DimensionRangeManager<uint8_t, false>>(
          HERE(), range_bounds);
    case Datatype::INT16:
      return make_shared<DimensionRangeManager<int16_t, false>>(
          HERE(), range_bounds);
    case Datatype::UINT16:
      return make_shared<DimensionRangeManager<uint16_t, false>>(
          HERE(), range_bounds);
    case Datatype::INT32:
      return make_shared<DimensionRangeManager<int32_t, false>>(
          HERE(), range_bounds);
    case Datatype::UINT32:
      return make_shared<DimensionRangeManager<uint32_t, false>>(
          HERE(), range_bounds);
    case Datatype::INT64:
      return make_shared<DimensionRangeManager<int64_t, false>>(
          HERE(), range_bounds);
    case Datatype::UINT64:
      return make_shared<DimensionRangeManager<uint64_t, false>>(
          HERE(), range_bounds);
    case Datatype::FLOAT32:
      return make_shared<DimensionRangeManager<float, false>>(
          HERE(), range_bounds);
    case Datatype::FLOAT64:
      return make_shared<DimensionRangeManager<double, false>>(
          HERE(), range_bounds);
    case Datatype::DATETIME_YEAR:
    case Datatype::DATETIME_MONTH:
    case Datatype::DATETIME_WEEK:
    case Datatype::DATETIME_DAY:
    case Datatype::DATETIME_HR:
    case Datatype::DATETIME_MIN:
    case Datatype::DATETIME_SEC:
    case Datatype::DATETIME_MS:
    case Datatype::DATETIME_US:
    case Datatype::DATETIME_NS:
    case Datatype::DATETIME_PS:
    case Datatype::DATETIME_FS:
    case Datatype::DATETIME_AS:
    case Datatype::TIME_HR:
    case Datatype::TIME_MIN:
    case Datatype::TIME_SEC:
    case Datatype::TIME_MS:
    case Datatype::TIME_US:
    case Datatype::TIME_NS:
    case Datatype::TIME_PS:
    case Datatype::TIME_FS:
    case Datatype::TIME_AS:
      return make_shared<DimensionRangeManager<int64_t, false>>(
          HERE(), range_bounds);
    case Datatype::CHAR:
      return make_shared<DimensionRangeManager<char, false>>(
          HERE(), range_bounds);
    case Datatype::STRING_ASCII:
    case Datatype::STRING_UTF8:
    case Datatype::STRING_UTF16:
    case Datatype::STRING_UTF32:
    case Datatype::STRING_UCS2:
    case Datatype::STRING_UCS4:
      return make_shared<DimensionRangeManager<std::string, false>>(
          HERE(), range_bounds);
    case Datatype::ANY:
      return make_shared<DimensionRangeManager<uint8_t, false>>(
          HERE(), range_bounds);
    default:
      LOG_FATAL("Unexpected datatype " + datatype_str(datatype));
      return nullptr;
  }
}

tdb_shared_ptr<RangeManager> create_range_manager(
    Datatype datatype,
    const Range& range_bounds,
    bool allow_multiple_ranges,
    bool coalesce_ranges) {
  switch (datatype) {
    case Datatype::INT8:
      return create_range_manager<int8_t>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::UINT8:
      return create_range_manager<uint8_t>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::INT16:
      return create_range_manager<int16_t>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::UINT16:
      return create_range_manager<uint16_t>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::INT32:
      return create_range_manager<int32_t>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::UINT32:
      return create_range_manager<uint32_t>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::INT64:
      return create_range_manager<int64_t>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::UINT64:
      return create_range_manager<uint64_t>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::FLOAT32:
      return create_range_manager<float>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::FLOAT64:
      return create_range_manager<double>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::DATETIME_YEAR:
    case Datatype::DATETIME_MONTH:
    case Datatype::DATETIME_WEEK:
    case Datatype::DATETIME_DAY:
    case Datatype::DATETIME_HR:
    case Datatype::DATETIME_MIN:
    case Datatype::DATETIME_SEC:
    case Datatype::DATETIME_MS:
    case Datatype::DATETIME_US:
    case Datatype::DATETIME_NS:
    case Datatype::DATETIME_PS:
    case Datatype::DATETIME_FS:
    case Datatype::DATETIME_AS:
    case Datatype::TIME_HR:
    case Datatype::TIME_MIN:
    case Datatype::TIME_SEC:
    case Datatype::TIME_MS:
    case Datatype::TIME_US:
    case Datatype::TIME_NS:
    case Datatype::TIME_PS:
    case Datatype::TIME_FS:
    case Datatype::TIME_AS:
      return create_range_manager<int64_t>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::CHAR:
      return create_range_manager<char>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::STRING_ASCII:
    case Datatype::STRING_UTF8:
    case Datatype::STRING_UTF16:
    case Datatype::STRING_UTF32:
    case Datatype::STRING_UCS2:
    case Datatype::STRING_UCS4:
      return create_range_manager<std::string>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::ANY:
      return create_range_manager<uint8_t>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    default:
      LOG_FATAL("Unexpected datatype " + datatype_str(datatype));
      return nullptr;
  }
}

}  // namespace sm
}  // namespace tiledb
