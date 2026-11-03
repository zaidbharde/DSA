"""Decide whether text can be segmented into words from a dictionary.

The trie construction takes expected O(D) time, where D is the total number
of dictionary characters. The dynamic program starts a trie walk only at
reachable prefix positions and examines at most L characters per position,
for expected O(D + n*L) construction/search time, where n is the text length
and L is the maximum word length. The trie and reachable-prefix table use
O(D + n) storage.
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

    # Trie nodes map characters to child nodes. None marks a completed word;
    # it cannot collide with a character key because string characters are str.
    trie = {}
    max_word_length = 0
    for word in words:
        if not isinstance(word, str):
            raise TypeError("every dictionary entry must be a string")
        if not word:
            raise ValueError("dictionary entries must not be empty")
        max_word_length = max(max_word_length, len(word))
        node = trie
        for character in word:
            node = node.setdefault(character, {})
        node[None] = True

    n = len(text)
    reachable = [False] * (n + 1)
    reachable[0] = True

    for start in range(n):
        if not reachable[start]:
            continue
        node = trie
        stop = min(n, start + max_word_length)
        for end in range(start, stop):
            node = node.get(text[end])
            if node is None:
                break
            if None in node:
                reachable[end + 1] = True

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

    def test_stress_overlapping_prefixes(self):
        words = ["a" * length for length in range(1, 65)]
        self.assertFalse(word_break("a" * 1200 + "b", words))
        self.assertTrue(word_break("a" * 1200, words))

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
