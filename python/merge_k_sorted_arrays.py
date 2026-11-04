"""Merge sorted integer sequences with a min-heap.

The merge takes O(N log k) time and O(k) auxiliary heap storage, excluding
O(N) space for the returned output, where N is the total number of integers
and k is the number of input sequences.
"""

from heapq import heappop, heappush
from typing import Iterable, List, Sequence


def merge_k_sorted_arrays(arrays: Iterable[Sequence[int]]) -> List[int]:
    """Return all values from sorted input sequences in sorted order.

    Duplicates are preserved. Equal values are resolved deterministically by
    input sequence order, then by position within that sequence.
    """
    sequences = list(arrays)
    heap = []
    for sequence_index, sequence in enumerate(sequences):
        if sequence:
            heappush(heap, (sequence[0], sequence_index, 0))

    merged = []
    while heap:
        value, sequence_index, element_index = heappop(heap)
        merged.append(value)

        next_index = element_index + 1
        sequence = sequences[sequence_index]
        if next_index < len(sequence):
            heappush(
                heap,
                (sequence[next_index], sequence_index, next_index),
            )

    return merged


if __name__ == "__main__":
    import unittest

    class MergeKSortedArraysTests(unittest.TestCase):
        def test_empty_outer_input(self):
            self.assertEqual(merge_k_sorted_arrays([]), [])

        def test_empty_sublists(self):
            self.assertEqual(merge_k_sorted_arrays([[], [], []]), [])
            self.assertEqual(merge_k_sorted_arrays([[], [1, 3], []]), [1, 3])

        def test_one_list(self):
            self.assertEqual(merge_k_sorted_arrays([[1, 2, 5]]), [1, 2, 5])

        def test_duplicates_are_preserved_and_stable(self):
            self.assertEqual(
                merge_k_sorted_arrays([[1, 2, 2], [1, 2], [2]]),
                [1, 1, 2, 2, 2, 2],
            )

        def test_negative_values(self):
            self.assertEqual(
                merge_k_sorted_arrays([[-8, -3, 0], [-5, -3, 4]]),
                [-8, -5, -3, -3, 0, 4],
            )

        def test_many_short_sorted_inputs(self):
            arrays = [[value, value + 100] for value in range(50)]
            expected = list(range(50)) + list(range(100, 150))
            self.assertEqual(merge_k_sorted_arrays(arrays), expected)

    unittest.main()
