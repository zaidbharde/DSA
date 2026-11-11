//! In-place 0-based kth-smallest selection with an introspective fallback.
//!
//! Average time is O(n); the depth-limited `sort_unstable` fallback bounds
//! worst-case time by O(n log n). Selection uses O(1) auxiliary space and
//! the fallback sort uses O(log n) stack space.

/// Returns the value at 0-based rank `k` in `values`, reordering the slice.
///
/// Returns `None` when `k` is outside the slice. Duplicate values are handled
/// by a three-way partition, so every valid rank selects the correct value.
pub fn kth_smallest(values: &mut [i64], k: usize) -> Option<i64> {
    if k >= values.len() {
        return None;
    }

    let mut left = 0;
    let mut right = values.len(); // Exclusive.
    let mut depth_budget = 2 * (usize::BITS - values.len().leading_zeros()) as usize;

    loop {
        if depth_budget == 0 {
            values[left..right].sort_unstable();
            return Some(values[k]);
        }
        depth_budget -= 1;

        let middle = left + (right - left) / 2;
        let pivot = median_of_three(values[left], values[middle], values[right - 1]);

        // After partitioning, [left, less_end) < pivot,
        // [less_end, greater_begin) == pivot, and [greater_begin, right) > pivot.
        let mut less_end = left;
        let mut scan = left;
        let mut greater_begin = right;
        while scan < greater_begin {
            if values[scan] < pivot {
                values.swap(less_end, scan);
                less_end += 1;
                scan += 1;
            } else if values[scan] > pivot {
                greater_begin -= 1;
                values.swap(scan, greater_begin);
            } else {
                scan += 1;
            }
        }

        if k < less_end {
            right = less_end;
        } else if k >= greater_begin {
            left = greater_begin;
        } else {
            return Some(pivot);
        }
    }
}

fn median_of_three(a: i64, b: i64, c: i64) -> i64 {
    if a < b {
        if b < c {
            b
        } else if a < c {
            c
        } else {
            a
        }
    } else if a < c {
        a
    } else if b < c {
        c
    } else {
        b
    }
}

#[cfg(test)]
mod tests {
    use super::kth_smallest;

    #[test]
    fn empty_and_invalid_ranks_return_none() {
        assert_eq!(kth_smallest(&mut [], 0), None);
        assert_eq!(kth_smallest(&mut [3, 1], 2), None);
        assert_eq!(kth_smallest(&mut [3, 1], usize::MAX), None);
    }

    #[test]
    fn single_element() {
        assert_eq!(kth_smallest(&mut [42], 0), Some(42));
    }

    #[test]
    fn duplicates_and_extreme_values() {
        let mut values = [4, 1, 4, 2, 4, 2, 1];
        assert_eq!(kth_smallest(&mut values, 3), Some(2));

        let mut extremes = [i64::MAX, 0, i64::MIN, i64::MAX, -1, i64::MIN];
        assert_eq!(kth_smallest(&mut extremes, 2), Some(-1));
    }

    #[test]
    fn sorted_and_reverse_sorted_inputs() {
        let mut sorted = [1, 2, 3, 4, 5, 6];
        assert_eq!(kth_smallest(&mut sorted, 2), Some(3));

        let mut reverse = [6, 5, 4, 3, 2, 1];
        assert_eq!(kth_smallest(&mut reverse, 3), Some(4));
    }

    #[test]
    fn deterministic_differential_checks_against_sorted_copies() {
        let mut state = 0x9e37_79b9_7f4a_7c15_u64;
        for len in 0..=96 {
            let mut original = Vec::with_capacity(len);
            for _ in 0..len {
                // A fixed-seed generator keeps this differential test reproducible.
                state = state
                    .wrapping_mul(6_364_136_223_846_793_005)
                    .wrapping_add(1_442_695_040_888_963_407);
                let value = ((state >> 32) % 41) as i64 - 20;
                original.push(value);
            }

            let mut expected = original.clone();
            expected.sort_unstable();
            for k in 0..len {
                let mut actual = original.clone();
                assert_eq!(
                    kth_smallest(&mut actual, k),
                    Some(expected[k]),
                    "mismatch for length {len}, rank {k}, input {original:?}"
                );
            }
        }
    }
}
