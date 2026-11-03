/**
 * Return the UTF-16 code-unit index of the first occurrence of `pattern` in
 * `text`, or -1 if it is absent. An empty pattern matches at index 0.
 *
 * Both matching and prefix-function construction operate on JavaScript
 * UTF-16 code units (the same indexing model used by String.prototype.length
 * and charCodeAt), not Unicode code points or grapheme clusters.
 *
 * Time: O(n + m), where n and m are the UTF-16 code-unit lengths of text and
 * pattern. Auxiliary space: O(m) for the prefix-function table.
 *
 * @param {string} text
 * @param {string} pattern
 * @returns {number}
 */
function kmpSearch(text, pattern) {
  if (typeof text !== 'string' || typeof pattern !== 'string') {
    throw new TypeError('text and pattern must be strings');
  }

  const n = text.length;
  const m = pattern.length;
  if (m === 0) return 0;
  if (m > n) return -1;

  const prefix = buildPrefixFunction(pattern);
  let textIndex = 0;
  let patternIndex = 0;

  while (textIndex < n) {
    if (text.charCodeAt(textIndex) === pattern.charCodeAt(patternIndex)) {
      textIndex += 1;
      patternIndex += 1;
      if (patternIndex === m) return textIndex - m;
    } else if (patternIndex > 0) {
      patternIndex = prefix[patternIndex - 1];
    } else {
      textIndex += 1;
    }
  }

  return -1;
}

/**
 * Build the KMP prefix function: prefix[i] is the length of the longest
 * proper prefix of pattern[0..i] that is also a suffix of pattern[0..i].
 * String indexing here is likewise by UTF-16 code unit.
 *
 * @param {string} pattern
 * @returns {number[]}
 */
function buildPrefixFunction(pattern) {
  const prefix = new Array(pattern.length).fill(0);
  let matched = 0;

  for (let i = 1; i < pattern.length; i += 1) {
    const unit = pattern.charCodeAt(i);
    while (matched > 0 && unit !== pattern.charCodeAt(matched)) {
      matched = prefix[matched - 1];
    }
    if (unit === pattern.charCodeAt(matched)) matched += 1;
    prefix[i] = matched;
  }

  return prefix;
}

module.exports = kmpSearch;

// Directly runnable tests: node javascript/kmp_search.js
if (require.main === module) {
  const assert = require('node:assert/strict');
  const { test } = require('node:test');

  test('handles repeated-prefix adversarial strings', () => {
    const text = 'a'.repeat(20000) + 'b';
    assert.equal(kmpSearch(text, 'a'.repeat(10000) + 'c'), -1);
    assert.equal(kmpSearch(text, 'a'.repeat(20000) + 'b'), 0);
  });

  test('finds the first match across overlapping candidates', () => {
    assert.equal(kmpSearch('ababababac', 'ababac'), 4);
    assert.equal(kmpSearch('aaaaa', 'aaa'), 0);
    assert.equal(kmpSearch('abcabcab', 'abcab'), 0);
  });

  test('handles empty text and empty pattern', () => {
    assert.equal(kmpSearch('', ''), 0);
    assert.equal(kmpSearch('abc', ''), 0);
    assert.equal(kmpSearch('', 'a'), -1);
  });

  test('returns -1 when the pattern is absent', () => {
    assert.equal(kmpSearch('the quick brown fox', 'slow'), -1);
    assert.equal(kmpSearch('abc', 'abcd'), -1);
  });

  test('uses UTF-16 code-unit indexing for Unicode', () => {
    assert.equal(kmpSearch('A😀B😀', '😀B'), 1);
    // The low surrogate is a single UTF-16 code unit at index 2.
    assert.equal(kmpSearch('A😀B', '\uDE00'), 2);
    assert.equal(kmpSearch('😀😀', '😀'), 0);
  });
}
