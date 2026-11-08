"""Maximum-cardinality selection of pairwise-compatible half-open intervals.

The earliest-finish greedy algorithm takes O(n log n) time for n intervals
and uses O(n) storage for validated intervals and the returned indices.
Intervals are half-open [start, end), so touching intervals are compatible.
Zero-duration intervals are empty and compatible with every interval; all of
these are selected without constraining the greedy choice among nonempty
intervals.
"""
from typing import Iterable, List, Tuple, TypeVar

Endpoint = TypeVar("Endpoint")


def interval_scheduling(
    intervals: Iterable[Iterable[Endpoint]],
) -> List[int]:
    """Return original indices for a maximum compatible interval selection.

    Intervals use half-open [start, end) semantics: two nonempty intervals
    are compatible when one ends no later than the other starts. Inputs are
    consumed once and each item must be an iterable containing exactly two
    endpoints with start <= end. Malformed pairs raise ``ValueError``; a
    non-iterable item raises ``TypeError``.

    Results are ordered deterministically by (end, start, original index).
    For equal finishing times, earlier starts and then lower original indices
    take precedence. Every zero-duration interval is selected because it is
    empty and cannot conflict with another interval.
    """
    indexed: List[Tuple[Endpoint, Endpoint, int]] = []
    for original_index, interval in enumerate(intervals):
        try:
            start, end = interval
        except TypeError as error:
            raise TypeError("each interval must be an iterable pair") from error
        except ValueError as error:
            raise ValueError("each interval must contain exactly two endpoints") from error

        if start > end:
            raise ValueError("interval start must be no greater than its end")
        indexed.append((start, end, original_index))

    indexed.sort(key=lambda item: (item[1], item[0], item[2]))
    selected: List[int] = []
    last_nonempty_finish = None
    has_nonempty_selection = False

    for start, end, original_index in indexed:
        if start == end:
            # Empty half-open intervals are compatible regardless of position.
            selected.append(original_index)
        elif not has_nonempty_selection or start >= last_nonempty_finish:
            selected.append(original_index)
            last_nonempty_finish = end
            has_nonempty_selection = True

    return selected


if __name__ == "__main__":
    import unittest

    class IntervalSchedulingTests(unittest.TestCase):
        def test_standard_optimum(self):
            intervals = [(1, 3), (2, 5), (4, 7), (1, 8), (8, 10)]
            self.assertEqual(interval_scheduling(intervals), [0, 2, 4])

        def test_unsorted_input_returns_original_indices(self):
            intervals = [(5, 7), (1, 3), (3, 5)]
            self.assertEqual(interval_scheduling(intervals), [1, 2, 0])

        def test_empty_input(self):
            self.assertEqual(interval_scheduling([]), [])
            self.assertEqual(interval_scheduling(iter(())), [])

        def test_touching_intervals_are_compatible(self):
            self.assertEqual(
                interval_scheduling([(0, 2), (2, 4), (4, 6)]),
                [0, 1, 2],
            )

        def test_zero_duration_intervals_are_always_selected(self):
            intervals = [(5, 5), (0, 10), (3, 3), (-1, 2)]
            # The nonempty greedy optimum is (-1, 2); both empty intervals
            # remain compatible even though they lie inside other intervals.
            self.assertEqual(interval_scheduling(intervals), [3, 2, 0])

        def test_deterministic_ties(self):
            intervals = [(1, 4), (1, 4), (4, 5), (0, 1), (0, 2)]
            self.assertEqual(interval_scheduling(intervals), [3, 0, 2])

        def test_malformed_pairs_are_rejected(self):
            for intervals in ([(1,)], [(1, 2, 3)]):
                with self.subTest(intervals=intervals):
                    with self.assertRaises(ValueError):
                        interval_scheduling(intervals)

        def test_non_iterable_pair_is_rejected(self):
            with self.assertRaises(TypeError):
                interval_scheduling([4])

        def test_reversed_interval_is_rejected(self):
            with self.assertRaises(ValueError):
                interval_scheduling([(3, 2)])

    unittest.main()
