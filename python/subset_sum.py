"""Subset-sum decision using a one-dimensional, 0/1 dynamic program.

The algorithm runs in O(n * T) time and uses O(T) auxiliary space, where
``n`` is the number of values and ``T`` is the target. Targets are updated in
descending order so that each input value can contribute at most once.
"""

from collections.abc import Iterable
import unittest


def subset_sum(values: Iterable[int], target: int) -> bool:
    """Return whether nonnegative integer ``values`` can sum exactly to target.

    Args:
        values: An iterable of nonnegative integers. Each item may be used at
            most once.
        target: The nonnegative integer sum to test.

    Raises:
        TypeError: If the target or any value is not an integer, or if
            ``values`` is not iterable. Booleans are not accepted as integers.
        ValueError: If the target or any value is negative.

    The dynamic-programming table is represented by a boolean list where
    ``reachable[s]`` means that a subset of the values processed so far sums
    to ``s``. Time complexity is O(n * T); auxiliary space complexity is
    O(T), with T equal to the target.
    """
    if isinstance(target, bool) or not isinstance(target, int):
        raise TypeError("target must be a nonnegative integer")
    if target < 0:
        raise ValueError("target must be nonnegative")

    if isinstance(values, (str, bytes)):
        raise TypeError("values must be an iterable of nonnegative integers")
    try:
        items = list(values)
    except TypeError as exc:
        raise TypeError("values must be an iterable of nonnegative integers") from exc

    for index, value in enumerate(items):
        if isinstance(value, bool) or not isinstance(value, int):
            raise TypeError(f"values[{index}] must be a nonnegative integer")
        if value < 0:
            raise ValueError(f"values[{index}] must be nonnegative")

    reachable = [False] * (target + 1)
    reachable[0] = True

    for value in items:
        # Values larger than target cannot contribute to any relevant sum.
        for current_sum in range(target, value - 1, -1):
            if reachable[current_sum - value]:
                reachable[current_sum] = True

    return reachable[target]


class SubsetSumTests(unittest.TestCase):
    def test_empty_values(self):
        self.assertTrue(subset_sum([], 0))
        self.assertFalse(subset_sum([], 1))

    def test_zero_target(self):
        self.assertTrue(subset_sum([2, 5, 9], 0))

    def test_reachable_and_unreachable_targets(self):
        self.assertTrue(subset_sum([3, 7, 10, 12], 22))
        self.assertFalse(subset_sum([2, 4, 6], 5))

    def test_duplicates_are_distinct_items(self):
        self.assertTrue(subset_sum([4, 4], 8))
        self.assertFalse(subset_sum([4], 8))

    def test_zero_values(self):
        self.assertTrue(subset_sum([0, 0, 3], 3))
        self.assertFalse(subset_sum([0, 0], 1))

    def test_invalid_target(self):
        with self.assertRaises(TypeError):
            subset_sum([1, 2], 2.0)
        with self.assertRaises(TypeError):
            subset_sum([1], True)
        with self.assertRaises(ValueError):
            subset_sum([1], -1)

    def test_invalid_values(self):
        with self.assertRaises(TypeError):
            subset_sum([1, "2"], 3)
        with self.assertRaises(TypeError):
            subset_sum([True], 1)
        with self.assertRaises(ValueError):
            subset_sum([1, -2], 1)
        with self.assertRaises(TypeError):
            subset_sum(None, 0)


if __name__ == "__main__":
    unittest.main()
