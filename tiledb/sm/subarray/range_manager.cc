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

using namespace tiledb::common;

namespace tiledb {
namespace sm {

namespace detail {
void BasicAddStrategy::add_range(
    std::vector<Range>* const ranges, Range& new_range) {
  ranges->emplace_back(new_range);
}
}  // namespace detail

tdb_shared_ptr<RangeManager> create_range_manager(
    Datatype datatype,
    uint32_t dim_index,
    Range& range_bounds,
    std::vector<std::vector<Range>>& ranges) {
  switch (datatype) {
    case Datatype::INT8:
      return make_shared<DimensionRangeManager<int8_t>>(
          HERE(), dim_index, range_bounds, ranges);
    case Datatype::UINT8:
      return make_shared<DimensionRangeManager<uint8_t>>(
          HERE(), dim_index, range_bounds, ranges);
    case Datatype::INT16:
      return make_shared<DimensionRangeManager<int16_t>>(
          HERE(), dim_index, range_bounds, ranges);
    case Datatype::UINT16:
      return make_shared<DimensionRangeManager<uint16_t>>(
          HERE(), dim_index, range_bounds, ranges);
    case Datatype::INT32:
      return make_shared<DimensionRangeManager<int32_t>>(
          HERE(), dim_index, range_bounds, ranges);
    case Datatype::UINT32:
      return make_shared<DimensionRangeManager<uint32_t>>(
          HERE(), dim_index, range_bounds, ranges);
    case Datatype::INT64:
      return make_shared<DimensionRangeManager<int64_t>>(
          HERE(), dim_index, range_bounds, ranges);
    case Datatype::UINT64:
      return make_shared<DimensionRangeManager<uint64_t>>(
          HERE(), dim_index, range_bounds, ranges);
    case Datatype::FLOAT32:
      return make_shared<DimensionRangeManager<float>>(
          HERE(), dim_index, range_bounds, ranges);
    case Datatype::FLOAT64:
      return make_shared<DimensionRangeManager<double>>(
          HERE(), dim_index, range_bounds, ranges);
      break;
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
      return make_shared<DimensionRangeManager<int64_t>>(
          HERE(), dim_index, range_bounds, ranges);
    case Datatype::CHAR:
      return make_shared<DimensionRangeManager<char>>(
          HERE(), dim_index, range_bounds, ranges);
    case Datatype::STRING_ASCII:
    case Datatype::STRING_UTF8:
    case Datatype::STRING_UTF16:
    case Datatype::STRING_UTF32:
    case Datatype::STRING_UCS2:
    case Datatype::STRING_UCS4:
      return make_shared<DimensionRangeManager<std::string>>(
          HERE(), dim_index, range_bounds, ranges);
    case Datatype::ANY:
      return make_shared<DimensionRangeManager<uint8_t>>(
          HERE(), dim_index, range_bounds, ranges);
    default:
      LOG_FATAL("Unexpected datatype " + datatype_str(datatype));
      return nullptr;
  }
}

}  // namespace sm
}  // namespace tiledb
