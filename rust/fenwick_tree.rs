//! Fenwick (Binary Indexed) tree for zero-based point additions and sums.
//!
//! `prefix_sum(end_exclusive)` returns the sum over `[0, end_exclusive)`, where
//! the endpoint is in `0..=len()`. `range_sum(start, end)` returns the sum over
//! `[start, end)`, including empty ranges. Updates, prefix sums, and range sums
//! take O(log n) time. Construction uses linear Fenwick propagation in O(n),
//! and the tree uses O(n) storage.
//!
//! All values and sums use `i64`. Arithmetic is checked: construction and
//! operations return [`FenwickError::Overflow`] if a stored aggregate or an
//! intermediate sum/difference cannot be represented. A failed update leaves
//! the tree unchanged. Range sums are computed as two checked prefixes followed
//! by checked subtraction, so an overflowing intermediate also returns an
//! error even if the mathematical range total would fit in `i64`.

/// Errors returned by Fenwick tree operations.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum FenwickError {
    /// A point index is outside `0..len()`.
    InvalidIndex { index: usize, len: usize },
    /// A prefix or range endpoint is outside `0..=len()`.
    InvalidEndpoint { endpoint: usize, len: usize },
    /// A range starts after it ends.
    ReversedRange { start: usize, end: usize },
    /// Checked `i64` arithmetic could not represent a result.
    Overflow,
}

/// A Fenwick tree representing a sequence of signed 64-bit values.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct FenwickTree {
    len: usize,
    tree: Vec<i64>,
}

impl FenwickTree {
    /// Builds a tree from `values` using linear Fenwick propagation.
    ///
    /// Returns [`FenwickError::Overflow`] if any constructed node aggregate
    /// overflows `i64`.
    pub fn new(values: &[i64]) -> Result<Self, FenwickError> {
        let len = values.len();
        let mut tree = vec![0_i64; len + 1];

        for (offset, &value) in values.iter().enumerate() {
            let index = offset + 1;
            tree[index] = tree[index]
                .checked_add(value)
                .ok_or(FenwickError::Overflow)?;
            let parent = index + (index & index.wrapping_neg());
            if parent <= len {
                tree[parent] = tree[parent]
                    .checked_add(tree[index])
                    .ok_or(FenwickError::Overflow)?;
            }
        }

        Ok(Self { len, tree })
    }

    /// Returns the number of represented values.
    pub fn len(&self) -> usize {
        self.len
    }

    /// Returns whether the tree contains no values.
    pub fn is_empty(&self) -> bool {
        self.len == 0
    }

    /// Adds `delta` to the value at the zero-based `index`.
    ///
    /// Returns [`FenwickError::InvalidIndex`] if `index` is outside `0..len()`.
    /// On overflow, returns [`FenwickError::Overflow`] without changing the tree.
    pub fn add(&mut self, index: usize, delta: i64) -> Result<(), FenwickError> {
        if index >= self.len {
            return Err(FenwickError::InvalidIndex {
                index,
                len: self.len,
            });
        }

        // Check every affected node first to keep failed updates transactional.
        let mut changes = Vec::new();
        let mut node = index + 1;
        while node <= self.len {
            let updated = self.tree[node]
                .checked_add(delta)
                .ok_or(FenwickError::Overflow)?;
            changes.push((node, updated));
            node += node & node.wrapping_neg();
        }
        for (node, updated) in changes {
            self.tree[node] = updated;
        }
        Ok(())
    }

    /// Returns the sum over `[0, end_exclusive)`.
    ///
    /// The endpoint must be in `0..=len()`. Returns
    /// [`FenwickError::InvalidEndpoint`] for an invalid endpoint and
    /// [`FenwickError::Overflow`] if the sum cannot be represented as `i64`.
    pub fn prefix_sum(&self, end_exclusive: usize) -> Result<i64, FenwickError> {
        if end_exclusive > self.len {
            return Err(FenwickError::InvalidEndpoint {
                endpoint: end_exclusive,
                len: self.len,
            });
        }

        let mut sum = 0_i64;
        let mut node = end_exclusive;
        while node > 0 {
            sum = sum
                .checked_add(self.tree[node])
                .ok_or(FenwickError::Overflow)?;
            node -= node & node.wrapping_neg();
        }
        Ok(sum)
    }

