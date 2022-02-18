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

template <typename T, Datatype D>
tdb_shared_ptr<detail::RangeSubsetInternals> create_range_subset_internals(
    bool coalesce_ranges) {
  if (coalesce_ranges)
    return make_shared<detail::RangeSubsetInternalsImpl<T, D, true>>(HERE());
  return make_shared<detail::RangeSubsetInternalsImpl<T, D, false>>(HERE());
};

tdb_shared_ptr<detail::RangeSubsetInternals> range_subset_internals(
    Datatype datatype, bool coalesce_ranges) {
  switch (datatype) {
    case Datatype::INT8:
      return create_range_subset_internals<int8_t, Datatype::INT8>(
          coalesce_ranges);
    case Datatype::UINT8:
      return create_range_subset_internals<uint8_t, Datatype::UINT8>(
          coalesce_ranges);
    case Datatype::INT16:
      return create_range_subset_internals<int16_t, Datatype::INT16>(
          coalesce_ranges);
    case Datatype::UINT16:
      return create_range_subset_internals<uint16_t, Datatype::UINT16>(
          coalesce_ranges);
    case Datatype::INT32:
      return create_range_subset_internals<int32_t, Datatype::INT32>(
          coalesce_ranges);
    case Datatype::UINT32:
      return create_range_subset_internals<uint32_t, Datatype::UINT32>(
          coalesce_ranges);
    case Datatype::INT64:
      return create_range_subset_internals<int64_t, Datatype::INT64>(
          coalesce_ranges);
    case Datatype::UINT64:
      return create_range_subset_internals<uint64_t, Datatype::UINT64>(
          coalesce_ranges);
    case Datatype::FLOAT32:
      return create_range_subset_internals<float, Datatype::FLOAT32>(
          coalesce_ranges);
    case Datatype::FLOAT64:
      return create_range_subset_internals<double, Datatype::FLOAT64>(
          coalesce_ranges);
    case Datatype::DATETIME_YEAR:
      return create_range_subset_internals<int64_t, Datatype::DATETIME_YEAR>(
          coalesce_ranges);
    case Datatype::DATETIME_MONTH:
      return create_range_subset_internals<int64_t, Datatype::DATETIME_MONTH>(
          coalesce_ranges);
    case Datatype::DATETIME_WEEK:
      return create_range_subset_internals<int64_t, Datatype::DATETIME_WEEK>(
          coalesce_ranges);
    case Datatype::DATETIME_DAY:
      return create_range_subset_internals<int64_t, Datatype::DATETIME_DAY>(
          coalesce_ranges);
    case Datatype::DATETIME_HR:
      return create_range_subset_internals<int64_t, Datatype::DATETIME_HR>(
          coalesce_ranges);
    case Datatype::DATETIME_MIN:
      return create_range_subset_internals<int64_t, Datatype::DATETIME_MIN>(
          coalesce_ranges);
    case Datatype::DATETIME_SEC:
      return create_range_subset_internals<int64_t, Datatype::DATETIME_SEC>(
          coalesce_ranges);
    case Datatype::DATETIME_MS:
      return create_range_subset_internals<int64_t, Datatype::DATETIME_MS>(
          coalesce_ranges);
    case Datatype::DATETIME_US:
      return create_range_subset_internals<int64_t, Datatype::DATETIME_US>(
          coalesce_ranges);
    case Datatype::DATETIME_NS:
      return create_range_subset_internals<int64_t, Datatype::DATETIME_NS>(
          coalesce_ranges);
    case Datatype::DATETIME_PS:
      return create_range_subset_internals<int64_t, Datatype::DATETIME_PS>(
          coalesce_ranges);
    case Datatype::DATETIME_FS:
      return create_range_subset_internals<int64_t, Datatype::DATETIME_FS>(
          coalesce_ranges);
    case Datatype::DATETIME_AS:
      return create_range_subset_internals<int64_t, Datatype::DATETIME_AS>(
          coalesce_ranges);
    case Datatype::TIME_HR:
      return create_range_subset_internals<int64_t, Datatype::TIME_HR>(
          coalesce_ranges);
    case Datatype::TIME_MIN:
      return create_range_subset_internals<int64_t, Datatype::TIME_MIN>(
          coalesce_ranges);
    case Datatype::TIME_SEC:
      return create_range_subset_internals<int64_t, Datatype::TIME_SEC>(
          coalesce_ranges);
    case Datatype::TIME_MS:
      return create_range_subset_internals<int64_t, Datatype::TIME_MS>(
          coalesce_ranges);
    case Datatype::TIME_US:
      return create_range_subset_internals<int64_t, Datatype::TIME_US>(
          coalesce_ranges);
    case Datatype::TIME_NS:
      return create_range_subset_internals<int64_t, Datatype::TIME_NS>(
          coalesce_ranges);
    case Datatype::TIME_PS:
      return create_range_subset_internals<int64_t, Datatype::TIME_PS>(
          coalesce_ranges);
    case Datatype::TIME_FS:
      return create_range_subset_internals<int64_t, Datatype::TIME_FS>(
          coalesce_ranges);
    case Datatype::TIME_AS:
      return create_range_subset_internals<int64_t, Datatype::TIME_AS>(
          coalesce_ranges);
    case Datatype::CHAR:
      return create_range_subset_internals<char, Datatype::CHAR>(
          coalesce_ranges);
    case Datatype::STRING_ASCII:
      return create_range_subset_internals<std::string, Datatype::STRING_ASCII>(
          coalesce_ranges);
    case Datatype::STRING_UTF8:
      return create_range_subset_internals<std::string, Datatype::STRING_UTF8>(
          coalesce_ranges);
    case Datatype::STRING_UTF16:
      return create_range_subset_internals<std::string, Datatype::STRING_UTF16>(
          coalesce_ranges);
    case Datatype::STRING_UTF32:
      return create_range_subset_internals<std::string, Datatype::STRING_UTF32>(
          coalesce_ranges);
    case Datatype::STRING_UCS2:
      return create_range_subset_internals<std::string, Datatype::STRING_UCS2>(
          coalesce_ranges);
    case Datatype::STRING_UCS4:
      return create_range_subset_internals<std::string, Datatype::STRING_UCS4>(
          coalesce_ranges);
    case Datatype::ANY:
      return create_range_subset_internals<uint8_t, Datatype::ANY>(
          coalesce_ranges);
    default:
      LOG_FATAL("Unexpected datatype " + datatype_str(datatype));
      return nullptr;
  }
}

RangeSubset::RangeSubset(
    Datatype datatype,
    const Range& full_range,
    bool is_default,
    bool coalesce_ranges)
    : impl_(range_subset_internals(datatype, coalesce_ranges))
    , full_range_(full_range)
    , is_default_(is_default) {
  if (is_default)
    ranges_.emplace_back(full_range);
}

}  // namespace sm
}  // namespace tiledb
