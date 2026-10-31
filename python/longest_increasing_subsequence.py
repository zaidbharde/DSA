"""Length of a longest strictly increasing subsequence.

The patience-sorting tails method runs in O(n log n) time and uses O(n)
auxiliary space. The input sequence is never modified.
"""

from bisect import bisect_left
from collections.abc import Sequence
import unittest


def length_of_lis(values: Sequence[int]) -> int:
    """Return the length of the longest strictly increasing subsequence.

    ``tails[i]`` stores the smallest possible ending value of an increasing
    subsequence of length ``i + 1`` among the values seen so far. ``bisect_left``
    replaces the first tail greater than or equal to the current value, which
    ensures that equal values do not extend a strictly increasing subsequence.

    Time complexity: O(n log n).
    Space complexity: O(n) auxiliary space in the worst case.
    """
    tails: list[int] = []
    for value in values:
        position = bisect_left(tails, value)
        if position == len(tails):
            tails.append(value)
        else:
            tails[position] = value
    return len(tails)


class LengthOfLisTests(unittest.TestCase):
    def test_empty(self) -> None:
        self.assertEqual(length_of_lis([]), 0)

    def test_singleton(self) -> None:
        self.assertEqual(length_of_lis([7]), 1)

    def test_duplicates_are_not_strictly_increasing(self) -> None:
        self.assertEqual(length_of_lis([2, 2, 2, 2]), 1)
        self.assertEqual(length_of_lis([1, 3, 3, 4]), 3)

    def test_decreasing(self) -> None:
        self.assertEqual(length_of_lis([9, 7, 5, 3, 1]), 1)

    def test_classic_mixed_input(self) -> None:
        values = [10, 9, 2, 5, 3, 7, 101, 18]
        original = values.copy()
        self.assertEqual(length_of_lis(values), 4)
        self.assertEqual(values, original)


if __name__ == "__main__":
    unittest.main()
