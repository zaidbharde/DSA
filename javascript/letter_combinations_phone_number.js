'use strict';

const PHONE_LETTERS = {
  '2': 'abc',
  '3': 'def',
  '4': 'ghi',
  '5': 'jkl',
  '6': 'mno',
  '7': 'pqrs',
  '8': 'tuv',
  '9': 'wxyz',
};

/**
 * Returns every letter combination represented by the given phone digits.
 *
 * @param {string} digits A string containing only digits 2 through 9.
 * @returns {string[]} Combinations in keypad order, or [] for empty input.
 * @throws {TypeError} If digits is not a string.
 * @throws {RangeError} If digits contains anything other than 2 through 9.
 *
 * Worst-case time is O(4^n * n), including materializing the returned strings.
 * Auxiliary recursion/path space is O(n), excluding the returned output.
 */
function letterCombinations(digits) {
  if (typeof digits !== 'string') {
    throw new TypeError('digits must be a string');
  }
  if (digits.length === 0) {
    return [];
  }
  if (!/^[2-9]+$/.test(digits)) {
    throw new RangeError('digits must contain only characters 2 through 9');
  }

  const combinations = [];
  const path = [];

  function backtrack(index) {
    if (index === digits.length) {
      combinations.push(path.join(''));
      return;
    }

    const letters = PHONE_LETTERS[digits[index]];
    for (const letter of letters) {
      path.push(letter);
      backtrack(index + 1);
      path.pop();
    }
  }

  backtrack(0);
  return combinations;
}

module.exports = letterCombinations;

if (require.main === module) {
  const { test } = require('node:test');
  const assert = require('node:assert/strict');

  test('empty input returns an empty list', () => {
    assert.deepEqual(letterCombinations(''), []);
  });

  test('one digit returns its letters in keypad order', () => {
    assert.deepEqual(letterCombinations('2'), ['a', 'b', 'c']);
  });

  test('multiple digits produce all combinations', () => {
    assert.deepEqual(letterCombinations('23'), [
      'ad', 'ae', 'af', 'bd', 'be', 'bf', 'cd', 'ce', 'cf',
    ]);
  });

  test('ordering follows the keypad letters at each position', () => {
    assert.deepEqual(letterCombinations('79'), [
      'pw', 'px', 'py', 'pz',
      'qw', 'qx', 'qy', 'qz',
      'rw', 'rx', 'ry', 'rz',
      'sw', 'sx', 'sy', 'sz',
    ]);
  });

  test('non-string input is rejected clearly', () => {
    assert.throws(() => letterCombinations(23), {
      name: 'TypeError',
      message: 'digits must be a string',
    });
  });

  test('digits 0 and 1 and other invalid characters are rejected clearly', () => {
    for (const invalid of ['0', '1', '20', '2a', ' 2']) {
      assert.throws(() => letterCombinations(invalid), {
        name: 'RangeError',
        message: 'digits must contain only characters 2 through 9',
      });
    }
  });
}
