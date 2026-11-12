"""Return the k most frequent integers with deterministic tie-breaking.

A frequency counter is built in O(n) time, then its u distinct values are
heapified in O(u) time. Popping k entries takes O(k log u) time, for total
O(n + u + k log u) time. The counter and heap require O(u) auxiliary space;
the returned list adds O(k) output space. Here n is the input length and u is
the number of distinct values. Equal-frequency values are ordered smaller
integer first.
"""

from collections import Counter
from collections.abc import Iterable
import heapq
import unittest


def top_k_frequent(values: Iterable[int], k: int) -> list[int]:
    """Return the k most frequent values, breaking frequency ties by value.

    ``k`` must be an integer (not a boolean) between zero and the number of
    distinct input values, inclusive. A negative k or a k larger than the
    number of distinct values raises ``ValueError``; a non-integer k raises
    ``TypeError``.

    The heap stores ``(-frequency, value)`` pairs. Consequently, the largest
    frequency is popped first and, among ties, the smallest value is popped
    first. Counting and heap construction take O(n + u) time; popping k values
    costs O(k log u), giving O(n + u + k log u) total time. The counter and
    heap use O(u) auxiliary space, excluding the O(k) returned list.
    """
    if not isinstance(k, int) or isinstance(k, bool):
        raise TypeError("k must be an integer")

    frequencies = Counter(values)
    distinct_count = len(frequencies)
    if k < 0 or k > distinct_count:
        raise ValueError("k must be between 0 and the number of distinct values")

    heap = [(-frequency, value) for value, frequency in frequencies.items()]
    heapq.heapify(heap)
    return [heapq.heappop(heap)[1] for _ in range(k)]


class TopKFrequentTests(unittest.TestCase):
    def test_empty_input(self) -> None:
        self.assertEqual(top_k_frequent([], 0), [])

    def test_duplicates_ordered_by_frequency(self) -> None:
        self.assertEqual(top_k_frequent([1, 1, 1, 2, 2, 3], 2), [1, 2])

    def test_ties_use_smaller_integer_first(self) -> None:
        self.assertEqual(top_k_frequent([5, 4, 3, 2, 1], 3), [1, 2, 3])
        self.assertEqual(top_k_frequent([4, 1, 4, 1, 3, 3], 3), [1, 3, 4])

    def test_zero_k(self) -> None:
        self.assertEqual(top_k_frequent([2, 2, 1], 0), [])

    def test_invalid_k_bounds(self) -> None:
        with self.assertRaises(ValueError):
            top_k_frequent([1, 1, 2], -1)
        with self.assertRaises(ValueError):
            top_k_frequent([1, 1, 2], 3)
        with self.assertRaises(ValueError):
            top_k_frequent([], 1)

    def test_non_integer_k(self) -> None:
        with self.assertRaises(TypeError):
            top_k_frequent([1, 2], 1.5)  # type: ignore[arg-type]
        with self.assertRaises(TypeError):
            top_k_frequent([1, 2], True)  # type: ignore[arg-type]


if __name__ == "__main__":
    unittest.main()
