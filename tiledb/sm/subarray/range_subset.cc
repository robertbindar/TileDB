/**
 * @file   range_subset.cc
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
 * This file defines the class RangeSubset.
 */

#include "tiledb/sm/subarray/range_subset.h"

#include <iostream>

using namespace tiledb::common;

namespace tiledb {
namespace sm {

tdb_shared_ptr<RangeSubsetBase> create_range_subset(
    Datatype datatype,
    const Range& full_range,
    bool is_default,
    bool coalesce_ranges) {
  switch (datatype) {
    case Datatype::INT8:
      return create_range_subset<int8_t, Datatype::INT8>(
          full_range, is_default, coalesce_ranges);
    case Datatype::UINT8:
      return create_range_subset<uint8_t, Datatype::UINT8>(
          full_range, is_default, coalesce_ranges);
    case Datatype::INT16:
      return create_range_subset<int16_t, Datatype::INT16>(
          full_range, is_default, coalesce_ranges);
    case Datatype::UINT16:
      return create_range_subset<uint16_t, Datatype::UINT16>(
          full_range, is_default, coalesce_ranges);
    case Datatype::INT32:
      return create_range_subset<int32_t, Datatype::INT32>(
          full_range, is_default, coalesce_ranges);
    case Datatype::UINT32:
      return create_range_subset<uint32_t, Datatype::UINT32>(
          full_range, is_default, coalesce_ranges);
    case Datatype::INT64:
      return create_range_subset<int64_t, Datatype::INT64>(
          full_range, is_default, coalesce_ranges);
    case Datatype::UINT64:
      return create_range_subset<uint64_t, Datatype::UINT64>(
          full_range, is_default, coalesce_ranges);
    case Datatype::FLOAT32:
      return create_range_subset<float, Datatype::FLOAT32>(
          full_range, is_default, coalesce_ranges);
    case Datatype::FLOAT64:
      return create_range_subset<double, Datatype::FLOAT64>(
          full_range, is_default, coalesce_ranges);
    case Datatype::DATETIME_YEAR:
      return create_range_subset<int64_t, Datatype::DATETIME_YEAR>(
          full_range, is_default, coalesce_ranges);
    case Datatype::DATETIME_MONTH:
      return create_range_subset<int64_t, Datatype::DATETIME_MONTH>(
          full_range, is_default, coalesce_ranges);
    case Datatype::DATETIME_WEEK:
      return create_range_subset<int64_t, Datatype::DATETIME_WEEK>(
          full_range, is_default, coalesce_ranges);
    case Datatype::DATETIME_DAY:
      return create_range_subset<int64_t, Datatype::DATETIME_DAY>(
          full_range, is_default, coalesce_ranges);
    case Datatype::DATETIME_HR:
      return create_range_subset<int64_t, Datatype::DATETIME_HR>(
          full_range, is_default, coalesce_ranges);
    case Datatype::DATETIME_MIN:
      return create_range_subset<int64_t, Datatype::DATETIME_MIN>(
          full_range, is_default, coalesce_ranges);
    case Datatype::DATETIME_SEC:
      return create_range_subset<int64_t, Datatype::DATETIME_SEC>(
          full_range, is_default, coalesce_ranges);
    case Datatype::DATETIME_MS:
      return create_range_subset<int64_t, Datatype::DATETIME_MS>(
          full_range, is_default, coalesce_ranges);
    case Datatype::DATETIME_US:
      return create_range_subset<int64_t, Datatype::DATETIME_US>(
          full_range, is_default, coalesce_ranges);
    case Datatype::DATETIME_NS:
      return create_range_subset<int64_t, Datatype::DATETIME_NS>(
          full_range, is_default, coalesce_ranges);
    case Datatype::DATETIME_PS:
      return create_range_subset<int64_t, Datatype::DATETIME_PS>(
          full_range, is_default, coalesce_ranges);
    case Datatype::DATETIME_FS:
      return create_range_subset<int64_t, Datatype::DATETIME_FS>(
          full_range, is_default, coalesce_ranges);
    case Datatype::DATETIME_AS:
      return create_range_subset<int64_t, Datatype::DATETIME_AS>(
          full_range, is_default, coalesce_ranges);
    case Datatype::TIME_HR:
      return create_range_subset<int64_t, Datatype::TIME_HR>(
          full_range, is_default, coalesce_ranges);
    case Datatype::TIME_MIN:
      return create_range_subset<int64_t, Datatype::TIME_MIN>(
          full_range, is_default, coalesce_ranges);
    case Datatype::TIME_SEC:
      return create_range_subset<int64_t, Datatype::TIME_SEC>(
          full_range, is_default, coalesce_ranges);
    case Datatype::TIME_MS:
      return create_range_subset<int64_t, Datatype::TIME_MS>(
          full_range, is_default, coalesce_ranges);
    case Datatype::TIME_US:
      return create_range_subset<int64_t, Datatype::TIME_US>(
          full_range, is_default, coalesce_ranges);
    case Datatype::TIME_NS:
      return create_range_subset<int64_t, Datatype::TIME_NS>(
          full_range, is_default, coalesce_ranges);
    case Datatype::TIME_PS:
      return create_range_subset<int64_t, Datatype::TIME_PS>(
          full_range, is_default, coalesce_ranges);
    case Datatype::TIME_FS:
      return create_range_subset<int64_t, Datatype::TIME_FS>(
          full_range, is_default, coalesce_ranges);
    case Datatype::TIME_AS:
      return create_range_subset<int64_t, Datatype::TIME_AS>(
          full_range, is_default, coalesce_ranges);
    case Datatype::CHAR:
      return create_range_subset<char, Datatype::CHAR>(
          full_range, is_default, coalesce_ranges);
    case Datatype::STRING_ASCII:
      return create_range_subset<std::string, Datatype::STRING_ASCII>(
          full_range, is_default, coalesce_ranges);
    case Datatype::STRING_UTF8:
      return create_range_subset<std::string, Datatype::STRING_UTF8>(
          full_range, is_default, coalesce_ranges);
    case Datatype::STRING_UTF16:
      return create_range_subset<std::string, Datatype::STRING_UTF16>(
          full_range, is_default, coalesce_ranges);
    case Datatype::STRING_UTF32:
      return create_range_subset<std::string, Datatype::STRING_UTF32>(
          full_range, is_default, coalesce_ranges);
    case Datatype::STRING_UCS2:
      return create_range_subset<std::string, Datatype::STRING_UCS2>(
          full_range, is_default, coalesce_ranges);
    case Datatype::STRING_UCS4:
      return create_range_subset<std::string, Datatype::STRING_UCS4>(
          full_range, is_default, coalesce_ranges);
    case Datatype::ANY:
      return create_range_subset<uint8_t, Datatype::ANY>(
          full_range, is_default, coalesce_ranges);
    default:
      LOG_FATAL("Unexpected datatype " + datatype_str(datatype));
      return nullptr;
  }
}

}  // namespace sm
}  // namespace tiledb
