/**
 * Lazy segment tree supporting range addition and range-sum queries on a
 * zero-based numeric array. All ranges are half-open: [left, right).
 * Each update and query takes O(log n) time; storage is O(n).
 *
 * Empty ranges are valid: adding to one is a no-op and its sum is zero.
 * The empty input is supported; any non-empty range on it fails validation.
 */
export class LazySegmentTree {
  private readonly size: number;
  private readonly sums: number[];
  private readonly lazy: number[];

  constructor(values: readonly number[]) {
    this.size = values.length;
    const capacity = Math.max(1, 4 * this.size);
    this.sums = new Array<number>(capacity).fill(0);
    this.lazy = new Array<number>(capacity).fill(0);

    if (this.size > 0) {
      this.build(1, 0, this.size, values);
    }
  }

  /** Number of elements represented by this tree. */
  get length(): number {
    return this.size;
  }

  /** Add `delta` to every element in the half-open range [left, right). */
  rangeAdd(left: number, right: number, delta: number): void {
    this.validateRange(left, right);
    if (left === right) {
      return;
    }
    this.add(1, 0, this.size, left, right, delta);
  }

  /** Return the sum of elements in the half-open range [left, right). */
  rangeSum(left: number, right: number): number {
    this.validateRange(left, right);
    if (left === right) {
      return 0;
    }
    return this.sum(1, 0, this.size, left, right);
  }

  private build(node: number, start: number, end: number, values: readonly number[]): void {
    if (end - start === 1) {
      this.sums[node] = values[start];
      return;
    }

    const middle = Math.floor((start + end) / 2);
    this.build(node * 2, start, middle, values);
    this.build(node * 2 + 1, middle, end, values);
    this.pull(node);
  }

  private add(
    node: number,
    start: number,
    end: number,
    queryStart: number,
    queryEnd: number,
    delta: number,
  ): void {
    if (queryStart <= start && end <= queryEnd) {
      this.apply(node, start, end, delta);
      return;
    }

    this.push(node, start, end);
    const middle = Math.floor((start + end) / 2);
    if (queryStart < middle) {
      this.add(node * 2, start, middle, queryStart, queryEnd, delta);
    }
    if (queryEnd > middle) {
      this.add(node * 2 + 1, middle, end, queryStart, queryEnd, delta);
    }
    this.pull(node);
  }

  private sum(
    node: number,
    start: number,
    end: number,
    queryStart: number,
    queryEnd: number,
  ): number {
    if (queryStart <= start && end <= queryEnd) {
      return this.sums[node];
    }

    this.push(node, start, end);
    const middle = Math.floor((start + end) / 2);
    let result = 0;
    if (queryStart < middle) {
      result += this.sum(node * 2, start, middle, queryStart, queryEnd);
    }
    if (queryEnd > middle) {
      result += this.sum(node * 2 + 1, middle, end, queryStart, queryEnd);
    }
    return result;
  }

  private apply(node: number, start: number, end: number, delta: number): void {
    this.sums[node] += (end - start) * delta;
    this.lazy[node] += delta;
  }

  private push(node: number, start: number, end: number): void {
    const delta = this.lazy[node];
    if (delta === 0 || end - start === 1) {
      return;
    }

    const middle = Math.floor((start + end) / 2);
    this.apply(node * 2, start, middle, delta);
    this.apply(node * 2 + 1, middle, end, delta);
    this.lazy[node] = 0;
  }

  private pull(node: number): void {
    this.sums[node] = this.sums[node * 2] + this.sums[node * 2 + 1];
  }

  private validateRange(left: number, right: number): void {
    if (!Number.isInteger(left) || !Number.isInteger(right)) {
      throw new RangeError(`Range bounds must be integers; received [${left}, ${right}).`);
    }
    if (left < 0 || right > this.size || left > right) {
      throw new RangeError(
        `Invalid range [${left}, ${right}); expected 0 <= left <= right <= ${this.size}.`,
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

function runTests(): void {
  const empty = new LazySegmentTree([]);
  assertEqual(empty.length, 0, "empty tree length");
  assertEqual(empty.rangeSum(0, 0), 0, "empty range on empty tree");
  empty.rangeAdd(0, 0, 10);
  assertThrows(() => empty.rangeSum(0, 1), "non-empty sum on empty tree");
  assertThrows(() => empty.rangeAdd(0, 1, 1), "non-empty update on empty tree");

  const singleton = new LazySegmentTree([-4]);
  assertEqual(singleton.rangeSum(0, 1), -4, "singleton initial sum");
  singleton.rangeAdd(0, 1, 7);
  assertEqual(singleton.rangeSum(0, 1), 3, "singleton after range addition");
  assertEqual(singleton.rangeSum(1, 1), 0, "empty suffix range");
  singleton.rangeAdd(1, 1, -100);
  assertEqual(singleton.rangeSum(0, 1), 3, "empty update is a no-op");

  const values = new LazySegmentTree([2, -1, 3, 5, -4, 6]);
  assertEqual(values.length, 6, "multi-element tree length");
  assertEqual(values.rangeSum(0, 6), 11, "initial full range sum");
  assertEqual(values.rangeSum(1, 4), 7, "initial partial sum");
  assertEqual(values.rangeSum(2, 3), 3, "single-element partial sum");
  assertEqual(values.rangeSum(0, 0), 0, "empty prefix range");

  values.rangeAdd(1, 5, 3);
  assertEqual(values.rangeSum(0, 6), 23, "full sum after partial update");
  assertEqual(values.rangeSum(1, 5), 15, "updated range sum");
  assertEqual(values.rangeSum(2, 4), 14, "updated interior partial sum");
  values.rangeAdd(3, 6, -2);
  assertEqual(values.rangeSum(0, 6), 17, "sum after overlapping update");
  assertEqual(values.rangeSum(1, 4), 14, "partial sum after overlapping updates");
  values.rangeAdd(0, 6, -1);
  assertEqual(values.rangeSum(0, 6), 11, "sum after full-range update");
  assertEqual(values.rangeSum(4, 6), -1, "suffix sum after full-range update");

  assertThrows(() => values.rangeSum(-1, 0), "negative left bound");
  assertThrows(() => values.rangeSum(0, 7), "right bound past end");
  assertThrows(() => values.rangeSum(4, 3), "reversed range");
  assertThrows(() => values.rangeSum(0.5, 2), "fractional left bound");
  assertThrows(() => values.rangeAdd(0, 1.5, 2), "fractional right bound");
  assertThrows(() => values.rangeAdd(-1, 2, 2), "negative update bound");
  assertThrows(() => values.rangeAdd(2, 7, 2), "update bound past end");

  console.log("lazy_segment_tree.ts: all tests passed");
}

runTests();
