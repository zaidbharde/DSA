"""Full-string matching for the ``.`` and ``*`` regular-expression operators.

A dot matches any one character. A star matches zero or more occurrences of
its immediately preceding literal or dot. The implementation uses dynamic
programming in O(n * m) time and O(m) auxiliary space, where n is the text
length and m is the pattern length.
"""

import unittest


def is_match(text: str, pattern: str) -> bool:
    """Return whether ``text`` fully matches a simple regular-expression pattern.

    The supported operators are ``.`` (any single character) and ``*`` (zero
    or more repetitions of the preceding token). Every other pattern character
    is treated literally. Matching is anchored to the entire text; there is no
    implicit substring search.

    Raises:
        TypeError: If ``text`` or ``pattern`` is not a string.
        ValueError: If ``pattern`` starts with ``*`` or contains consecutive
            stars, since either case has no preceding token for a star.

    Let D[i][j] mean that the first i text characters match the first j
    pattern characters. A non-star token matches when its character is a dot
    or equals the current text character, and then D[i][j] = D[i-1][j-1].
    For a star at pattern position j, D[i][j] is true either by using zero
    copies of its preceding token (D[i][j-2]), or by matching one more
    character with that token and retaining the star (D[i-1][j]).

    The table is compressed to one row indexed by pattern prefixes. While
    processing each text character from left to right, the old value at j is
    the value from the previous text row, and the already-updated value at
    j-2 supplies the zero-copy transition. A saved diagonal value supplies
    D[i-1][j-1] for non-star tokens. This uses O(m) auxiliary space.
    """
    if not isinstance(text, str):
        raise TypeError("text must be a string")
    if not isinstance(pattern, str):
        raise TypeError("pattern must be a string")

    for index, char in enumerate(pattern):
        if char == "*" and (index == 0 or pattern[index - 1] == "*"):
            raise ValueError("'*' must follow a non-star pattern token")

    pattern_length = len(pattern)
    dp = [False] * (pattern_length + 1)
    dp[0] = True

    # With empty text, only stars can extend a matching pattern prefix,
    # each by matching zero copies of their preceding token.
    for j in range(2, pattern_length + 1):
        if pattern[j - 1] == "*":
            dp[j] = dp[j - 2]

    for text_char in text:
        previous_diagonal = dp[0]
        dp[0] = False
        for j in range(1, pattern_length + 1):
            previous_row_value = dp[j]
            pattern_char = pattern[j - 1]

            if pattern_char == "*":
                repeated_token = pattern[j - 2]
                matches_one = repeated_token == "." or repeated_token == text_char
                dp[j] = dp[j - 2] or (matches_one and previous_row_value)
            else:
                token_matches = pattern_char == "." or pattern_char == text_char
                dp[j] = previous_diagonal and token_matches

            previous_diagonal = previous_row_value

    return dp[pattern_length]


class RegexMatchingTests(unittest.TestCase):
    def test_empty_text_and_pattern(self):
        self.assertTrue(is_match("", ""))

    def test_empty_text_with_stars(self):
        self.assertTrue(is_match("", "a*"))
        self.assertTrue(is_match("", "a*b*c*"))
        self.assertFalse(is_match("", "a"))

    def test_classic_matches(self):
        self.assertTrue(is_match("aa", "a*"))
        self.assertTrue(is_match("aab", "c*a*b"))
        self.assertTrue(is_match("aaa", "ab*a*c*a"))

    def test_classic_nonmatches(self):
        self.assertFalse(is_match("aa", "a"))
        self.assertFalse(is_match("ab", ".*c"))
        self.assertFalse(is_match("mississippi", "mis*is*p*."))

    def test_dot_matches_one_character(self):
        self.assertTrue(is_match("a", "."))
        self.assertTrue(is_match("ab", ".."))
        self.assertFalse(is_match("", "."))
        self.assertFalse(is_match("ab", "."))

    def test_repeated_stars(self):
        self.assertTrue(is_match("", "a*"))
        self.assertTrue(is_match("aaaa", "a*"))
        self.assertTrue(is_match("abc", ".*"))
        self.assertTrue(is_match("abbb", "ab*"))
        self.assertFalse(is_match("ac", "ab*"))

    def test_literals_are_matched_exactly(self):
        self.assertTrue(is_match("abc", "abc"))
        self.assertFalse(is_match("abC", "abc"))
        self.assertTrue(is_match("a+b", "a+b"))
        self.assertFalse(is_match("a-b", "a+b"))

    def test_invalid_patterns(self):
        for pattern in ("*a", "**", "a**", "a***b"):
            with self.subTest(pattern=pattern):
                with self.assertRaises(ValueError):
                    is_match("", pattern)

    def test_non_string_arguments(self):
        with self.assertRaises(TypeError):
            is_match(None, "a")
        with self.assertRaises(TypeError):
            is_match("a", None)


if __name__ == "__main__":
    unittest.main()
