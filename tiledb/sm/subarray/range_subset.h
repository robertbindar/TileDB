/**
 * @file   range_subset.h
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

#ifndef TILEDB_RANGE_SUBSET_H
#define TILEDB_RANGE_SUBSET_H

#include "tiledb/common/heap_memory.h"
#include "tiledb/sm/enums/datatype.h"
#include "tiledb/sm/misc/parallel_functions.h"
#include "tiledb/sm/misc/types.h"

#include <optional>
#include <type_traits>
#include <vector>

using namespace tiledb::common;

namespace tiledb {
namespace sm {

namespace detail {

/** Default add strategy: simple add. */
template <bool Coalesce, typename T, typename Enable = T>
struct AddStrategy {
  static Status add_range(std::vector<Range>& ranges, const Range& new_range) {
    ranges.emplace_back(new_range);
    return Status::Ok();
  };
};

/** Specialization for coalescing integer-type ranges. */
template <typename T>
struct AddStrategy<
    true,
    T,
    typename std::enable_if<std::is_integral<T>::value, T>::type> {
  static Status add_range(std::vector<Range>& ranges, const Range& new_range) {
    if (ranges.empty()) {
      ranges.emplace_back(new_range);
      return Status::Ok();
    }

    // If the start index of `range` immediately follows the end of the
    // last range on `ranges`, they are contiguous and will be coalesced.
    Range& last_range = ranges.back();
    const bool contiguous_after =
        *static_cast<const T*>(last_range.end()) !=
            std::numeric_limits<T>::max() &&
        *static_cast<const T*>(last_range.end()) + 1 ==
            *static_cast<const T*>(new_range.start());

    // Coalesce `range` with `last_range` if they are contiguous.
    if (contiguous_after) {
      last_range.set_end(new_range.end());
    } else {
      ranges.emplace_back(new_range);
    }
    return Status::Ok();
  };
};

/**
 * Sort algorithm for ranges.
 *
 * Default behavior: sorting is not enable.
 */
template <Datatype D, typename T, typename Enable = T>
struct SortStrategy {
  static Status sort(ThreadPool* const, std::vector<Range>&) {
    return LOG_STATUS(Status_SubarrayError(
        "Invalid datatype " + datatype_str(D) + " for sorting."));
  };
};

template <Datatype D, typename T>
struct SortStrategy<
    D,
    T,
    typename std::enable_if<std::is_arithmetic<T>::value, T>::type> {
  static Status sort(ThreadPool* const compute_tp, std::vector<Range>& ranges) {
    parallel_sort(
        compute_tp,
        ranges.begin(),
        ranges.end(),
        [&](const Range& a, const Range& b) {
          const T* a_data = static_cast<const T*>(a.start());
          const T* b_data = static_cast<const T*>(b.start());
          return a_data[0] < b_data[0] ||
                 (a_data[0] == b_data[0] && a_data[1] < b_data[1]);
        });
    return Status::Ok();
  };
};

template <Datatype D>
struct SortStrategy<D, char, char> {
  static Status sort(ThreadPool* const, std::vector<Range>&) {
    return LOG_STATUS(Status_SubarrayError(
        "Invalid datatype " + datatype_str(D) + " for sorting."));
  };
};

template <>
struct SortStrategy<Datatype::STRING_ASCII, std::string, std::string> {
  static Status sort(ThreadPool* const compute_tp, std::vector<Range>& ranges) {
    parallel_sort(
        compute_tp,
        ranges.begin(),
        ranges.end(),
        [&](const Range& a, const Range& b) {
          return a.start_str() < b.start_str() ||
                 (a.start_str() == b.start_str() && a.end_str() < b.end_str());
        });
    return Status::Ok();
  };
};

class RangeSubsetInternals {
 public:
  /* ********************************* */
  /*     CONSTRUCTORS & DESTRUCTORS    */
  /* ********************************* */

  /** Destructor. */
  virtual ~RangeSubsetInternals() = default;

