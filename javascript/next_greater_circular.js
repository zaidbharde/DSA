'use strict';

/**
 * Return the next strictly greater value in circular order for each element,
 * or -1 if no such value exists. Equal values do not qualify as greater.
 *
 * Time: O(n). Auxiliary space: O(n).
 * @param {number[]} values
 * @returns {number[]}
 * @throws {TypeError} If values is not an array of finite numbers.
 */
function nextGreaterCircular(values) {
  if (!Array.isArray(values)) {
    throw new TypeError('Expected an array of finite numbers');
  }
  for (const value of values) {
    if (typeof value !== 'number' || !Number.isFinite(value)) {
      throw new TypeError('Expected an array of finite numbers');
    }
  }

  const length = values.length;
  const result = new Array(length).fill(-1);
  const stack = []; // Indices whose next strictly greater value is not found yet.

  // Scan two logical copies to account for wraparound, but push each index only
  // during the first copy. Strict < preserves duplicate values as unresolved.
  for (let step = 0; step < 2 * length; step += 1) {
    const index = step % length;
    const current = values[index];

    while (stack.length > 0 && values[stack[stack.length - 1]] < current) {
      result[stack.pop()] = current;
    }

    if (step < length) {
      stack.push(index);
    }
  }

  return result;
}

module.exports = nextGreaterCircular;

if (require.main === module) {
  const assert = require('node:assert/strict');

  assert.deepEqual(nextGreaterCircular([]), []);
  assert.deepEqual(nextGreaterCircular([42]), [-1]);
  assert.deepEqual(nextGreaterCircular([2, 2, 2]), [-1, -1, -1]);
  assert.deepEqual(nextGreaterCircular([1, 2, 2, 3]), [2, 3, 3, -1]);
  assert.deepEqual(nextGreaterCircular([-3, -2, -4]), [-2, -1, -3]);
  assert.deepEqual(nextGreaterCircular([3, 1, 2]), [-1, 2, 3]);
  assert.deepEqual(nextGreaterCircular([9, 8, 7]), [-1, 9, 9]);
  assert.throws(() => nextGreaterCircular(null), TypeError);
  assert.throws(() => nextGreaterCircular([1, NaN]), TypeError);
  assert.throws(() => nextGreaterCircular([Infinity]), TypeError);

  console.log('All nextGreaterCircular assertions passed.');
}
