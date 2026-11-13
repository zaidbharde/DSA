"""Uniformly sample one item from a single-pass iterable.

For an input of n items, reservoir sampling takes O(n) time and O(1)
auxiliary space, excluding the returned item. Each item has an equal 1/n
probability of being selected. The iterable is consumed once, and empty input
raises ValueError.
"""

from collections.abc import Iterable
from random import Random
from typing import Protocol, TypeVar
import unittest


T = TypeVar("T")


class RandomSource(Protocol):
    """Minimal interface required of an injectable random-number source."""

    def randrange(self, stop: int, /) -> int:
        """Return an integer in the half-open interval [0, stop)."""
        ...


def reservoir_sample(items: Iterable[T], rng: RandomSource | None = None) -> T:
    """Return one uniformly selected item without storing the full iterable.

    ``rng`` may be any source compatible with ``random.Random.randrange``;
    when omitted, a fresh ``random.Random`` instance is used. Values are
    considered as individual positions, so repeated values can be sampled
    independently.

    Raises:
        ValueError: If ``items`` is empty.

    Time complexity is O(n), where n is the number of yielded items.
    Auxiliary space is O(1), excluding the sampled item and the RNG itself.
    """
    source = Random() if rng is None else rng
    iterator = iter(items)
    try:
        selected = next(iterator)
    except StopIteration as exc:
        raise ValueError("cannot sample from an empty iterable") from exc

    for count, item in enumerate(iterator, start=2):
        if source.randrange(count) == 0:
            selected = item
    return selected


class ReservoirSampleTests(unittest.TestCase):
    def test_consumes_single_pass_iterable_once(self):
        class SinglePass:
            def __init__(self):
                self.iterations = 0
                self.yielded = 0

            def __iter__(self):
                self.iterations += 1
                if self.iterations > 1:
                    raise AssertionError("iterable was traversed more than once")
                for value in range(5):
                    self.yielded += 1
                    yield value

        items = SinglePass()
        self.assertIn(reservoir_sample(items, Random(7)), range(5))
        self.assertEqual(items.iterations, 1)
        self.assertEqual(items.yielded, 5)

    def test_empty_iterable_raises_value_error(self):
        with self.assertRaises(ValueError):
            reservoir_sample(iter(()))

    def test_singleton_is_returned_without_random_draw(self):
        class NoDraws:
            def randrange(self, stop: int, /) -> int:
                raise AssertionError("singleton sampling must not draw")

        self.assertEqual(reservoir_sample(iter(("only",)), NoDraws()), "only")

    def test_controlled_replacement_decisions(self):
        class ControlledRandom:
            def __init__(self, decisions):
                self.decisions = iter(decisions)
                self.bounds = []

            def randrange(self, stop: int, /) -> int:
                self.bounds.append(stop)
                decision = next(self.decisions)
                if not 0 <= decision < stop:
                    raise AssertionError("controlled choice is outside randrange bounds")
                return decision

        rng = ControlledRandom([1, 0, 2])
        self.assertEqual(reservoir_sample([10, 20, 30, 40], rng), 30)
        self.assertEqual(rng.bounds, [2, 3, 4])
        with self.assertRaises(StopIteration):
            next(rng.decisions)

    def test_same_seed_reproduces_sample(self):
        values = list(range(100))
        self.assertEqual(
            reservoir_sample(values, Random(12345)),
            reservoir_sample(values, Random(12345)),
        )


if __name__ == "__main__":
    unittest.main()
