'use strict';

/**
 * Return the maximum value in every contiguous window of length k.
 *
 * Input must be an array of finite numbers. k must be a positive integer;
 * for non-empty input it must also be no greater than the array length.
 * An empty array returns [] for any positive-integer k.
 *
 * Runs in O(n) time with O(k) auxiliary space, excluding the returned array.
 * @param {number[]} values
 * @param {number} k
 * @returns {number[]}
 */
function maxSlidingWindow(values, k) {
  if (!Array.isArray(values)) {
    throw new TypeError('values must be an array');
  }
  if (!Number.isInteger(k) || k < 1) {
    throw new RangeError('k must be a positive integer');
  }
  for (let i = 0; i < values.length; i += 1) {
    if (typeof values[i] !== 'number' || !Number.isFinite(values[i])) {
      throw new TypeError(`values[${i}] must be a finite number`);
    }
  }
  if (values.length === 0) {
    return [];
  }
  if (k > values.length) {
    throw new RangeError('k must not exceed the number of values');
  }

  // Circular deque stores indices, from the largest value at the front to
  // the smallest at the back. Its fixed capacity keeps auxiliary space O(k).
  const deque = new Array(k);
  let head = 0;
  let size = 0;
  const result = [];

  const front = () => deque[head];
  const back = () => deque[(head + size - 1) % k];
  const popFront = () => {
    head = (head + 1) % k;
    size -= 1;
  };
  const popBack = () => {
    size -= 1;
  };
  const pushBack = (index) => {
    deque[(head + size) % k] = index;
    size += 1;
  };

  for (let i = 0; i < values.length; i += 1) {
    // Discard indices that have fallen outside the current window.
    while (size > 0 && front() <= i - k) {
      popFront();
    }

    // A newer value at least as large makes older, smaller values obsolete.
    while (size > 0 && values[back()] <= values[i]) {
      popBack();
    }
    pushBack(i);

    if (i >= k - 1) {
      result.push(values[front()]);
    }
  }

  return result;
}

module.exports = maxSlidingWindow;
module.exports.maxSlidingWindow = maxSlidingWindow;

if (require.main === module) {
  const assert = require('node:assert/strict');

  assert.deepEqual(
    maxSlidingWindow([1, 3, -1, -3, 5, 3, 6, 7], 3),
    [3, 3, 5, 5, 6, 7],
  );
  assert.deepEqual(maxSlidingWindow([4, 2, 12, 3], 1), [4, 2, 12, 3]);
  assert.deepEqual(maxSlidingWindow([4, 2, 12, 3], 4), [12]);
  assert.deepEqual(maxSlidingWindow([2, 2, 2, 2], 2), [2, 2, 2]);
  assert.deepEqual(maxSlidingWindow([-8, -3, -5, -10], 2), [-3, -3, -5]);
  assert.deepEqual(maxSlidingWindow([], 1), []);

  assert.throws(() => maxSlidingWindow('not an array', 1), TypeError);
  assert.throws(() => maxSlidingWindow([1, 2], 0), RangeError);
  assert.throws(() => maxSlidingWindow([1, 2], -1), RangeError);
  assert.throws(() => maxSlidingWindow([1, 2], 1.5), RangeError);
  assert.throws(() => maxSlidingWindow([1, 2], 3), RangeError);
  assert.throws(() => maxSlidingWindow([1, NaN], 1), TypeError);

  console.log('All sliding-window maximum tests passed.');
}
