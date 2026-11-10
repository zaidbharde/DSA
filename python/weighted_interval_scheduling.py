"""Maximum-value selection of pairwise-compatible weighted intervals.

Jobs are half-open intervals ``(start, end, weight)``. Touching jobs are
compatible when one ends exactly as another begins. The dynamic program and
predecessor search take O(n log n) time for n jobs and use O(n) auxiliary
space, including the sorted jobs and reconstruction tables.
"""

from bisect import bisect_right
import unittest
from typing import Any, Iterable, List, Tuple


Job = Tuple[Any, Any, Any]


def weighted_interval_scheduling(jobs: Iterable[Iterable[Any]]) -> Tuple[Any, List[Job]]:
    """Return ``(maximum_total_weight, chosen_jobs)`` for the supplied jobs.

    Each job must be an iterable of exactly three values: ``start``, ``end``,
    and ``weight``. Its range must satisfy ``start <= end``. A malformed job
    raises ``ValueError``; a job that is not iterable raises ``TypeError``.
    Endpoints must be mutually orderable, and weights must support addition and
    comparison. Jobs are interpreted as half-open intervals, so ``end == start``
    is compatible. Negative-weight jobs may be omitted; the empty schedule has
    value zero.

    Ties are deterministic: jobs are considered by ``(end, start, input
    position)``, and an equal-value include candidate is skipped in favor of
    the already-computed schedule. Returned jobs are normalized to tuples.

    Time complexity is O(n log n); auxiliary space is O(n).
    """
    indexed_jobs = []
    for input_index, raw_job in enumerate(jobs):
        try:
            job = tuple(raw_job)
        except TypeError as error:
            raise TypeError("each job must be an iterable triple") from error
        if len(job) != 3:
            raise ValueError("each job must contain exactly start, end, and weight")

        start, end, weight = job
        if start > end:
            raise ValueError("job start must be no greater than its end")
        indexed_jobs.append((start, end, weight, input_index, job))

    indexed_jobs.sort(key=lambda item: (item[1], item[0], item[3]))
    ends = [job[1] for job in indexed_jobs]
    count = len(indexed_jobs)

    # predecessor_count[i] is the number of earlier jobs ending no later than
    # job i starts; that count is also the corresponding DP prefix length.
    predecessor_count = [
        bisect_right(ends, indexed_jobs[i][0], 0, i) for i in range(count)
    ]

    best_value = [0] * (count + 1)
    take_job = [False] * count
    for i, (_, _, weight, _, _) in enumerate(indexed_jobs):
        include_value = weight + best_value[predecessor_count[i]]
        exclude_value = best_value[i]
        if include_value > exclude_value:
            best_value[i + 1] = include_value
            take_job[i] = True
        else:
            best_value[i + 1] = exclude_value

    chosen = []
    i = count - 1
    while i >= 0:
        if take_job[i] and best_value[i + 1] > best_value[i]:
            chosen.append(indexed_jobs[i][4])
            i = predecessor_count[i] - 1
        else:
            i -= 1

    chosen.reverse()
    return best_value[count], chosen


class WeightedIntervalSchedulingTests(unittest.TestCase):
    def test_classic_example(self):
        jobs = [
            (1, 3, 5),
            (2, 5, 6),
            (4, 6, 5),
            (6, 7, 4),
            (5, 8, 11),
            (7, 9, 2),
        ]
        self.assertEqual(
            weighted_interval_scheduling(jobs),
            (17, [(2, 5, 6), (5, 8, 11)]),
        )

    def test_empty_input(self):
        self.assertEqual(weighted_interval_scheduling([]), (0, []))
        self.assertEqual(weighted_interval_scheduling(iter(())), (0, []))

    def test_negative_weights_are_omitted(self):
        self.assertEqual(
            weighted_interval_scheduling([(0, 2, -3), (2, 4, -1)]),
            (0, []),
        )

    def test_equal_value_tie_prefers_earlier_sorted_job(self):
        jobs = [(0, 2, 5), (0, 2, 5)]
        self.assertEqual(weighted_interval_scheduling(jobs), (5, [jobs[0]]))

    def test_touching_boundaries_are_compatible(self):
        jobs = [(0, 2, 3), (2, 4, 4), (4, 6, 5)]
        self.assertEqual(weighted_interval_scheduling(jobs), (12, jobs))

    def test_invalid_job_shapes_are_rejected(self):
        for jobs in ([(1, 2)], [(1, 2, 3, 4)]):
            with self.subTest(jobs=jobs):
                with self.assertRaises(ValueError):
                    weighted_interval_scheduling(jobs)

    def test_non_iterable_job_is_rejected(self):
        with self.assertRaises(TypeError):
            weighted_interval_scheduling([7])

    def test_reversed_ranges_are_rejected(self):
        with self.assertRaises(ValueError):
            weighted_interval_scheduling([(3, 2, 10)])

    def test_reconstruction_is_compatible_and_matches_value(self):
        jobs = [(4, 7, 8), (1, 3, 5), (3, 5, 4), (2, 6, 12), (7, 9, 3)]
        value, chosen = weighted_interval_scheduling(jobs)

        self.assertEqual(value, sum(job[2] for job in chosen))
        self.assertTrue(all(job in jobs for job in chosen))
        ordered = sorted(chosen, key=lambda job: (job[0], job[1]))
        self.assertTrue(
            all(earlier[1] <= later[0] for earlier, later in zip(ordered, ordered[1:]))
        )


if __name__ == "__main__":
    unittest.main()
