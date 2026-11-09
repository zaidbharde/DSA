'use strict';

/**
 * Return the largest rectangle area in a histogram.
 *
 * Heights must be finite, nonnegative Numbers. Throws RangeError if a
 * candidate area is non-finite or exceeds Number.MAX_SAFE_INTEGER.
 *
 * Time: O(n); space: O(n), where n is the number of bars.
 *
 * @param {number[]} heights
 * @returns {number}
 */
function largestRectangleArea(heights) {
  if (!Array.isArray(heights)) {
    throw new TypeError('heights must be an array');
  }

  for (let i = 0; i < heights.length; i += 1) {
    const height = heights[i];
    if (typeof height !== 'number' || !Number.isFinite(height) || height < 0) {
      throw new TypeError(`heights[${i}] must be a finite, nonnegative number`);
    }
  }

  const stack = [];
  let maxArea = 0;

  // The zero-height sentinel flushes bars remaining on the stack.
  for (let i = 0; i <= heights.length; i += 1) {
    const currentHeight = i === heights.length ? 0 : heights[i];

    while (stack.length > 0 && heights[stack[stack.length - 1]] >= currentHeight) {
      const height = heights[stack.pop()];
      const leftBoundary = stack.length > 0 ? stack[stack.length - 1] : -1;
      const width = i - leftBoundary - 1;
      const area = height * width;

      if (!Number.isFinite(area) || area > Number.MAX_SAFE_INTEGER) {
        throw new RangeError('rectangle area exceeds the safe Number range');
      }
      if (area > maxArea) {
        maxArea = area;
      }
    }

    if (i < heights.length) {
      stack.push(i);
    }
  }

  return maxArea;
}

module.exports = largestRectangleArea;

if (require.main === module) {
  const assert = require('node:assert/strict');

  assert.equal(largestRectangleArea([]), 0, 'empty histogram');
  assert.equal(largestRectangleArea([7]), 7, 'single bar');
  assert.equal(largestRectangleArea([2, 1, 5, 6, 2, 3]), 10, 'classic example');
  assert.equal(largestRectangleArea([1, 2, 3, 4, 5]), 9, 'increasing heights');
  assert.equal(largestRectangleArea([5, 4, 3, 2, 1]), 9, 'decreasing heights');
  assert.equal(largestRectangleArea([0, 0, 0]), 0, 'zero heights');
  assert.equal(largestRectangleArea([2, 2, 2]), 6, 'duplicate heights');
  assert.equal(largestRectangleArea([1.5, 2]), 3, 'finite fractional heights');

  assert.throws(() => largestRectangleArea(null), TypeError, 'input must be an array');
  assert.throws(() => largestRectangleArea([-1]), TypeError, 'negative heights are invalid');
  assert.throws(() => largestRectangleArea([NaN]), TypeError, 'NaN is invalid');
  assert.throws(() => largestRectangleArea([Infinity]), TypeError, 'infinite heights are invalid');
  assert.throws(
    () => largestRectangleArea([Number.MAX_SAFE_INTEGER, Number.MAX_SAFE_INTEGER]),
    RangeError,
    'unsafe integer area is rejected'
  );
  assert.throws(
    () => largestRectangleArea([Number.MAX_VALUE]),
    RangeError,
    'non-finite area is rejected'
  );

  console.log('All largest rectangle histogram tests passed.');
}
