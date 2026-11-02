#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <utility>
#include <vector>

// Merges closed integer intervals. A pair {start, end} denotes [start, end].
// Throws std::invalid_argument if any interval has start > end.
// Complexity: O(n log n) time for sorting; the merge pass is O(n) time and
// O(1) auxiliary space. It compacts the by-value input vector and returns that
// storage, avoiding a separate O(n) result allocation. std::sort's auxiliary
// storage is implementation-dependent (commonly O(log n) stack space).
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

int main() {
  using Interval = std::pair<int, int>;

  assert(merge_intervals({}).empty());
  assert((merge_intervals({{3, 7}}) == std::vector<Interval>{{3, 7}}));
  assert((merge_intervals({{1, 10}, {3, 5}, {2, 8}}) ==
          std::vector<Interval>{{1, 10}}));
  assert((merge_intervals({{2, 5}, {2, 5}, {2, 5}}) ==
          std::vector<Interval>{{2, 5}}));
  assert((merge_intervals({{1, 3}, {3, 5}, {4, 8}}) ==
          std::vector<Interval>{{1, 8}}));
  assert((merge_intervals({{8, 9}, {1, 2}, {4, 6}}) ==
          std::vector<Interval>{{1, 2}, {4, 6}, {8, 9}}));

  bool rejected_invalid_interval = false;
  try {
    (void)merge_intervals({{1, 2}, {5, 4}});
  } catch (const std::invalid_argument&) {
    rejected_invalid_interval = true;
  }
  assert(rejected_invalid_interval);

  return 0;
}
