'use strict';
/**
 * Return the stock span for each day's price: the number of consecutive days
 * ending today whose prices are less than or equal to today's price.
 *
 * Prices must be finite JavaScript Numbers. The input is not modified.
 * Time: O(n), since every index is pushed and popped at most once.
 * Auxiliary space: O(n) for the monotonic stack and returned result.
 *
 * @param {number[]} prices
 * @returns {number[]}
 * @throws {TypeError} If prices is not an array of finite numbers.
 */
function stockSpan(prices) {
  if (!Array.isArray(prices)) {
    throw new TypeError('prices must be an array of finite numbers');
  }
  for (let i = 0; i < prices.length; i += 1) {
    if (typeof prices[i] !== 'number' || !Number.isFinite(prices[i])) {
      throw new TypeError(`prices[${i}] must be a finite number`);
    }
  }

  const spans = new Array(prices.length);
  const stack = []; // Indices of unresolved previous strictly greater prices.
  for (let i = 0; i < prices.length; i += 1) {
    while (stack.length > 0 && prices[stack[stack.length - 1]] <= prices[i]) {
      stack.pop();
    }
    spans[i] = stack.length === 0 ? i + 1 : i - stack[stack.length - 1];
    stack.push(i);
  }
  return spans;
}

module.exports = stockSpan;

if (require.main === module) {
  const assert = require('node:assert/strict');

  assert.deepEqual(stockSpan([100, 80, 60, 70, 60, 75, 85]), [1, 1, 1, 2, 1, 4, 6], 'canonical example');
  assert.deepEqual(stockSpan([1, 2, 3, 4, 5]), [1, 2, 3, 4, 5], 'increasing prices');
  assert.deepEqual(stockSpan([5, 4, 3, 2, 1]), [1, 1, 1, 1, 1], 'decreasing prices');
  assert.deepEqual(stockSpan([7, 7, 7, 7]), [1, 2, 3, 4], 'equal prices count in the span');
  assert.deepEqual(stockSpan([]), [], 'empty input');
  assert.deepEqual(stockSpan([30, 35, 35, 34, 40]), [1, 2, 3, 1, 5], 'duplicates and intervening lower price');
  assert.deepEqual(stockSpan([-2, -1, -1, -3]), [1, 2, 3, 1], 'finite negative prices');
  assert.deepEqual(stockSpan([1.25, 1.5, 1.4]), [1, 2, 1], 'finite fractional prices');
  assert.throws(() => stockSpan(null), TypeError, 'non-array input');
  assert.throws(() => stockSpan('1,2,3'), TypeError, 'string input');
  assert.throws(() => stockSpan([1, '2']), TypeError, 'non-number element');
  assert.throws(() => stockSpan([NaN]), TypeError, 'NaN element');
  assert.throws(() => stockSpan([Infinity]), TypeError, 'infinite element');
  assert.throws(() => stockSpan([, 1]), TypeError, 'sparse array hole');

  // Deterministic generated cases checked against the direct O(n^2) definition.
  function quadraticReference(values) {
    return values.map((price, day) => {
      let span = 1;
      for (let previous = day - 1; previous >= 0 && values[previous] <= price; previous -= 1) {
        span += 1;
      }
      return span;
    });
  }
  let state = 0x5eed1234;
  function nextRandom() {
    state = (Math.imul(state, 1664525) + 1013904223) >>> 0;
    return state;
  }
  for (let caseIndex = 0; caseIndex < 250; caseIndex += 1) {
    const length = nextRandom() % 41;
    const values = Array.from({ length }, () => (nextRandom() % 17) - 8);
    assert.deepEqual(stockSpan(values), quadraticReference(values), `quadratic comparison case ${caseIndex}`);
  }

  const unchangedInput = [3, 1, 2];
  stockSpan(unchangedInput);
  assert.deepEqual(unchangedInput, [3, 1, 2], 'input remains unchanged');
  console.log('All stockSpan tests passed.');
}