    /// Returns the sum over the half-open range `[start, end)`.
    ///
    /// Both endpoints must be in `0..=len()` and `start` must not exceed `end`.
    /// Empty ranges return zero. Invalid endpoints, reversed ranges, or checked
    /// arithmetic overflow are reported with their corresponding error variants.
    pub fn range_sum(&self, start: usize, end: usize) -> Result<i64, FenwickError> {
        self.validate_endpoint(start)?;
        self.validate_endpoint(end)?;
        if start > end {
            return Err(FenwickError::ReversedRange { start, end });
        }
        self.prefix_sum(end)?
            .checked_sub(self.prefix_sum(start)?)
            .ok_or(FenwickError::Overflow)
    }

    fn validate_endpoint(&self, endpoint: usize) -> Result<(), FenwickError> {
        if endpoint > self.len {
            Err(FenwickError::InvalidEndpoint {
                endpoint,
                len: self.len,
            })
        } else {
            Ok(())
        }
    }
}

#[cfg(test)]
mod tests {
    use super::{FenwickError, FenwickTree};

    #[test]
    fn empty_tree_and_invalid_update() {
        let mut tree = FenwickTree::new(&[]).unwrap();
        assert_eq!(tree.len(), 0);
        assert!(tree.is_empty());
        assert_eq!(tree.prefix_sum(0), Ok(0));
        assert_eq!(tree.range_sum(0, 0), Ok(0));
        assert_eq!(
            tree.add(0, 1),
            Err(FenwickError::InvalidIndex { index: 0, len: 0 })
        );
        assert_eq!(
            tree.prefix_sum(1),
            Err(FenwickError::InvalidEndpoint { endpoint: 1, len: 0 })
        );
    }

    #[test]
    fn singleton_boundaries_and_update() {
        let mut tree = FenwickTree::new(&[7]).unwrap();
        assert_eq!(tree.len(), 1);
        assert!(!tree.is_empty());
        assert_eq!(tree.prefix_sum(0), Ok(0));
        assert_eq!(tree.prefix_sum(1), Ok(7));
        assert_eq!(tree.range_sum(0, 1), Ok(7));
        assert_eq!(tree.range_sum(1, 1), Ok(0));
        tree.add(0, -10).unwrap();
        assert_eq!(tree.prefix_sum(1), Ok(-3));
    }

    #[test]
    fn build_prefix_ranges_and_point_updates() {
        let mut tree = FenwickTree::new(&[2, -1, 3, 5, 4]).unwrap();
        assert_eq!(tree.len(), 5);
        assert_eq!(tree.prefix_sum(0), Ok(0));
        assert_eq!(tree.prefix_sum(3), Ok(4));
        assert_eq!(tree.prefix_sum(5), Ok(13));
        assert_eq!(tree.range_sum(0, 5), Ok(13));
        assert_eq!(tree.range_sum(1, 4), Ok(7));
        assert_eq!(tree.range_sum(2, 3), Ok(3));
        assert_eq!(tree.range_sum(3, 3), Ok(0));

        tree.add(2, 7).unwrap();
        assert_eq!(tree.prefix_sum(3), Ok(11));
        assert_eq!(tree.range_sum(1, 4), Ok(14));
        assert_eq!(tree.range_sum(0, 5), Ok(20));
        tree.add(0, -4).unwrap();
        assert_eq!(tree.range_sum(0, 2), Ok(-3));
    }

    #[test]
    fn typed_index_and_range_errors() {
        let mut tree = FenwickTree::new(&[1, 2, 3]).unwrap();
        assert_eq!(
            tree.add(3, 1),
            Err(FenwickError::InvalidIndex { index: 3, len: 3 })
        );
        assert_eq!(
            tree.prefix_sum(4),
            Err(FenwickError::InvalidEndpoint {
                endpoint: 4,
                len: 3
            })
        );
        assert_eq!(
            tree.range_sum(0, 4),
            Err(FenwickError::InvalidEndpoint {
                endpoint: 4,
                len: 3
            })
        );
        assert_eq!(
            tree.range_sum(2, 1),
            Err(FenwickError::ReversedRange { start: 2, end: 1 })
        );
    }

    #[test]
    fn checked_overflow_and_failed_update_is_atomic() {
        assert_eq!(
            FenwickTree::new(&[i64::MAX, 1]),
            Err(FenwickError::Overflow)
        );

        let mut tree = FenwickTree::new(&[i64::MAX]).unwrap();
        assert_eq!(tree.add(0, 1), Err(FenwickError::Overflow));
        assert_eq!(tree.prefix_sum(1), Ok(i64::MAX));
    }
}
