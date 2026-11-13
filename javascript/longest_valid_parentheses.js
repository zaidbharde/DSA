'use strict';

/**
 * Return the length of the longest contiguous, well-formed parentheses
 * substring. The input must contain only '(' and ')' characters.
 *
 * Time: O(n). Auxiliary space: O(n), where n is the input string length.
 *
 * @param {string} text Parentheses-only string to inspect.
 * @returns {number} Length of its longest valid parentheses substring.
 * @throws {TypeError} If text is not a string or contains other characters.
 */
function longestValidParentheses(text) {
  if (typeof text !== 'string') {
    throw new TypeError('text must be a string containing only parentheses');
  }

  const stack = [-1]; // Index before the current valid candidate's start.
  let longest = 0;

  for (let i = 0; i < text.length; i += 1) {
    const character = text[i];
    if (character === '(') {
      stack.push(i);
    } else if (character === ')') {
      stack.pop();
      if (stack.length === 0) {
        // This close cannot be matched; it becomes the next left boundary.
        stack.push(i);
      } else {
        longest = Math.max(longest, i - stack[stack.length - 1]);
      }
    } else {
      throw new TypeError('text must be a string containing only parentheses');
    }
  }

  return longest;
}

module.exports = longestValidParentheses;

if (require.main === module) {
  const assert = require('node:assert/strict');

  // Deliberately simple oracle: enumerate every substring and check its
  // balance. This is only used on short deterministic test inputs.
  function referenceLongestValidParentheses(text) {
    let best = 0;
    for (let start = 0; start < text.length; start += 1) {
      let balance = 0;
      for (let end = start; end < text.length; end += 1) {
        balance += text[end] === '(' ? 1 : -1;
        if (balance < 0) {
          break;
        }
        if (balance === 0) {
          best = Math.max(best, end - start + 1);
        }
      }
    }
    return best;
  }

  assert.equal(longestValidParentheses(''), 0, 'empty input');
  assert.equal(longestValidParentheses('((()))'), 6, 'nested groups');
  assert.equal(longestValidParentheses('()()()'), 6, 'concatenated groups');
  assert.equal(longestValidParentheses(')((()))'), 6, 'unmatched prefix');
  assert.equal(longestValidParentheses('(()())('), 6, 'unmatched suffix');
  assert.equal(longestValidParentheses('((('), 0, 'repeated opens');
  assert.equal(longestValidParentheses(')))'), 0, 'repeated closes');
  assert.equal(longestValidParentheses(')()())'), 4, 'valid group after unmatched close');
  assert.throws(() => longestValidParentheses(null), TypeError, 'non-string input');
  assert.throws(() => longestValidParentheses(123), TypeError, 'numeric input');
  assert.throws(() => longestValidParentheses('()x()'), TypeError, 'non-parenthesis character');

  // Exhaust all strings through length 10 in a fixed order, comparing each
  // result against the independent reference implementation.
  for (let length = 0; length <= 10; length += 1) {
    for (let mask = 0; mask < 2 ** length; mask += 1) {
      let text = '';
      for (let bit = 0; bit < length; bit += 1) {
        text += (mask & (1 << bit)) === 0 ? '(' : ')';
      }
      assert.equal(
        longestValidParentheses(text),
        referenceLongestValidParentheses(text),
        `reference comparison for ${JSON.stringify(text)}`
      );
    }
  }

  console.log('All longestValidParentheses tests passed.');
}
