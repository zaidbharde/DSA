"""Decide whether text can be segmented into words from a dictionary.

The dynamic program performs O(n^2) expected dictionary-membership checks for
n characters, assuming constant-time substring creation and set lookup (the
usual analysis of the simple substring/set approach). In CPython, slicing and
hashing a candidate substring can take time proportional to its length, so the
worst-case character-processing time can be O(n^3). Auxiliary storage is
O(n + D), where D is the total number of characters in the dictionary; this
includes the reachable-prefix table and the copied set of words.
"""

from collections.abc import Iterable
import unittest


def word_break(text: str, words: Iterable[str]) -> bool:
    """Return whether *text* is a concatenation of words in *words*.

    The empty text is segmentable. ``text`` must be a string and ``words``
    must be an iterable other than a string/bytes object containing only
    non-empty strings. Invalid argument types raise ``TypeError``; empty
    dictionary entries raise ``ValueError``.
    """
    if not isinstance(text, str):
        raise TypeError("text must be a string")
    if isinstance(words, (str, bytes)) or not isinstance(words, Iterable):
        raise TypeError("words must be an iterable of non-empty strings")

    dictionary = set()
    for word in words:
        if not isinstance(word, str):
            raise TypeError("every dictionary entry must be a string")
        if not word:
            raise ValueError("dictionary entries must not be empty")
        dictionary.add(word)

    n = len(text)
    reachable = [False] * (n + 1)
    reachable[0] = True

    for end in range(1, n + 1):
        for start in range(end):
            if reachable[start] and text[start:end] in dictionary:
                reachable[end] = True
                break

    return reachable[n]


class WordBreakTests(unittest.TestCase):
    def test_positive_segmentation(self):
        self.assertTrue(word_break("leetcode", ["leet", "code"]))
        self.assertTrue(word_break("applepenapple", {"apple", "pen"}))

    def test_negative_segmentation(self):
        self.assertFalse(word_break("catsandog", ["cats", "dog", "sand", "and", "cat"]))
        self.assertFalse(word_break("anything", []))

    def test_overlapping_prefixes_and_backtracking(self):
        self.assertTrue(word_break("aaaaab", ["a", "aa", "aaa", "aaaa", "ab"]))
        self.assertTrue(word_break("cars", ["car", "ca", "rs"]))

    def test_empty_text_is_segmentable(self):
        self.assertTrue(word_break("", []))
        self.assertTrue(word_break("", ["word"]))

    def test_invalid_text_type(self):
        with self.assertRaises(TypeError):
            word_break(123, ["123"])

    def test_invalid_dictionary_type(self):
        with self.assertRaises(TypeError):
            word_break("text", "text")
        with self.assertRaises(TypeError):
            word_break("text", None)

    def test_invalid_dictionary_entries(self):
        with self.assertRaises(ValueError):
            word_break("text", ["text", ""])
        with self.assertRaises(TypeError):
            word_break("text", ["text", 3])


if __name__ == "__main__":
    unittest.main()
