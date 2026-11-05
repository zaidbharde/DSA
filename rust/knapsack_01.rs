//! 0/1 knapsack with one-dimensional dynamic programming.
//!
//! Each item is used at most once. Negative-value items may be skipped, and
//! zero-weight items are considered exactly once. The algorithm runs in
//! O(nW) time and uses O(W) auxiliary space, where `n` is the number of items
//! and `W` is the capacity.

use std::fmt;

/// Errors that can occur while computing a 0/1 knapsack optimum.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum KnapsackError {
    /// The weights and values slices have different lengths.
    LengthMismatch,
    /// The capacity cannot be represented as an allocation size or allocated.
    CapacityTooLarge,
    /// A candidate total value does not fit in an `i64`.
    SumOverflow,
}

impl fmt::Display for KnapsackError {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::LengthMismatch => write!(formatter, "weights and values must have the same length"),
            Self::CapacityTooLarge => write!(formatter, "capacity is too large to size the dynamic program"),
            Self::SumOverflow => write!(formatter, "candidate value sum exceeds the i64 range"),
        }
    }
}

impl std::error::Error for KnapsackError {}

/// Computes the maximum value achievable without exceeding `capacity`.
///
/// Returns zero for empty inputs or when no positive-value item fits. Each
/// item is considered at most once, including zero-weight items. Returns
/// [`KnapsackError::LengthMismatch`] if the slices differ in length,
/// [`KnapsackError::CapacityTooLarge`] if the DP array cannot be sized, or
/// [`KnapsackError::SumOverflow`] if a candidate value addition overflows.
pub fn knapsack_01(
    weights: &[usize],
    values: &[i64],
    capacity: usize,
) -> Result<i64, KnapsackError> {
    if weights.len() != values.len() {
        return Err(KnapsackError::LengthMismatch);
    }

    let state_count = capacity
        .checked_add(1)
        .ok_or(KnapsackError::CapacityTooLarge)?;
    let mut best = Vec::new();
    best.try_reserve_exact(state_count)
        .map_err(|_| KnapsackError::CapacityTooLarge)?;
    best.resize(state_count, 0_i64);

    for (&weight, &value) in weights.iter().zip(values) {
        if weight > capacity {
            continue;
        }

        // Descending capacities prevent reusing the current item. The inclusive
        // range also visits each state exactly once for a zero-weight item.
        for current_capacity in (weight..=capacity).rev() {
            let candidate = best[current_capacity - weight]
                .checked_add(value)
                .ok_or(KnapsackError::SumOverflow)?;
            if candidate > best[current_capacity] {
                best[current_capacity] = candidate;
            }
        }
    }

    Ok(best[capacity])
}

#[cfg(test)]
mod tests {
    use super::{knapsack_01, KnapsackError};

    #[test]
    fn classic_example() {
        assert_eq!(knapsack_01(&[10, 20, 30], &[60, 100, 120], 50), Ok(220));
    }

    #[test]
    fn empty_items_return_zero() {
        assert_eq!(knapsack_01(&[], &[], 10), Ok(0));
    }

    #[test]
    fn zero_capacity_returns_zero_for_positive_weights() {
        assert_eq!(knapsack_01(&[1], &[5], 0), Ok(0));
    }

    #[test]
    fn zero_weight_items_are_used_once_each() {
        assert_eq!(knapsack_01(&[0, 0, 2], &[4, 3, 10], 2), Ok(17));
        assert_eq!(knapsack_01(&[0], &[5], 0), Ok(5));
    }

    #[test]
    fn oversized_items_are_ignored() {
        assert_eq!(knapsack_01(&[6, 7], &[9, 10], 5), Ok(0));
    }

    #[test]
    fn negative_values_can_be_skipped() {
        assert_eq!(knapsack_01(&[1, 2], &[-5, -1], 2), Ok(0));
        assert_eq!(knapsack_01(&[0, 1], &[-3, 5], 1), Ok(5));
    }

    #[test]
    fn mismatched_lengths_return_typed_error() {
        assert_eq!(
            knapsack_01(&[1], &[], 1),
            Err(KnapsackError::LengthMismatch)
        );
    }

    #[test]
    fn unrepresentable_capacity_sizing_returns_typed_error() {
        assert_eq!(
            knapsack_01(&[], &[], usize::MAX),
            Err(KnapsackError::CapacityTooLarge)
        );
    }

    #[test]
    fn overflowing_candidate_sum_returns_typed_error() {
        assert_eq!(
            knapsack_01(&[0, 0], &[i64::MAX, 1], 0),
            Err(KnapsackError::SumOverflow)
        );
    }
}
