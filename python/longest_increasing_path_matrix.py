"""Longest strictly increasing path in a 4-neighbor matrix.

Reverse topological dynamic programming visits each matrix cell and each
neighbor a constant number of times, taking O(R * C) time and O(R * C) space.

The input must be a sequence of row sequences with a rectangular shape. Entries
must be finite ``numbers.Real`` values; booleans, complex numbers, other types,
and non-finite values are rejected. Empty matrices and rectangular matrices
with zero columns have path length zero.
"""

from collections import deque
from collections.abc import Sequence
import math
from numbers import Real
import sys
import unittest


def longest_increasing_path(matrix: Sequence[Sequence[Real]]) -> int:
    """Return the length of the longest strictly increasing 4-neighbor path.

    A path may move up, down, left, or right, and each next value must be
    strictly greater than the previous one. The matrix is not modified.

    Raises:
        TypeError: If the matrix is not a sequence of row sequences, or any
            entry is not a real number or is a boolean.
        ValueError: If rows have different lengths or an entry is non-finite.

    Time complexity: O(R * C), since each cell and its four neighbors are
    processed a constant number of times.
    Space complexity: O(R * C) for the dynamic-programming table, remaining
    outdegrees, and processing queue.
    """
    if not isinstance(matrix, Sequence):
        raise TypeError("matrix must be a sequence of row sequences")
    if not matrix:
        return 0

    if any(not isinstance(row, Sequence) for row in matrix):
        raise TypeError("each matrix row must be a sequence")

    column_count = len(matrix[0])
    if any(len(row) != column_count for row in matrix):
        raise ValueError("matrix rows must all have the same length")

    for row in matrix:
        for value in row:
            if isinstance(value, bool) or not isinstance(value, Real):
                raise TypeError("matrix entries must be real numbers, not booleans")
            # Comparisons avoid converting very large finite integers to float.
            if value != value or value == math.inf or value == -math.inf:
                raise ValueError("matrix entries must be finite")

    row_count = len(matrix)
    if column_count == 0:
        return 0

    directions = ((1, 0), (-1, 0), (0, 1), (0, -1))
    # Each cell's outdegree is the number of strictly larger neighbors. Begin
    # with local maxima; processing them backwards propagates path lengths to
    # smaller neighbors once all of their larger neighbors are available.
    remaining_larger = [[0] * column_count for _ in range(row_count)]
    path_lengths = [[1] * column_count for _ in range(row_count)]
    ready = deque()
    for row in range(row_count):
        for column in range(column_count):
            current = matrix[row][column]
            for row_delta, column_delta in directions:
                next_row = row + row_delta
                next_column = column + column_delta
                if (
                    0 <= next_row < row_count
                    and 0 <= next_column < column_count
                    and matrix[next_row][next_column] > current
                ):
                    remaining_larger[row][column] += 1
            if remaining_larger[row][column] == 0:
                ready.append((row, column))

    longest = 1
    while ready:
        row, column = ready.popleft()
        current_length = path_lengths[row][column]
        longest = max(longest, current_length)
        current = matrix[row][column]
        for row_delta, column_delta in directions:
            next_row = row + row_delta
            next_column = column + column_delta
            if (
                0 <= next_row < row_count
                and 0 <= next_column < column_count
                and matrix[next_row][next_column] < current
            ):
                path_lengths[next_row][next_column] = max(
                    path_lengths[next_row][next_column], current_length + 1
                )
                remaining_larger[next_row][next_column] -= 1
                if remaining_larger[next_row][next_column] == 0:
                    ready.append((next_row, next_column))

    return longest


class LongestIncreasingPathTests(unittest.TestCase):
    def test_classic_example(self) -> None:
        self.assertEqual(
            longest_increasing_path(
                [[9, 9, 4], [6, 6, 8], [2, 1, 1]]
            ),
            4,
        )

    def test_increasing_and_decreasing_rows(self) -> None:
        self.assertEqual(longest_increasing_path([[1, 2, 3, 4]]), 4)
        self.assertEqual(longest_increasing_path([[4, 3, 2, 1]]), 4)

    def test_long_monotone_path_exceeds_recursion_limit(self) -> None:
        path_length = sys.getrecursionlimit() + 100
        matrix = [[value] for value in range(path_length)]
        self.assertEqual(longest_increasing_path(matrix), path_length)

    def test_duplicates_do_not_extend_path(self) -> None:
        self.assertEqual(longest_increasing_path([[5, 5], [5, 5]]), 1)

    def test_empty_and_zero_width_matrices(self) -> None:
        self.assertEqual(longest_increasing_path([]), 0)
        self.assertEqual(longest_increasing_path([[], []]), 0)

    def test_one_cell(self) -> None:
        self.assertEqual(longest_increasing_path([[7]]), 1)

    def test_jagged_matrix_is_rejected(self) -> None:
        with self.assertRaises(ValueError):
            longest_increasing_path([[1, 2], [3]])

    def test_non_numeric_and_boolean_entries_are_rejected(self) -> None:
        for invalid in ("1", 1 + 2j, True):
            with self.subTest(invalid=invalid):
                with self.assertRaises(TypeError):
                    longest_increasing_path([[invalid]])

    def test_non_finite_entries_are_rejected(self) -> None:
        for invalid in (math.nan, math.inf, -math.inf):
            with self.subTest(invalid=invalid):
                with self.assertRaises(ValueError):
                    longest_increasing_path([[invalid]])

    def test_rows_must_be_sequences(self) -> None:
        with self.assertRaises(TypeError):
            longest_increasing_path([[1], None])  # type: ignore[list-item]


if __name__ == "__main__":
    unittest.main()
