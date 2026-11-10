//! Immutable sparse table for range-minimum queries over `i64` values.
//!
//! Query bounds are zero-based and half-open: `query(start, end)` considers
//! the nonempty range `[start, end)`. Construction accepts an empty slice, but
//! every query on an empty table (and every empty query range) returns `Err`.
//! Out-of-bounds and reversed ranges also return `Err`.
//!
//! Building and storing the table take O(n log n) time and space. A valid
//! query takes O(1) time.

/// Immutable sparse table for range-minimum queries.
///
/// The input is copied during construction, and no mutation operation is
/// provided. All stored levels and values remain private.
#[derive(Debug, Clone)]
pub struct SparseTable {
    len: usize,
    levels: Vec<Vec<i64>>,
}

impl SparseTable {
    /// Build a sparse table from `values`.
    ///
    /// An empty input is valid; queries against it return `Err`.
    pub fn new(values: &[i64]) -> Self {
        let len = values.len();
        let mut levels = Vec::new();

        if len > 0 {
            levels.push(values.to_vec());
            let mut span = 2usize;

            while span <= len {
                let half_span = span / 2;
                let previous = levels.last().expect("the base level was inserted");
                let level_len = len - span + 1;
                let mut current = Vec::with_capacity(level_len);

                for start in 0..level_len {
                    current.push(previous[start].min(previous[start + half_span]));
                }

                levels.push(current);
                match span.checked_mul(2) {
                    Some(next_span) => span = next_span,
                    None => break,
                }
            }
        }

        Self { len, levels }
    }

    /// Return the minimum value in the nonempty half-open range `[start, end)`.
    ///
    /// Returns `Err` if `start >= end` or if either bound lies outside
    /// `0..=len`. Since bounds use `usize`, fractional or negative endpoints
    /// cannot be passed to this method.
    pub fn query(&self, start: usize, end: usize) -> Result<i64, &'static str> {
        if start >= end {
            return Err("query range must be nonempty and forward");
        }
        if end > self.len {
            return Err("query range end is out of bounds");
        }

        let range_len = end - start;
        let level = (usize::BITS - 1 - range_len.leading_zeros()) as usize;
        let span = 1usize << level;
        let row = &self.levels[level];
        Ok(row[start].min(row[end - span]))
    }
}

#[cfg(test)]
mod tests {
    use super::SparseTable;

    #[test]
    fn empty_table_rejects_queries() {
        let table = SparseTable::new(&[]);
        assert!(table.query(0, 1).is_err());
        assert!(table.query(0, 0).is_err());
    }

    #[test]
    fn singleton_and_boundary_ranges() {
        let table = SparseTable::new(&[7, -2, 5, 0]);
        assert_eq!(table.query(0, 1), Ok(7));
        assert_eq!(table.query(0, 4), Ok(-2));
        assert_eq!(table.query(3, 4), Ok(0));
        assert_eq!(table.query(1, 2), Ok(-2));
    }

    #[test]
    fn duplicate_minima_are_handled() {
        let table = SparseTable::new(&[4, 1, 1, 6, 1]);
        assert_eq!(table.query(1, 4), Ok(1));
        assert_eq!(table.query(3, 5), Ok(1));
        assert_eq!(table.query(2, 3), Ok(1));
    }

    #[test]
    fn invalid_ranges_are_rejected() {
        let table = SparseTable::new(&[9, 5, 8]);
        assert!(table.query(0, 0).is_err());
        assert!(table.query(2, 1).is_err());
        assert!(table.query(0, 4).is_err());
        assert!(table.query(4, 5).is_err());
    }
}
