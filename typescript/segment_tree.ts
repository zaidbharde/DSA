/**
 * Iterative segment tree for point assignment and inclusive range-sum queries.
 * Storage is a flat 2n array, giving O(n) build time and storage.
 */
export class SumSegmentTree {
  private readonly size: number;
  private readonly tree: number[];

  constructor(values: readonly number[]) {
    this.size = values.length;
    this.tree = new Array<number>(2 * this.size).fill(0);

    for (let index = 0; index < this.size; index += 1) {
      this.tree[this.size + index] = values[index];
    }
    for (let index = this.size - 1; index > 0; index -= 1) {
      this.tree[index] = this.tree[index * 2] + this.tree[index * 2 + 1];
    }
  }

  /** Number of elements represented by this tree. */
  get length(): number {
    return this.size;
  }

  /** Assign a value at a zero-based index. */
  update(index: number, value: number): void {
    this.validateIndex(index);

    let node = this.size + index;
    this.tree[node] = value;
    while (node > 1) {
      node = Math.floor(node / 2);
      this.tree[node] = this.tree[node * 2] + this.tree[node * 2 + 1];
    }
  }

  /** Return the sum over the inclusive, zero-based range [left, right]. */
  query(left: number, right: number): number {
    this.validateIndex(left);
    this.validateIndex(right);
    if (left > right) {
      throw new RangeError(`Invalid range: left (${left}) must not exceed right (${right}).`);
    }

    let start = left + this.size;
    let end = right + this.size + 1;
    let sum = 0;

    while (start < end) {
      if (start % 2 === 1) {
        sum += this.tree[start];
        start += 1;
      }
      if (end % 2 === 1) {
        end -= 1;
        sum += this.tree[end];
      }
      start = Math.floor(start / 2);
      end = Math.floor(end / 2);
    }

    return sum;
  }

  private validateIndex(index: number): void {
    if (!Number.isInteger(index) || index < 0 || index >= this.size) {
      throw new RangeError(`Index ${index} is outside the valid range [0, ${this.size - 1}].`);
    }
  }
}

// Dependency-free executable tests; run the emitted JavaScript with Node.js.
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
  const empty = new SumSegmentTree([]);
  assertEqual(empty.length, 0, "empty tree length");
  assertThrows(() => empty.query(0, 0), "querying an empty tree");
  assertThrows(() => empty.update(0, 1), "updating an empty tree");

  const singleton = new SumSegmentTree([7]);
  assertEqual(singleton.query(0, 0), 7, "singleton range sum");
  singleton.update(0, -3);
  assertEqual(singleton.query(0, 0), -3, "singleton after assignment");
  assertThrows(() => singleton.query(-1, 0), "negative query index");
  assertThrows(() => singleton.query(0, 1), "query index past end");
  assertThrows(() => singleton.query(0.5, 0.5), "fractional query index");
  assertThrows(() => singleton.query(0, 0 - 1), "reversed range");
  assertThrows(() => singleton.update(1, 9), "update index past end");
  assertThrows(() => singleton.update(Number.NaN, 9), "non-integer update index");

  const values = new SumSegmentTree([2, -1, 3, 5, 4]);
  assertEqual(values.length, 5, "multi-element tree length");
  assertEqual(values.query(0, 4), 13, "full range sum");
  assertEqual(values.query(1, 3), 7, "middle range sum");
  assertEqual(values.query(2, 2), 3, "single-element subrange sum");
  assertEqual(values.query(0, 1), 1, "prefix range sum");
  assertEqual(values.query(3, 4), 9, "suffix range sum");

  values.update(2, 10);
  assertEqual(values.query(0, 4), 20, "full sum after point assignment");
  assertEqual(values.query(1, 3), 14, "range sum after point assignment");
  values.update(0, -2);
  assertEqual(values.query(0, 2), 7, "range sum after second assignment");
  assertThrows(() => values.query(3, 2), "reversed multi-element range");
  assertThrows(() => values.query(0, 5), "range endpoint past end");

  console.log("segment_tree.ts: all tests passed");
}

runTests();
