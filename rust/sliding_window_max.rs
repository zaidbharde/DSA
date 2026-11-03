use std::collections::VecDeque;
use std::error::Error;
use std::fmt;

/// Errors returned when a sliding-window size is invalid.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum SlidingWindowError {
    /// The window size must be positive.
    ZeroWindowSize,
    /// A non-empty input cannot have a window larger than its length.
    WindowLargerThanInput {
        window_size: usize,
        input_len: usize,
    },
}

impl fmt::Display for SlidingWindowError {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::ZeroWindowSize => write!(formatter, "window size must be positive"),
            Self::WindowLargerThanInput {
                window_size,
                input_len,
            } => write!(
                formatter,
                "window size {window_size} exceeds input length {input_len}"
            ),
        }
    }
}

impl Error for SlidingWindowError {}

/// Returns the maximum value in each contiguous window of length `k`.
///
/// A zero window size returns [`SlidingWindowError::ZeroWindowSize`]. For an
/// empty input, any positive `k` returns an empty vector. For non-empty input,
/// `k` must not exceed the input length, or
/// [`SlidingWindowError::WindowLargerThanInput`] is returned.
///
/// Uses a monotonic deque of indices, running in O(n) time with O(k)
/// auxiliary storage, excluding the returned vector.
pub fn max_sliding_window(values: &[i32], k: usize) -> Result<Vec<i32>, SlidingWindowError> {
    if k == 0 {
        return Err(SlidingWindowError::ZeroWindowSize);
    }
    if values.is_empty() {
        return Ok(Vec::new());
    }
    if k > values.len() {
        return Err(SlidingWindowError::WindowLargerThanInput {
            window_size: k,
            input_len: values.len(),
        });
    }

    let mut indices = VecDeque::with_capacity(k);
    let mut maxima = Vec::with_capacity(values.len() - k + 1);

    for (index, &value) in values.iter().enumerate() {
        // Remove indices that have fallen outside the current window.
        if index >= k {
            let first_valid_index = index - k + 1;
            while indices
                .front()
                .is_some_and(|&front_index| front_index < first_valid_index)
            {
                indices.pop_front();
            }
        }

        // Newer values at least as large make older candidates obsolete.
        while indices
            .back()
            .is_some_and(|&back_index| values[back_index] <= value)
        {
            indices.pop_back();
        }
        indices.push_back(index);

        if index + 1 >= k {
            maxima.push(values[indices[0]]);
        }
    }

    Ok(maxima)
}

#[cfg(test)]
mod tests {
    use super::{max_sliding_window, SlidingWindowError};

    #[test]
    fn window_size_one_returns_each_value() {
        assert_eq!(
            max_sliding_window(&[4, -2, 7, 1], 1),
            Ok(vec![4, -2, 7, 1])
        );
    }

    #[test]
    fn window_size_equal_to_input_length_returns_one_maximum() {
        assert_eq!(max_sliding_window(&[1, 3, -1, 5], 4), Ok(vec![5]));
    }

    #[test]
    fn duplicate_maxima_are_preserved_across_windows() {
        assert_eq!(max_sliding_window(&[2, 2, 2, 2], 2), Ok(vec![2, 2, 2]));
    }

    #[test]
    fn increasing_and_decreasing_inputs() {
        assert_eq!(
            max_sliding_window(&[1, 2, 3, 4, 5], 3),
            Ok(vec![3, 4, 5])
        );
        assert_eq!(
            max_sliding_window(&[5, 4, 3, 2, 1], 3),
            Ok(vec![5, 4, 3])
        );
    }

    #[test]
    fn invalid_window_sizes_return_typed_errors() {
        assert_eq!(
            max_sliding_window(&[1, 2], 0),
            Err(SlidingWindowError::ZeroWindowSize)
        );
        assert_eq!(
            max_sliding_window(&[1, 2], 3),
            Err(SlidingWindowError::WindowLargerThanInput {
                window_size: 3,
                input_len: 2,
            })
        );
    }

    #[test]
    fn empty_input_returns_empty_for_positive_window_size() {
        assert_eq!(max_sliding_window(&[], 1), Ok(vec![]));
        assert_eq!(max_sliding_window(&[], 7), Ok(vec![]));
    }
}
