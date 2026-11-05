"""Count inversions using merge sort.

An inversion is a pair of indices ``i < j`` for which ``values[i] > values[j]``;
equal values are therefore not inversions.

``count_inversions`` runs in O(n log n) time. It uses O(n) auxiliary storage
for merging, in addition to the returned sorted copy (also O(n)); the input is
not modified.
"""

from typing import Sequence, TypeVar
import unittest

T = TypeVar("T")


def count_inversions(values: Sequence[T]) -> tuple[int, list[T]]:
    """Return the inversion count and a sorted copy of *values*.

    The input sequence is left unchanged. Items must support ordering with
    ``>`` and ``<=``, as ordinary numeric values do.
    """
    sorted_values = list(values)
    auxiliary = [None] * len(sorted_values)

    def sort_and_count(start: int, end: int) -> int:
        if end - start <= 1:
            return 0

        middle = (start + end) // 2
        inversions = sort_and_count(start, middle)
        inversions += sort_and_count(middle, end)

        left, right, output = start, middle, start
        while left < middle and right < end:
            if sorted_values[left] <= sorted_values[right]:
                auxiliary[output] = sorted_values[left]
                left += 1
            else:
                auxiliary[output] = sorted_values[right]
                right += 1
                inversions += middle - left
            output += 1

        while left < middle:
            auxiliary[output] = sorted_values[left]
            left += 1
            output += 1
        while right < end:
            auxiliary[output] = sorted_values[right]
            right += 1
            output += 1

        sorted_values[start:end] = auxiliary[start:end]
        return inversions

    inversion_count = sort_and_count(0, len(sorted_values))
    return inversion_count, sorted_values


class CountInversionsTests(unittest.TestCase):
    def test_empty(self) -> None:
        self.assertEqual(count_inversions([]), (0, []))

    def test_singleton(self) -> None:
        self.assertEqual(count_inversions([7]), (0, [7]))

    def test_already_sorted(self) -> None:
        self.assertEqual(count_inversions([1, 2, 3, 4]), (0, [1, 2, 3, 4]))

    def test_reverse_order(self) -> None:
        self.assertEqual(count_inversions([4, 3, 2, 1]), (6, [1, 2, 3, 4]))

    def test_duplicates_are_not_inversions(self) -> None:
        self.assertEqual(count_inversions([2, 1, 1, 2]), (2, [1, 1, 2, 2]))

    def test_negative_values(self) -> None:
        self.assertEqual(count_inversions([0, -1, -3, 2]), (3, [-3, -1, 0, 2]))

    def test_canonical_example(self) -> None:
        self.assertEqual(count_inversions([2, 4, 1, 3, 5]), (3, [1, 2, 3, 4, 5]))

    def test_input_is_not_modified(self) -> None:
        original = [3, 1, 2]
        count_inversions(original)
        self.assertEqual(original, [3, 1, 2])


if __name__ == "__main__":
    unittest.main()
