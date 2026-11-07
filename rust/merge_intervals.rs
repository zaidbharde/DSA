use std::error::Error;
use std::fmt;

/// A closed integer interval `[start, end]`.
pub type Interval = (i32, i32);

/// The input interval at `index` has a start greater than its end.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct InvalidInterval {
    /// Zero-based position of the invalid interval in the input.
    pub index: usize,
    /// The invalid interval as supplied to [`merge_intervals`].
    pub interval: Interval,
}

impl fmt::Display for InvalidInterval {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(
            formatter,
            "interval at index {} has start greater than end: [{}, {}]",
            self.index, self.interval.0, self.interval.1
        )
    }
}

impl Error for InvalidInterval {}

/// Sorts and merges overlapping or touching closed intervals.
///
/// Returns an [`InvalidInterval`] if any interval has `start > end`. Intervals
/// are sorted lexicographically before merging, and closed intervals that share
/// an endpoint are merged (for example, `[1, 3]` and `[3, 5]`).
///
/// Complexity: `O(n log n)` time for sorting and `O(n)` time for the merge
/// pass; the returned result uses `O(n)` storage.
pub fn merge_intervals(
    intervals: Vec<Interval>,
) -> Result<Vec<Interval>, InvalidInterval> {
    for (index, &interval) in intervals.iter().enumerate() {
        if interval.0 > interval.1 {
            return Err(InvalidInterval { index, interval });
        }
    }

    let mut intervals = intervals;
    intervals.sort_unstable();

    let mut merged: Vec<Interval> = Vec::with_capacity(intervals.len());
    for (start, end) in intervals {
        if let Some(current) = merged.last_mut() {
            if start <= current.1 {
                current.1 = current.1.max(end);
                continue;
            }
        }
        merged.push((start, end));
    }

    Ok(merged)
}

#[cfg(test)]
mod tests {
    use super::{merge_intervals, InvalidInterval};

    #[test]
    fn empty_input_returns_empty_result() {
        assert_eq!(merge_intervals(vec![]), Ok(vec![]));
    }

    #[test]
    fn sorted_disjoint_input_is_preserved() {
        assert_eq!(
            merge_intervals(vec![(1, 2), (4, 6), (8, 9)]),
            Ok(vec![(1, 2), (4, 6), (8, 9)])
        );
    }

    #[test]
    fn unsorted_input_is_sorted() {
        assert_eq!(
            merge_intervals(vec![(8, 9), (1, 2), (4, 6)]),
            Ok(vec![(1, 2), (4, 6), (8, 9)])
        );
    }

    #[test]
    fn overlapping_intervals_are_merged() {
        assert_eq!(
            merge_intervals(vec![(1, 10), (3, 5), (2, 8)]),
            Ok(vec![(1, 10)])
        );
    }

    #[test]
    fn touching_closed_intervals_are_merged() {
        assert_eq!(
            merge_intervals(vec![(1, 3), (3, 5), (4, 8)]),
            Ok(vec![(1, 8)])
        );
    }

    #[test]
    fn duplicate_intervals_are_merged() {
        assert_eq!(
            merge_intervals(vec![(2, 5), (2, 5), (2, 5)]),
            Ok(vec![(2, 5)])
        );
    }

    #[test]
    fn negative_endpoints_are_supported() {
        assert_eq!(
            merge_intervals(vec![(-8, -5), (-6, -2), (0, 1)]),
            Ok(vec![(-8, -2), (0, 1)])
        );
    }

    #[test]
    fn invalid_interval_returns_typed_error_with_original_index() {
        assert_eq!(
            merge_intervals(vec![(1, 2), (5, 4)]),
            Err(InvalidInterval {
                index: 1,
                interval: (5, 4),
            })
        );
    }
}
