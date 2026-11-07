#include <algorithm>
#include <cassert>
#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

// Merges closed integer intervals. A pair {start, end} denotes [start, end].
// Touching intervals are merged; throws std::invalid_argument if any interval
// has start > end. Complexity: O(n log n) time (sorting dominates), O(n) time
// for validation and merging, and O(1) auxiliary space for the in-place merge
// pass, excluding std::sort's implementation-dependent auxiliary storage
// (commonly O(log n) stack space). The by-value input vector is compacted and
// returned, avoiding a separate O(n) result allocation.
std::vector<std::pair<int, int>> merge_intervals(
    std::vector<std::pair<int, int>> intervals) {
  for (const auto& interval : intervals) {
    if (interval.first > interval.second) {
      throw std::invalid_argument("interval start must not exceed its end");
    }
  }

  std::sort(intervals.begin(), intervals.end());
  std::size_t merged_size = 0;
  for (std::size_t i = 0; i < intervals.size(); ++i) {
    const auto next = intervals[i];
    if (merged_size == 0) {
      intervals[merged_size++] = next;
      continue;
    }

    auto& current = intervals[merged_size - 1];
    // Closed intervals overlap when the next start is at most the current end.
    if (next.first <= current.second) {
      current.second = std::max(current.second, next.second);
    } else {
      intervals[merged_size++] = next;
    }
  }

  intervals.resize(merged_size);
  return intervals;
}

namespace {

using Interval = std::pair<int, int>;
using Intervals = std::vector<Interval>;

// Deliberately use a separate result vector and a simple append-based merge as
// the test oracle, rather than duplicating the production in-place compaction.
Intervals reference_merge(Intervals intervals) {
  for (const auto& interval : intervals) {
    if (interval.first > interval.second) {
      throw std::invalid_argument("invalid interval in reference input");
    }
  }

  std::sort(intervals.begin(), intervals.end());
  Intervals result;
  for (const auto& interval : intervals) {
    if (result.empty() || interval.first > result.back().second) {
      result.push_back(interval);
    } else if (interval.second > result.back().second) {
      result.back().second = interval.second;
    }
  }
  return result;
}

void verify_against_reference(const Intervals& input) {
  const Intervals actual = merge_intervals(input);
  assert(actual == reference_merge(input));
  assert(merge_intervals(actual) == actual);
  for (std::size_t i = 1; i < actual.size(); ++i) {
    // The result is sorted, and distinct output intervals cannot overlap or
    // touch because the intervals are closed and touching ranges are merged.
    assert(actual[i - 1].first <= actual[i].first);
    assert(actual[i - 1].second < actual[i].first);
  }
}

void verify_invalid_input_is_rejected(const Intervals& input) {
  bool rejected = false;
  try {
    (void)merge_intervals(input);
  } catch (const std::invalid_argument&) {
    rejected = true;
  }
  assert(rejected);
}

void test_exhaustive_short_sequences() {
  // All valid integer intervals over this small domain, including singletons.
  const int endpoints[] = {-2, -1, 0, 1, 2};
  Intervals interval_choices;
  for (int start : endpoints) {
    for (int end : endpoints) {
      if (start <= end) {
        interval_choices.emplace_back(start, end);
      }
    }
  }

  // Enumerate every ordered sequence of up to four choices. Enumeration as
  // sequences intentionally includes duplicates and every input ordering.
  std::size_t sequence_count = 1;
  for (std::size_t length = 0; length <= 4; ++length) {
    for (std::size_t code = 0; code < sequence_count; ++code) {
      Intervals input(length);
      std::size_t remaining = code;
      for (std::size_t position = 0; position < length; ++position) {
        input[position] = interval_choices[remaining % interval_choices.size()];
        remaining /= interval_choices.size();
      }
      verify_against_reference(input);
    }
    sequence_count *= interval_choices.size();
  }
}

void test_boundary_intervals() {
  const int minimum = std::numeric_limits<int>::min();
  const int maximum = std::numeric_limits<int>::max();
  verify_against_reference({{maximum, maximum}, {minimum, minimum}});
  verify_against_reference({{minimum, 0}, {0, maximum}});
  verify_against_reference({{minimum, minimum}, {minimum, maximum},
                            {maximum, maximum}});
  assert((merge_intervals({{maximum, maximum}, {minimum, minimum}}) ==
          Intervals{{minimum, minimum}, {maximum, maximum}}));
  assert((merge_intervals({{minimum, 0}, {0, maximum}}) ==
          Intervals{{minimum, maximum}}));
}

void test_invalid_ranges() {
  verify_invalid_input_is_rejected({{1, 2}, {5, 4}});
  verify_invalid_input_is_rejected({{2, 1}});
  verify_invalid_input_is_rejected(
      {{std::numeric_limits<int>::max(), std::numeric_limits<int>::min()}});
  // Invalid ranges are rejected even when valid intervals precede or follow
  // them, rather than being silently discarded during sorting or merging.
  verify_invalid_input_is_rejected({{-2, -1}, {3, 2}, {4, 5}});
  verify_invalid_input_is_rejected({{-2, -1}, {4, 5}, {3, 2}});
}

}  // namespace

int main() {
  verify_against_reference({});
  verify_against_reference({{3, 7}});
  verify_against_reference({{1, 10}, {3, 5}, {2, 8}});
  verify_against_reference({{2, 5}, {2, 5}, {2, 5}});
  verify_against_reference({{1, 3}, {3, 5}, {4, 8}});
  verify_against_reference({{8, 9}, {1, 2}, {4, 6}});
  verify_against_reference({{1, 2}, {4, 5}});  // Strictly separated.
  verify_against_reference({{5, 5}, {5, 5}, {4, 5}});  // Touching singletons.

  test_exhaustive_short_sequences();
  test_boundary_intervals();
  test_invalid_ranges();
  return 0;
}
