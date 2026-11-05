/**
 * Fenwick (Binary Indexed) Tree for zero-based point additions and sums.
 *
 * prefixSum(endExclusive) returns the sum over [0, endExclusive), so the
 * valid endpoint range is 0 through length inclusive. rangeSum(start, end)
 * returns the sum over [start, end), allowing empty ranges when start === end.
 *
 * Construction from initial values is O(n) using linear Fenwick propagation;
 * add, prefixSum, and rangeSum take O(log n) time. Storage is O(n).
 */
export class FenwickTree {
  private readonly size: number;
  private readonly tree: number[];

  constructor(values: readonly number[]) {
    this.size = values.length;
    this.tree = new Array<number>(this.size + 1).fill(0);

    for (let index = 1; index <= this.size; index += 1) {
      this.tree[index] += values[index - 1];
      const parent = index + (index & -index);
      if (parent <= this.size) {
        this.tree[parent] += this.tree[index];
      }
    }
  }

  /** Number of values represented by this tree. */
  get length(): number {
    return this.size;
  }

  /** Add delta to the value at the zero-based index. */
  add(index: number, delta: number): void {
    this.validateIndex(index);
    for (let node = index + 1; node <= this.size; node += node & -node) {
      this.tree[node] += delta;
    }
  }

  /**
   * Return the sum over [0, endExclusive). The endpoint must be an integer in
   * [0, length]; in particular, prefixSum(0) is 0 and prefixSum(length) is
   * the sum of the complete array. Invalid endpoints throw RangeError.
   */
  prefixSum(endExclusive: number): number {
    this.validateEndpoint(endExclusive, "endExclusive");
    let sum = 0;
    for (let node = endExclusive; node > 0; node -= node & -node) {
      sum += this.tree[node];
    }
    return sum;
  }

  /**
   * Return the sum over the half-open range [start, end). Both endpoints
   * must be integers in [0, length], and start must not exceed end;
   * violations throw RangeError. An empty range returns 0.
   */
  rangeSum(start: number, end: number): number {
    this.validateEndpoint(start, "start");
    this.validateEndpoint(end, "end");
    if (start > end) {
      throw new RangeError(`Invalid range: start (${start}) must not exceed end (${end}).`);
    }
    return this.prefixSum(end) - this.prefixSum(start);
  }

  private validateIndex(index: number): void {
    if (!Number.isInteger(index) || index < 0 || index >= this.size) {
      throw new RangeError(`Index ${index} is outside the valid range [0, ${this.size - 1}].`);
    }
  }

  private validateEndpoint(endpoint: number, name: string): void {
    if (!Number.isInteger(endpoint) || endpoint < 0 || endpoint > this.size) {
      throw new RangeError(
        `${name} (${endpoint}) is outside the valid endpoint range [0, ${this.size}].`,
      );
    }
  }
}

// Dependency-free executable tests; compile to ES2020 CommonJS and run with Node.js.
function assert(condition: unknown, message: string): asserts condition {
  if (!condition) {
    throw new Error(`Assertion failed: ${message}`);
  }
}

function assertEqual(actual: number, expected: number, message: string): void {
  assert(actual === expected, `${message}; expected ${expected}, got ${actual}`);
}

function assertThrows(action: () => void, message: string): void {
  let threw = false;
  try {
    action();
  } catch (error) {
    threw = error instanceof RangeError;
  }
  assert(threw, `${message}; expected RangeError`);
}

function runSelfTests(): void {
  const empty = new FenwickTree([]);
  assertEqual(empty.length, 0, "empty tree length");
  assertEqual(empty.prefixSum(0), 0, "empty prefix sum");
  assertEqual(empty.rangeSum(0, 0), 0, "empty range sum");
  assertThrows(() => empty.add(0, 1), "updating an empty tree");
  assertThrows(() => empty.prefixSum(1), "empty prefix endpoint past end");

  const singleton = new FenwickTree([7]);
  assertEqual(singleton.length, 1, "singleton length");
  assertEqual(singleton.prefixSum(0), 0, "singleton empty prefix");
  assertEqual(singleton.prefixSum(1), 7, "singleton complete prefix");
  assertEqual(singleton.rangeSum(0, 1), 7, "singleton full range");
  assertEqual(singleton.rangeSum(1, 1), 0, "singleton empty suffix");
  singleton.add(0, -10);
  assertEqual(singleton.prefixSum(1), -3, "singleton after point addition");

  const values = new FenwickTree([2, -1, 3, 5, 4]);
  assertEqual(values.length, 5, "multi-element tree length");
  assertEqual(values.prefixSum(0), 0, "zero-length prefix");
  assertEqual(values.prefixSum(3), 4, "end-exclusive prefix sum");
  assertEqual(values.prefixSum(5), 13, "full prefix sum");
  assertEqual(values.rangeSum(0, 5), 13, "full range sum");
  assertEqual(values.rangeSum(1, 4), 7, "middle range sum");
  assertEqual(values.rangeSum(2, 3), 3, "single-element range sum");
  assertEqual(values.rangeSum(3, 3), 0, "empty range sum");
  values.add(2, 7);
  assertEqual(values.prefixSum(3), 11, "prefix after point addition");
  assertEqual(values.rangeSum(1, 4), 14, "range after point addition");
  assertEqual(values.rangeSum(0, 5), 20, "full range after point addition");
  values.add(0, -4);
  assertEqual(values.rangeSum(0, 2), -3, "range after second point addition");

  assertThrows(() => values.add(-1, 1), "negative update index");
  assertThrows(() => values.add(5, 1), "update index past end");
  assertThrows(() => values.add(1.5, 1), "fractional update index");
  assertThrows(() => values.add(Number.NaN, 1), "NaN update index");
  assertThrows(() => values.prefixSum(-1), "negative prefix endpoint");
  assertThrows(() => values.prefixSum(6), "prefix endpoint past end");
  assertThrows(() => values.prefixSum(2.5), "fractional prefix endpoint");
  assertThrows(() => values.rangeSum(3, 2), "reversed range");
  assertThrows(() => values.rangeSum(-1, 2), "negative range endpoint");
  assertThrows(() => values.rangeSum(0, 6), "range endpoint past end");
  assertThrows(() => values.rangeSum(Number.NaN, 2), "NaN range endpoint");

  console.log("fenwick_tree.ts: all tests passed");
}

runSelfTests();
