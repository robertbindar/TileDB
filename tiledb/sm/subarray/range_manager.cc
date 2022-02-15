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
      return create_default_range_manager<int8_t, Datatype::INT8>(range_bounds);
    case Datatype::UINT8:
      return create_default_range_manager<uint8_t, Datatype::UINT8>(
          range_bounds);
    case Datatype::INT16:
      return create_default_range_manager<int16_t, Datatype::INT16>(
          range_bounds);
    case Datatype::UINT16:
      return create_default_range_manager<uint16_t, Datatype::UINT16>(
          range_bounds);
    case Datatype::INT32:
      return create_default_range_manager<int32_t, Datatype::INT32>(
          range_bounds);
    case Datatype::UINT32:
      return create_default_range_manager<uint32_t, Datatype::UINT32>(
          range_bounds);
    case Datatype::INT64:
      return create_default_range_manager<int64_t, Datatype::INT64>(
          range_bounds);
    case Datatype::UINT64:
      return create_default_range_manager<uint64_t, Datatype::UINT64>(
          range_bounds);
    case Datatype::FLOAT32:
      return create_default_range_manager<float, Datatype::FLOAT32>(
          range_bounds);
    case Datatype::FLOAT64:
      return create_default_range_manager<double, Datatype::FLOAT64>(
          range_bounds);
    case Datatype::DATETIME_YEAR:
      return create_default_range_manager<int64_t, Datatype::DATETIME_YEAR>(
          range_bounds);
    case Datatype::DATETIME_MONTH:
      return create_default_range_manager<int64_t, Datatype::DATETIME_MONTH>(
          range_bounds);
    case Datatype::DATETIME_WEEK:
      return create_default_range_manager<int64_t, Datatype::DATETIME_WEEK>(
          range_bounds);
    case Datatype::DATETIME_DAY:
      return create_default_range_manager<int64_t, Datatype::DATETIME_DAY>(
          range_bounds);
    case Datatype::DATETIME_HR:
      return create_default_range_manager<int64_t, Datatype::DATETIME_HR>(
          range_bounds);
    case Datatype::DATETIME_MIN:
      return create_default_range_manager<int64_t, Datatype::DATETIME_MIN>(
          range_bounds);
    case Datatype::DATETIME_SEC:
      return create_default_range_manager<int64_t, Datatype::DATETIME_SEC>(
          range_bounds);
    case Datatype::DATETIME_MS:
      return create_default_range_manager<int64_t, Datatype::DATETIME_MS>(
          range_bounds);
    case Datatype::DATETIME_US:
      return create_default_range_manager<int64_t, Datatype::DATETIME_US>(
          range_bounds);
    case Datatype::DATETIME_NS:
      return create_default_range_manager<int64_t, Datatype::DATETIME_NS>(
          range_bounds);
    case Datatype::DATETIME_PS:
      return create_default_range_manager<int64_t, Datatype::DATETIME_PS>(
          range_bounds);
    case Datatype::DATETIME_FS:
      return create_default_range_manager<int64_t, Datatype::DATETIME_FS>(
          range_bounds);
    case Datatype::DATETIME_AS:
      return create_default_range_manager<int64_t, Datatype::DATETIME_AS>(
          range_bounds);
    case Datatype::TIME_HR:
      return create_default_range_manager<int64_t, Datatype::TIME_HR>(
          range_bounds);
    case Datatype::TIME_MIN:
      return create_default_range_manager<int64_t, Datatype::TIME_MIN>(
          range_bounds);
    case Datatype::TIME_SEC:
      return create_default_range_manager<int64_t, Datatype::TIME_SEC>(
          range_bounds);
    case Datatype::TIME_MS:
      return create_default_range_manager<int64_t, Datatype::TIME_MS>(
          range_bounds);
    case Datatype::TIME_US:
      return create_default_range_manager<int64_t, Datatype::TIME_US>(
          range_bounds);
    case Datatype::TIME_NS:
      return create_default_range_manager<int64_t, Datatype::TIME_NS>(
          range_bounds);
    case Datatype::TIME_PS:
      return create_default_range_manager<int64_t, Datatype::TIME_PS>(
          range_bounds);
    case Datatype::TIME_FS:
      return create_default_range_manager<int64_t, Datatype::TIME_FS>(
          range_bounds);
    case Datatype::TIME_AS:
      return create_default_range_manager<int64_t, Datatype::TIME_AS>(
          range_bounds);
    case Datatype::CHAR:
      return create_default_range_manager<char, Datatype::CHAR>(range_bounds);
    case Datatype::STRING_ASCII:
      return create_default_range_manager<std::string, Datatype::STRING_ASCII>(
          range_bounds);
    case Datatype::STRING_UTF8:
      return create_default_range_manager<std::string, Datatype::STRING_UTF8>(
          range_bounds);
    case Datatype::STRING_UTF16:
      return create_default_range_manager<std::string, Datatype::STRING_UTF16>(
          range_bounds);
    case Datatype::STRING_UTF32:
      return create_default_range_manager<std::string, Datatype::STRING_UTF32>(
          range_bounds);
    case Datatype::STRING_UCS2:
      return create_default_range_manager<std::string, Datatype::STRING_UCS2>(
          range_bounds);
    case Datatype::STRING_UCS4:
      return create_default_range_manager<std::string, Datatype::STRING_UCS4>(
          range_bounds);
    case Datatype::ANY:
      return create_default_range_manager<uint8_t, Datatype::ANY>(range_bounds);
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
      return create_range_manager<int8_t, Datatype::INT8>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::UINT8:
      return create_range_manager<uint8_t, Datatype::UINT8>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::INT16:
      return create_range_manager<int16_t, Datatype::INT16>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::UINT16:
      return create_range_manager<uint16_t, Datatype::UINT16>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::INT32:
      return create_range_manager<int32_t, Datatype::INT32>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::UINT32:
      return create_range_manager<uint32_t, Datatype::UINT32>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::INT64:
      return create_range_manager<int64_t, Datatype::INT64>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::UINT64:
      return create_range_manager<uint64_t, Datatype::UINT64>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::FLOAT32:
      return create_range_manager<float, Datatype::FLOAT32>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::FLOAT64:
      return create_range_manager<double, Datatype::FLOAT64>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::DATETIME_YEAR:
      return create_range_manager<int64_t, Datatype::DATETIME_YEAR>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::DATETIME_MONTH:
      return create_range_manager<int64_t, Datatype::DATETIME_MONTH>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::DATETIME_WEEK:
      return create_range_manager<int64_t, Datatype::DATETIME_WEEK>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::DATETIME_DAY:
      return create_range_manager<int64_t, Datatype::DATETIME_DAY>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::DATETIME_HR:
      return create_range_manager<int64_t, Datatype::DATETIME_HR>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::DATETIME_MIN:
      return create_range_manager<int64_t, Datatype::DATETIME_MIN>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::DATETIME_SEC:
      return create_range_manager<int64_t, Datatype::DATETIME_SEC>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::DATETIME_MS:
      return create_range_manager<int64_t, Datatype::DATETIME_MS>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::DATETIME_US:
      return create_range_manager<int64_t, Datatype::DATETIME_US>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::DATETIME_NS:
      return create_range_manager<int64_t, Datatype::DATETIME_NS>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::DATETIME_PS:
      return create_range_manager<int64_t, Datatype::DATETIME_PS>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::DATETIME_FS:
      return create_range_manager<int64_t, Datatype::DATETIME_FS>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::DATETIME_AS:
      return create_range_manager<int64_t, Datatype::DATETIME_AS>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::TIME_HR:
      return create_range_manager<int64_t, Datatype::TIME_HR>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::TIME_MIN:
      return create_range_manager<int64_t, Datatype::TIME_MIN>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::TIME_SEC:
      return create_range_manager<int64_t, Datatype::TIME_SEC>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::TIME_MS:
      return create_range_manager<int64_t, Datatype::TIME_MS>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::TIME_US:
      return create_range_manager<int64_t, Datatype::TIME_US>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::TIME_NS:
      return create_range_manager<int64_t, Datatype::TIME_NS>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::TIME_PS:
      return create_range_manager<int64_t, Datatype::TIME_PS>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::TIME_FS:
      return create_range_manager<int64_t, Datatype::TIME_FS>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::TIME_AS:
      return create_range_manager<int64_t, Datatype::TIME_AS>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::CHAR:
      return create_range_manager<char, Datatype::CHAR>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::STRING_ASCII:
      return create_range_manager<std::string, Datatype::STRING_ASCII>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::STRING_UTF8:
      return create_range_manager<std::string, Datatype::STRING_UTF8>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::STRING_UTF16:
      return create_range_manager<std::string, Datatype::STRING_UTF16>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::STRING_UTF32:
      return create_range_manager<std::string, Datatype::STRING_UTF32>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::STRING_UCS2:
      return create_range_manager<std::string, Datatype::STRING_UCS2>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::STRING_UCS4:
      return create_range_manager<std::string, Datatype::STRING_UCS4>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    case Datatype::ANY:
      return create_range_manager<uint8_t, Datatype::ANY>(
          range_bounds, allow_multiple_ranges, coalesce_ranges);
    default:
      LOG_FATAL("Unexpected datatype " + datatype_str(datatype));
      return nullptr;
  }
}

}  // namespace sm
}  // namespace tiledb
