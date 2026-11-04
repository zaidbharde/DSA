//! Maximum-sum non-empty contiguous subarray using Kadane's algorithm.
//!
//! Indices are zero-based and inclusive. Equal sums prefer the earliest start,
//! then the earliest end. The algorithm runs in O(n) time and uses O(1)
//! auxiliary storage.

/// A maximum-sum non-empty contiguous subarray.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct MaximumSubarray {
    pub sum: i64,
    pub start: usize,
    pub end: usize,
}

/// The maximum subarray sum cannot be represented as an `i64`.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct SumOverflow;

/// Finds the maximum-sum non-empty contiguous subarray.
///
/// Returns `Ok(None)` for empty input and `Err(SumOverflow)` if the maximum
/// subarray sum exceeds `i64::MAX`. The returned sum is otherwise representable
/// as an `i64`. In particular, a negative underflowing extension is discarded
/// in favor of starting a new subarray at the current element.
pub fn maximum_subarray(values: &[i64]) -> Result<Option<MaximumSubarray>, SumOverflow> {
    let Some(&first) = values.first() else {
        return Ok(None);
    };

    let mut current_sum = first;
    let mut current_start = 0;
    let mut best = MaximumSubarray {
        sum: first,
        start: 0,
        end: 0,
    };

    for (index, &value) in values.iter().enumerate().skip(1) {
        let (extended_sum, extended_start) = match current_sum.checked_add(value) {
            Some(sum) if sum < value => (value, index),
            Some(sum) => (sum, current_start),
            None if current_sum > 0 => return Err(SumOverflow),
            None => (value, index),
        };

        current_sum = extended_sum;
        current_start = extended_start;

        if current_sum > best.sum
            || (current_sum == best.sum
                && (current_start < best.start
                    || (current_start == best.start && index < best.end)))
        {
            best = MaximumSubarray {
                sum: current_sum,
                start: current_start,
                end: index,
            };
        }
    }

    Ok(Some(best))
}

#[cfg(test)]
mod tests {
    use super::{maximum_subarray, MaximumSubarray, SumOverflow};

    fn expect(values: &[i64], sum: i64, start: usize, end: usize) {
        assert_eq!(
            maximum_subarray(values),
            Ok(Some(MaximumSubarray { sum, start, end }))
        );
    }

    #[test]
    fn mixed_values() {
        expect(&[-2, 1, -3, 4, -1, 2, 1, -5, 4], 6, 3, 6);
    }

    #[test]
    fn all_negative() {
        expect(&[-8, -3, -6, -2, -5], -2, 3, 3);
    }

    #[test]
    fn singleton_i64_min() {
        expect(&[i64::MIN], i64::MIN, 0, 0);
    }

    #[test]
    fn ties_prefer_earliest_start() {
        expect(&[1, -1, 2], 2, 0, 2);
    }

    #[test]
    fn ties_prefer_earliest_end() {
        expect(&[2, -2, 2], 2, 0, 0);
    }

    #[test]
    fn empty_input_returns_none() {
        assert_eq!(maximum_subarray(&[]), Ok(None));
    }

    #[test]
    fn negative_underflowing_extension_can_be_recovered_from() {
        expect(&[i64::MIN, -1], -1, 1, 1);
    }

    #[test]
    fn largest_representable_sum() {
        expect(&[i64::MAX, 0], i64::MAX, 0, 0);
    }

    #[test]
    fn positive_overflow_returns_error() {
        assert_eq!(maximum_subarray(&[i64::MAX, 1]), Err(SumOverflow));
    }
}
