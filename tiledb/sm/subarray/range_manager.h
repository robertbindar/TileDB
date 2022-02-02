/**
 * @file   range_manager.h
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

#ifndef TILEDB_RANGE_MANAGER_H
#define TILEDB_RANGE_MANAGER_H

#include "tiledb/common/heap_memory.h"
#include "tiledb/sm/enums/datatype.h"
#include "tiledb/sm/misc/types.h"

#include <optional>
#include <vector>

using namespace tiledb::common;

namespace tiledb {
namespace sm {

namespace detail {

struct AddRangeStrategy {
  virtual ~AddRangeStrategy() = default;
  virtual void add_range(
      std::vector<Range>* const ranges, Range& new_range) = 0;
};

struct BasicAddStrategy : public AddRangeStrategy {
  BasicAddStrategy() = default;
  ~BasicAddStrategy() = default;
  void add_range(std::vector<Range>* const ranges, Range& new_range) override;
};

template <typename T>
struct CoalescingAddStrategy : public AddRangeStrategy {
  CoalescingAddStrategy() = default;
  ~CoalescingAddStrategy() = default;
  void add_range(std::vector<Range>* const ranges, Range& new_range) override {
    assert(!(ranges->empty()));

    // If the start index of `range` immediately follows the end of the
    // last range on `ranges`, they are contiguous and will be coalesced.
    Range& last_range = ranges->back();
    const bool contiguous_after =
        *static_cast<const T*>(last_range.end()) !=
            std::numeric_limits<T>::max() &&
        *static_cast<const T*>(last_range.end()) + 1 ==
            *static_cast<const T*>(new_range.start());

    // Coalesce `range` with `last_range` if they are contiguous.
    if (contiguous_after) {
      last_range.set_end(new_range.end());
    } else {
      ranges->emplace_back(new_range);
    }
  };
};

}  // namespace detail

class RangeManager {
 public:
  /* ********************************* */
  /*     CONSTRUCTORS & DESTRUCTORS    */
  /* ********************************* */

  /** Destructor. */
  virtual ~RangeManager() = default;

  // /**
  //  * Adds a range to the range manager. If a default range manager strategy
  //  is,
  //  * then first update the range strategy.
  //  *
  //  * @param ranges The current ranges in the subarray (remove after
  //  refactor).
  //  * @param new_range The range to add.
  //  */
  // // virtual Status add_range(
  // //   std::vector<std::vector<Range>>& ranges, Range&& new_range) = 0;

  /**
   * Adds a range to the range manager without performing any checkes. If a
   * default strategy is set, then first update the range strategy.
   *
   * @param ranges The current ranges in the subarray (remove after
   refactor).
   * @param new_range The range to add.
   */
  virtual Status add_range_unsafe(
      std::vector<std::vector<Range>>& ranges, Range&& new_range) = 0;

  /**
   * Returns ``true`` if the current range is the default range.
   *
   * The default range is the full range for the dimension the range is
   * managing.
   **/
  virtual bool is_default() const = 0;

  /** Returns ``true`` if the current range manager is for a dimension label.*/
  virtual bool is_label_manager() const = 0;
};

template <typename T>
class DimensionRangeManager : public RangeManager {
 public:
  /* ********************************* */
  /*     CONSTRUCTORS & DESTRUCTORS    */
  /* ********************************* */

  /** Disable default constructor. */
  DimensionRangeManager() = delete;

  /**
   * Constructor for the default RangeManager.
   *
   * This will set the range to the full domain. No new ranges can be added.
   */
  DimensionRangeManager(
      uint32_t dim_index,
      Range& bounds,
      std::vector<std::vector<Range>>& ranges)
      : bounds_(bounds)
      , dim_index_(dim_index)
      , is_default_(true) {
    ranges[dim_index].clear();
    ranges[dim_index].emplace_back(bounds);
  }

  /** Destructor. */
  ~DimensionRangeManager() = default;

  // //  Status add_range(std::vector<std::vector<Range>>& ranges, Range&&
  // //  new_range, error_on_oob) {
  // //    if (!error_on_oob)
  // //      RETURN_NOT_OK(adjust_range_oob(&new_range));
  // //    RETURN_NOT_OK(check_range(&new_range));
  // //    add_range_unsafe(ranges, new_range);
  // //  };

  Status add_range_unsafe(
      std::vector<std::vector<Range>>& ranges, Range&& new_range) override {
    if (ranges[dim_index_].empty()) {
      ranges[dim_index_].emplace_back(new_range);
      return Status::Ok();
    }
    if (!add_strategy_) {
      // TODO: This should be replaced with a failed status code. Right now it
      // matches the previous implementation, but it can lead to more than one
      // range per dimension for a subarray with global order or default range
      // (which is an unexpected state).
      ranges[dim_index_].emplace_back(new_range);
      return Status::Ok();
    }
    add_strategy_->add_range(&ranges[dim_index_], new_range);
    return Status::Ok();
  }

  bool is_default() const override {
    return is_default_;
  };

  bool is_label_manager() const override {
    return label_name_.has_value();
  };

 private:
  /* ********************************* */
  /*         PRIVATE ATTRIBUTES        */
  /* ********************************* */

  /** Maximum possible range. */
  Range bounds_;

  /** Dimension index of the dimension this is managing the range for. */
  uint32_t dim_index_;

  /**
   * Label name this dimension of the dimension label this RangeManager is
   * managing the
   */
  std::optional<std::string> label_name_ = std::nullopt;

  /**
   * If ``true``, the range contains the full domain for the dimension (the
   * default value for a subarray before any other values is set. Otherwise,
   * some values has been explicitly set to the range.
   */
  bool is_default_ = true;

  /** Strategy for adding ranges. */
  tdb_shared_ptr<detail::AddRangeStrategy> add_strategy_;
};

/* Create default RangeManager. */
tdb_shared_ptr<RangeManager> create_range_manager(
    Datatype datatype,
    uint32_t dim_index,
    Range& range_bounds,
    std::vector<std::vector<Range>>& ranges);

}  // namespace sm
}  // namespace tiledb

#endif
