"""Minimum-coin change for unbounded positive denominations.

The dynamic program takes O(amount * number_of_coins) time and uses
O(amount) auxiliary space. :func:`minimum_coin_composition` also reconstructs
an optimal composition with a deterministic tie-break.
"""

import unittest


def _minimum_coin_table(amount, denominations):
    """Return a table whose entries are minimum coin counts or infinity sentinel."""
    unreachable = amount + 1
    dp = [unreachable] * (amount + 1)
    dp[0] = 0

    for value in range(1, amount + 1):
        for coin in denominations:
            if coin <= value:
                dp[value] = min(dp[value], dp[value - coin] + 1)

    return dp, unreachable


def minimum_coins(amount, coins):
    """Return the fewest coins needed to make ``amount``, or ``None``.

    Each denomination may be used an unlimited number of times. A negative
    amount or any nonpositive denomination is rejected with ``ValueError``.
    Duplicate denominations are allowed and do not affect correctness.

    Time complexity is O(amount * number_of_coins); auxiliary space is
    O(amount).
    """
    if amount < 0:
        raise ValueError("amount must be nonnegative")

    denominations = list(coins)
    if any(coin <= 0 for coin in denominations):
        raise ValueError("coin denominations must be positive")

    dp, unreachable = _minimum_coin_table(amount, denominations)
    return None if dp[amount] == unreachable else dp[amount]


def minimum_coin_composition(amount, coins):
    """Return a deterministic minimum-coin composition, or ``None``.

    Each denomination may be used an unlimited number of times. A negative
    amount or any nonpositive denomination raises ``ValueError``, matching
    :func:`minimum_coins`. The zero amount returns an empty list.

    Among optimal compositions, the returned coins are in nondecreasing order
    and lexicographically smallest: at each step, choose the smallest
    denomination that still permits the remaining optimal coin count. Duplicate
    denominations are ignored for reconstruction.

    Time complexity is O(amount * number_of_coins) for the dynamic program,
    plus O(number_of_coins log number_of_coins) to sort unique denominations.
    Auxiliary space is O(amount), excluding the returned composition.
    """
    if amount < 0:
        raise ValueError("amount must be nonnegative")

    denominations = list(coins)
    if any(coin <= 0 for coin in denominations):
        raise ValueError("coin denominations must be positive")

    ordered_denominations = sorted(set(denominations))
    dp, unreachable = _minimum_coin_table(amount, ordered_denominations)
    if dp[amount] == unreachable:
        return None

    composition = []
    remaining = amount
    while remaining:
        needed = dp[remaining]
        for coin in ordered_denominations:
            if coin <= remaining and dp[remaining - coin] == needed - 1:
                composition.append(coin)
                remaining -= coin
                break
        else:  # Defensive guard: a reachable table should always reconstruct.
            raise RuntimeError("could not reconstruct a reachable amount")

    return composition


class MinimumCoinsTests(unittest.TestCase):
    def test_common_amount(self):
        self.assertEqual(minimum_coins(11, [1, 2, 5]), 3)

    def test_impossible_amount(self):
        self.assertIsNone(minimum_coins(3, [2]))

    def test_zero_amount(self):
        self.assertEqual(minimum_coins(0, [1, 2, 5]), 0)
        self.assertEqual(minimum_coins(0, []), 0)

    def test_duplicate_coins(self):
        self.assertEqual(minimum_coins(6, [1, 3, 3, 4]), 2)

    def test_negative_amount_is_rejected(self):
        with self.assertRaises(ValueError):
            minimum_coins(-1, [1, 2])

    def test_nonpositive_coin_is_rejected(self):
        for coins in ([0, 1], [-2, 1]):
            with self.subTest(coins=coins):
                with self.assertRaises(ValueError):
                    minimum_coins(5, coins)


class MinimumCoinCompositionTests(unittest.TestCase):
    def test_composition_sums_uses_allowed_coins_and_is_optimal(self):
        amount = 11
        coins = [1, 2, 5]
        composition = minimum_coin_composition(amount, coins)

        self.assertEqual(sum(composition), amount)
        self.assertTrue(all(coin in coins for coin in composition))
        self.assertEqual(len(composition), minimum_coins(amount, coins))

    def test_tie_break_is_deterministic_and_lexicographically_smallest(self):
        expected = [3, 5]
        self.assertEqual(minimum_coin_composition(8, [3, 4, 5]), expected)
        self.assertEqual(minimum_coin_composition(8, [5, 4, 3]), expected)
        self.assertEqual(minimum_coin_composition(8, [3, 4, 5, 3]), expected)

    def test_impossible_amount_returns_none(self):
        self.assertIsNone(minimum_coin_composition(3, [2]))

    def test_zero_amount_returns_empty_composition(self):
        self.assertEqual(minimum_coin_composition(0, [1, 2, 5]), [])
        self.assertEqual(minimum_coin_composition(0, []), [])

    def test_invalid_amount_and_coin_inputs_are_rejected(self):
        with self.assertRaises(ValueError):
            minimum_coin_composition(-1, [1, 2])
        for coins in ([0, 1], [-2, 1]):
            with self.subTest(coins=coins):
                with self.assertRaises(ValueError):
                    minimum_coin_composition(5, coins)


if __name__ == "__main__":
    unittest.main()
