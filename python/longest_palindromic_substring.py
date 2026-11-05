"""Find a longest palindromic substring in linear time with Manacher's algorithm.

Strings are treated as sequences of Unicode code points, as in Python's
normal string indexing. No Unicode normalization or grapheme-cluster grouping
is performed.

Time complexity: O(n). Auxiliary space: O(n), excluding the returned
substring.
"""


def longest_palindromic_substring(text: str) -> str:
    """Return the longest contiguous palindrome in *text*.

    If multiple longest palindromes exist, return the one whose occurrence
    starts earliest. The empty string is returned for empty input.
    """
    n = len(text)
    if n == 0:
        return ""

    best_start = 0
    best_length = 1

    # odd[i] is the radius (including the center) of the longest odd
    # palindrome centered at i.
    odd = [0] * n
    left, right = 0, -1
    for i in range(n):
        radius = 1 if i > right else min(odd[left + right - i], right - i + 1)
        while i - radius >= 0 and i + radius < n and text[i - radius] == text[i + radius]:
            radius += 1
        odd[i] = radius
        start = i - radius + 1
        length = 2 * radius - 1
        if length > best_length or (length == best_length and start < best_start):
            best_start = start
            best_length = length
        if i + radius - 1 > right:
            left, right = i - radius + 1, i + radius - 1

    # even[i] is the radius of the longest even palindrome centered between
    # i - 1 and i.
    even = [0] * n
    left, right = 0, -1
    for i in range(n):
        radius = 0 if i > right else min(even[left + right - i + 1], right - i + 1)
        while i - radius - 1 >= 0 and i + radius < n and text[i - radius - 1] == text[i + radius]:
            radius += 1
        even[i] = radius
        start = i - radius
        length = 2 * radius
        if length > best_length or (length == best_length and start < best_start):
            best_start = start
            best_length = length
        if i + radius - 1 > right:
            left, right = i - radius, i + radius - 1

    return text[best_start : best_start + best_length]


if __name__ == "__main__":
    import itertools
    import unittest

    class LongestPalindromicSubstringTests(unittest.TestCase):
        def test_empty_input(self) -> None:
            self.assertEqual(longest_palindromic_substring(""), "")

        def test_singleton(self) -> None:
            self.assertEqual(longest_palindromic_substring("q"), "q")

        def test_odd_center(self) -> None:
            self.assertEqual(longest_palindromic_substring("babad"), "bab")

        def test_even_center(self) -> None:
            self.assertEqual(longest_palindromic_substring("cbbd"), "bb")

        def test_all_equal(self) -> None:
            self.assertEqual(longest_palindromic_substring("aaaaa"), "aaaaa")

        def test_earliest_of_multiple_tied_answers(self) -> None:
            self.assertEqual(
                longest_palindromic_substring("abacdfgdcaba"), "aba"
            )

        def test_unicode_code_points(self) -> None:
            self.assertEqual(
                longest_palindromic_substring("x🙂y🙂x"), "x🙂y🙂x"
            )

        def test_adversarial_long_repeated_string(self) -> None:
            text = "a" * 100_000
            self.assertEqual(longest_palindromic_substring(text), text)

        def test_exhaustive_short_strings_against_oracle(self) -> None:
            def oracle(text: str) -> str:
                best = ""
                best_start = 0
                for start in range(len(text)):
                    for end in range(start + 1, len(text) + 1):
                        candidate = text[start:end]
                        if candidate == candidate[::-1] and (
                            len(candidate) > len(best)
                            or (len(candidate) == len(best) and start < best_start)
                        ):
                            best = candidate
                            best_start = start
                return best

            for length in range(8):
                for chars in itertools.product("ab🙂", repeat=length):
                    text = "".join(chars)
                    with self.subTest(text=text):
                        self.assertEqual(longest_palindromic_substring(text), oracle(text))

    unittest.main()
