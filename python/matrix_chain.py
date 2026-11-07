"""Minimum scalar multiplication cost for a matrix chain.

Uses bottom-up interval dynamic programming with deterministic optimal
parenthesization reconstruction. The algorithm takes O(n^3) time and O(n^2)
auxiliary space for n matrices.
"""
from collections.abc import Iterable
import unittest


def matrix_chain_order(dimensions: Iterable[int]) -> tuple[int, str]:
    """Return ``(minimum_cost, parenthesization)`` for a matrix chain.

    ``dimensions`` is the sequence ``[p0, p1, ..., pn]`` describing matrices
    A1 through An, where Ai has shape ``(p[i-1], p[i])``. Every dimension
    must be a positive integer (booleans are not accepted). At least two
    dimensions, corresponding to at least one matrix, are required.

    Raises:
        TypeError: If ``dimensions`` is not an iterable of integers.
        ValueError: If fewer than one matrix is described or a dimension is
            not positive.

    Ties are resolved by choosing the smallest split index at each interval.
    The algorithm takes O(n^3) time and O(n^2) auxiliary space for n matrices.
    """
    if isinstance(dimensions, (str, bytes)) or not isinstance(dimensions, Iterable):
        raise TypeError("dimensions must be an iterable of positive integers")
    dims = list(dimensions)
    if len(dims) < 2:
        raise ValueError("dimensions must describe at least one matrix")
    for index, dimension in enumerate(dims):
        if isinstance(dimension, bool) or not isinstance(dimension, int):
            raise TypeError(f"dimensions[{index}] must be a positive integer")
        if dimension <= 0:
            raise ValueError(f"dimensions[{index}] must be positive")

    matrix_count = len(dims) - 1
    costs = [[0] * matrix_count for _ in range(matrix_count)]
    splits = [[None] * matrix_count for _ in range(matrix_count)]

    # Increasing interval lengths ensure all smaller subproblems are solved.
    for chain_length in range(2, matrix_count + 1):
        for start in range(matrix_count - chain_length + 1):
            end = start + chain_length - 1
            best_cost = None
            best_split = None
            for split in range(start, end):
                candidate = (
                    costs[start][split]
                    + costs[split + 1][end]
                    + dims[start] * dims[split + 1] * dims[end + 1]
                )
                # Strict comparison preserves the first (smallest) split on ties.
                if best_cost is None or candidate < best_cost:
                    best_cost = candidate
                    best_split = split
            costs[start][end] = best_cost
            splits[start][end] = best_split

    def build_parenthesization(start: int, end: int) -> str:
        if start == end:
            return f"A{start + 1}"
        split = splits[start][end]
        return (
            f"({build_parenthesization(start, split)} x "
            f"{build_parenthesization(split + 1, end)})"
        )

    return costs[0][matrix_count - 1], build_parenthesization(0, matrix_count - 1)


class MatrixChainOrderTests(unittest.TestCase):
    def test_classic_example(self):
        self.assertEqual(
            matrix_chain_order([30, 35, 15, 5, 10, 20, 25]),
            (15125, "((A1 x (A2 x A3)) x ((A4 x A5) x A6))"),
        )

    def test_single_matrix(self):
        self.assertEqual(matrix_chain_order([7, 11]), (0, "A1"))

    def test_tied_costs_are_deterministic(self):
        expected = (16, "(A1 x (A2 x A3))")
        self.assertEqual(matrix_chain_order([2, 2, 2, 2]), expected)
        self.assertEqual(matrix_chain_order([2, 2, 2, 2]), expected)

    def test_fewer_than_one_matrix(self):
        for dimensions in ([], [5]):
            with self.subTest(dimensions=dimensions):
                with self.assertRaises(ValueError):
                    matrix_chain_order(dimensions)

    def test_invalid_dimensions(self):
        for dimensions in ([2, 0], [2, -3], [1, 2.5], [True, 2]):
            with self.subTest(dimensions=dimensions):
                expected_error = TypeError if any(
                    isinstance(value, bool) or not isinstance(value, int)
                    for value in dimensions
                ) else ValueError
                with self.assertRaises(expected_error):
                    matrix_chain_order(dimensions)
        with self.assertRaises(TypeError):
            matrix_chain_order(None)
        with self.assertRaises(TypeError):
            matrix_chain_order("23")


if __name__ == "__main__":
    unittest.main()
