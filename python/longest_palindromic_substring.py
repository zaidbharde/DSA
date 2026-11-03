"""Find a longest palindromic substring using center expansion.

Strings are treated as sequences of Unicode code points, as in Python's
normal string indexing. No Unicode normalization or grapheme-cluster grouping
is performed.

Time complexity: O(n^2). Auxiliary space: O(1), excluding the returned
substring.
"""


def longest_palindromic_substring(text: str) -> str:
    """Return the longest contiguous palindrome in *text*.

    If multiple longest palindromes exist, return the one whose occurrence
    starts earliest. The empty string is returned for empty input.
    """
    if not text:
        return ""

    best_start = 0
    best_length = 1
    n = len(text)

    def expand(left: int, right: int) -> tuple[int, int]:
        """Expand around a center; return inclusive bounds of its palindrome."""
        while left >= 0 and right < n and text[left] == text[right]:
            left -= 1
            right += 1
        return left + 1, right - left - 1

    for center in range(n):
        # Odd-length palindrome centered on one code point.
        for left, length in (expand(center, center), expand(center, center + 1)):
            if length > best_length or (
                length == best_length and left < best_start
            ):
                best_start = left
                best_length = length

    return text[best_start : best_start + best_length]


if __name__ == "__main__":
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

    unittest.main()
