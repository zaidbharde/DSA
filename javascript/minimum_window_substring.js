'use strict';

/**
 * Return the shortest substring of `source` containing every UTF-16 code unit
 * in `target`, including the required multiplicities. If several shortest
 * windows exist, return the earliest one. Return an empty string when target
 * is empty or no covering window exists.
 *
 * Time: O(n + m), where n and m are the UTF-16 code-unit lengths of source
 * and target. Auxiliary space: O(u), where u is the number of distinct
 * UTF-16 code units in target.
 *
 * @param {string} source
 * @param {string} target
 * @returns {string}
 * @throws {TypeError} If either argument is not a string.
 */
function minimumWindowSubstring(source, target) {
  if (typeof source !== 'string' || typeof target !== 'string') {
    throw new TypeError('source and target must be strings');
  }
  if (target.length === 0 || source.length === 0 || target.length > source.length) {
    return '';
  }

  const needed = new Map();
  for (let i = 0; i < target.length; i += 1) {
    const unit = target.charAt(i);
    needed.set(unit, (needed.get(unit) || 0) + 1);
  }

  const windowCounts = new Map();
  let missing = target.length;
  let left = 0;
  let bestStart = 0;
  let bestLength = Infinity;

  for (let right = 0; right < source.length; right += 1) {
    const unit = source.charAt(right);
    if (needed.has(unit)) {
      const count = windowCounts.get(unit) || 0;
      windowCounts.set(unit, count + 1);
      if (count < needed.get(unit)) missing -= 1;
    }

    while (missing === 0) {
      const length = right - left + 1;
      // Strict comparison preserves the earliest window on equal-length ties.
      if (length < bestLength) {
        bestStart = left;
        bestLength = length;
      }

      const removed = source.charAt(left);
      if (needed.has(removed)) {
        const count = windowCounts.get(removed);
        if (count <= needed.get(removed)) missing += 1;
        windowCounts.set(removed, count - 1);
      }
      left += 1;
    }
  }

  return bestLength === Infinity ? '' : source.slice(bestStart, bestStart + bestLength);
}

module.exports = minimumWindowSubstring;

if (require.main === module) {
  const assert = require('node:assert/strict');

  assert.equal(minimumWindowSubstring('ADOBECODEBANC', 'ABC'), 'BANC');
  assert.equal(minimumWindowSubstring('a', 'a'), 'a');
  assert.equal(minimumWindowSubstring('a', 'aa'), '');
  assert.equal(minimumWindowSubstring('AAABBC', 'AABC'), 'AABBC'); // duplicates
  assert.equal(minimumWindowSubstring('aaab', 'aab'), 'aab'); // overlapping candidates
  assert.equal(minimumWindowSubstring('', 'a'), '');
  assert.equal(minimumWindowSubstring('abc', ''), '');
  assert.equal(minimumWindowSubstring('abc', 'z'), '');
  assert.equal(minimumWindowSubstring('abxxba', 'ab'), 'ab'); // earliest tie

  const emoji = '\uD83D\uDE00';
  assert.equal(minimumWindowSubstring(emoji, '\uDE00'), '\uDE00');
  assert.equal(minimumWindowSubstring(emoji, emoji), emoji);
  assert.equal(minimumWindowSubstring(emoji, '\uD83D\uD83D'), '');

  assert.throws(() => minimumWindowSubstring(null, 'a'), TypeError);
  assert.throws(() => minimumWindowSubstring('a', undefined), TypeError);

  console.log('All minimum-window-substring assertions passed.');
}
