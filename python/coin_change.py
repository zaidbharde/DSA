"""Minimum-coin change for unbounded positive denominations.

The dynamic program takes O(amount * number_of_coins) time and uses
O(amount) auxiliary space.
"""

import unittest


def minimum_coins(amount, coins):
    """Return the fewest coins needed to make ``amount``, or ``None``.

    Each denomination may be used an unlimited number of times. A negative
    amount or any nonpositive denomination is rejected with ``ValueError``.
    Duplicate denominations are allowed and do not affect correctness.
    """
    if amount < 0:
        raise ValueError("amount must be nonnegative")

    denominations = list(coins)
    if any(coin <= 0 for coin in denominations):
        raise ValueError("coin denominations must be positive")

    # dp[value] stores the fewest coins needed to form value; infinity marks
    # values not yet reachable. Iterating values upward permits unbounded use.
    unreachable = amount + 1
    dp = [unreachable] * (amount + 1)
    dp[0] = 0

    for value in range(1, amount + 1):
        for coin in denominations:
            if coin <= value:
                dp[value] = min(dp[value], dp[value - coin] + 1)

    return None if dp[amount] == unreachable else dp[amount]


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


if __name__ == "__main__":
    unittest.main()
