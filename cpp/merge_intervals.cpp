#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <utility>
#include <vector>

// Merges closed integer intervals. A pair {start, end} denotes [start, end].
// Throws std::invalid_argument if any interval has start > end.
// Complexity: O(n log n) time for sorting and O(n) auxiliary space for the
// result (the sort itself uses O(log n) stack space).
std::vector<std::pair<int, int>> merge_intervals(
    std::vector<std::pair<int, int>> intervals) {
  for (const auto& interval : intervals) {
    if (interval.first > interval.second) {
      throw std::invalid_argument("interval start must not exceed its end");
    }
  }

  if (intervals.empty()) {
    return {};
  }

  std::sort(intervals.begin(), intervals.end());
  std::vector<std::pair<int, int>> merged;
  merged.reserve(intervals.size());
  merged.push_back(intervals.front());

  for (std::size_t i = 1; i < intervals.size(); ++i) {
    const auto& next = intervals[i];
    auto& current = merged.back();
    // Closed intervals overlap when the next start is at most the current end.
    if (next.first <= current.second) {
      current.second = std::max(current.second, next.second);
    } else {
      merged.push_back(next);
    }
  }

  return merged;
}

int main() {
  using Interval = std::pair<int, int>;

  assert(merge_intervals({}).empty());
  assert((merge_intervals({{3, 7}}) == std::vector<Interval>{{3, 7}}));
  assert((merge_intervals({{1, 10}, {3, 5}, {2, 8}}) ==
          std::vector<Interval>{{1, 10}}));
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