  /**
   * Adds a range to the range manager without performing any checkes. If a
   * default strategy is set, then first update the range strategy.
   *
   * @param ranges The current ranges in the subarray (remove after
   refactor).
   * @param new_range The range to add.
   */
  virtual Status add_range_unsafe(
      std::vector<Range>& ranges, const Range& range) = 0;

  /**
   * Sorts the ranges in the range manager.
   *
   * @param compute_tp
   */
  virtual Status sort_ranges(
      ThreadPool* const compute_tp, std::vector<Range>& ranges) = 0;
};

template <typename T, Datatype D, bool CoalesceAdds>
class RangeSubsetInternalsImpl : public RangeSubsetInternals {
 private:
  using AddRange = AddStrategy<CoalesceAdds, T>;
  using SortRanges = SortStrategy<D, T>;

 public:
  Status add_range_unsafe(
      std::vector<Range>& ranges, const Range& new_range) override {
    return AddRange::add_range(ranges, new_range);
  };

  Status sort_ranges(
      ThreadPool* const compute_tp, std::vector<Range>& ranges) override {
    return SortRanges::sort(compute_tp, ranges);
  };
};

}  // namespace detail

class RangeSubset {
 private:
  tdb_shared_ptr<detail::RangeSubsetInternals> impl_ = nullptr;
  /** Maximum possible range. */
  Range full_range_ = Range();

  /**
   * If ``true``, the range contains the full domain for the dimension (the
   * default value for a subarray before any other values is set. Otherwise,
   * some values has been explicitly set to the range.
   */
  bool is_default_ = true;

  /** Stored ranges. */
  std::vector<Range> ranges_{};

 public:
  /* ********************************* */
  /*     CONSTRUCTORS & DESTRUCTORS    */
  /* ********************************* */

  /** Default constructor. */
  RangeSubset() = default;

  /** General constructor. */
  RangeSubset(
      Datatype datatype,
      const Range& full_range,
      bool is_default,
      bool coalesce_ranges);

  /** Destructor. */
  ~RangeSubset() = default;

  /**
   * Adds a range to the range manager without performing any checkes. If a
   * default strategy is set, then first update the range strategy.
   *
   * @param ranges The current ranges in the subarray (remove after
   refactor).
   * @param new_range The range to add.
   */
  Status add_range_unsafe(const Range& range) {
    if (is_default_) {
      ranges_.clear();
      is_default_ = false;
    }
    return impl_->add_range_unsafe(ranges_, range);
  }

  const Range& get_range(const uint64_t range_index) const {
    return ranges_[range_index];
  };

  const std::vector<Range>& get_ranges() const {
    return ranges_;
  };

  /**
   * Returns ``true`` if the current range is the default range.
   *
   * The default range is the full range for the dimension the range is
   * managing.
   **/
  bool is_default() const {
    return is_default_;
  };

  /** Returns ``true`` if the range subset is the empty set. */
  bool is_empty() const {
    return ranges_.empty();
  };

  /**
   * Returns ``false`` if the subset contains a range other than the default
   * range.
   *
   * IMPORTANT: This method is different from the `is_set` method of the
   *Subarray. The Subarray class only checks if any ranges are not default
   *whereas this method considers the RangeSubset to be unset if it is cleared.
   **/
  bool is_set() const {
    return !is_default_ && !ranges_.empty();
  };

  /**
   * Returns ``true`` if there is exactly one unary ranage in the subset.
   */
  bool is_unary() const {
    if (ranges_.size() != 1)
      return false;
    return ranges_[0].unary();
  };

  /** Returns the number of distinct ranges stored in the range manager. */
  uint64_t num_ranges() const {
    return ranges_.size();
  };

  /**
   * Sorts the ranges in the range manager.
   *
   * @param compute_tp
   */
  Status sort_ranges(ThreadPool* const compute_tp) {
    return impl_->sort_ranges(compute_tp, ranges_);
  };
};

}  // namespace sm
}  // namespace tiledb

#endif
