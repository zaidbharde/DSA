'use strict';

/**
 * Compute the maximum value achievable in a 0/1 knapsack.
 *
 * Each item can be selected at most once. Weights and capacity must be
 * nonnegative safe integers; values must be finite numbers (negative values
 * are allowed). Empty item arrays return 0. Zero-weight items are considered
 * exactly once and are included when they increase the total value. Capacity
 * must also fit in a JavaScript array-backed DP table.
 *
 * Time: O(nW), where n is the number of items and W is the capacity.
 * Auxiliary space: O(W), excluding the input arrays.
 *
 * @param {number[]} weights Nonnegative integer item weights.
 * @param {number[]} values Finite item values, including negative values.
 * @param {number} capacity Nonnegative integer knapsack capacity.
 * @returns {number} Maximum total value without exceeding capacity.
 */
function knapsack01(weights, values, capacity) {
  if (!Array.isArray(weights) || !Array.isArray(values)) {
    throw new TypeError('weights and values must be arrays');
  }
  if (weights.length !== values.length) {
    throw new RangeError('weights and values must have the same length');
  }
  if (!Number.isSafeInteger(capacity) || capacity < 0) {
    throw new RangeError('capacity must be a nonnegative safe integer');
  }
  // JavaScript arrays can have at most 2^32 - 1 elements; the DP needs W + 1.
  if (capacity >= 0xffffffff) {
    throw new RangeError('capacity is too large for an array-backed DP table');
  }

  for (let i = 0; i < weights.length; i += 1) {
    if (!Number.isSafeInteger(weights[i]) || weights[i] < 0) {
      throw new RangeError(`weight at index ${i} must be a nonnegative safe integer`);
    }
    if (typeof values[i] !== 'number' || !Number.isFinite(values[i])) {
      throw new TypeError(`value at index ${i} must be a finite number`);
    }
  }

  const best = new Array(capacity + 1).fill(0);

  for (let i = 0; i < weights.length; i += 1) {
    const weight = weights[i];
    const value = values[i];

    // Descending capacities prevent reusing the current item. For a
    // zero-weight item, each capacity is visited once for that item.
    for (let currentCapacity = capacity; currentCapacity >= weight; currentCapacity -= 1) {
      best[currentCapacity] = Math.max(
        best[currentCapacity],
        best[currentCapacity - weight] + value,
      );
    }
  }

  return best[capacity];
}

module.exports = knapsack01;

if (require.main === module) {
  const assert = require('node:assert/strict');

  // Independent reference: each row uses only the preceding item's row.
  function referenceKnapsack(weights, values, capacity) {
    const dp = Array.from({ length: weights.length + 1 }, () =>
      new Array(capacity + 1).fill(0),
    );
    for (let i = 1; i <= weights.length; i += 1) {
      for (let currentCapacity = 0; currentCapacity <= capacity; currentCapacity += 1) {
        dp[i][currentCapacity] = dp[i - 1][currentCapacity];
        if (weights[i - 1] <= currentCapacity) {
          dp[i][currentCapacity] = Math.max(
            dp[i][currentCapacity],
            dp[i - 1][currentCapacity - weights[i - 1]] + values[i - 1],
          );
        }
      }
    }
    return dp[weights.length][capacity];
  }

  // Compare every ordered item list of length up to four from this small
  // weight/value domain, across capacities 0..5.
  const choices = [];
  for (const weight of [0, 1, 2, 3]) {
    for (const value of [-2, 0, 1, 3]) choices.push([weight, value]);
  }
  function compareAllPrefixes(weights, values, remaining) {
    for (let capacity = 0; capacity <= 5; capacity += 1) {
      assert.equal(
        knapsack01(weights, values, capacity),
        referenceKnapsack(weights, values, capacity),
        `weights=${JSON.stringify(weights)}, values=${JSON.stringify(values)}, capacity=${capacity}`,
      );
    }
    if (remaining === 0) return;
    for (const [weight, value] of choices) {
      weights.push(weight);
      values.push(value);
      compareAllPrefixes(weights, values, remaining - 1);
      weights.pop();
      values.pop();
    }
  }
  compareAllPrefixes([], [], 4);

  // Canonical optimum: items 2 and 3 yield value 220 at capacity 50.
  assert.equal(knapsack01([10, 20, 30], [60, 100, 120], 50), 220);
  assert.equal(knapsack01([], [], 10), 0);
  assert.equal(knapsack01([], [], 0), 0);
  assert.equal(knapsack01([1], [5], 0), 0);
  assert.equal(knapsack01([0], [7], 0), 7);
  assert.equal(knapsack01([0, 0, 2], [4, 3, 10], 2), 17);
  assert.equal(knapsack01([0, 0], [4, 3], 0), 7);
  assert.equal(knapsack01([2, 3], [4, 5], 1), 0);
  assert.equal(knapsack01([2, 3], [4, 5], 2), 4);
  assert.equal(knapsack01([2, 3], [4, 5], 5), 9);
  assert.equal(knapsack01([6, 7], [9, 10], 5), 0);
  assert.equal(knapsack01([1, 2], [-5, -1], 2), 0);
  assert.equal(knapsack01([0, 1], [-3, 5], 1), 5);

  assert.throws(() => knapsack01('1', [1], 1), TypeError);
  assert.throws(() => knapsack01([1], null, 1), TypeError);
  assert.throws(() => knapsack01([1], [], 1), RangeError);
  assert.throws(() => knapsack01([1], [1], -1), RangeError);
  assert.throws(() => knapsack01([1], [1], 1.5), RangeError);
  assert.throws(() => knapsack01([1], [1], 0xffffffff), {
    name: 'RangeError',
    message: 'capacity is too large for an array-backed DP table',
  });
  assert.throws(() => knapsack01([-1], [1], 1), RangeError);
  assert.throws(() => knapsack01([1.25], [1], 1), RangeError);
  assert.throws(() => knapsack01([1], [NaN], 1), TypeError);

  console.log('All knapsack_01 tests passed.');
}
