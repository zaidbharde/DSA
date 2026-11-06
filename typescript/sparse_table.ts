/**
 * Immutable sparse table for range minimum queries over numbers.
 *
 * Preprocessing and storage take O(n log n) time and space. Each query takes
 * O(1) time and uses zero-based half-open bounds [start, end).
 */
export class SparseTable {
  private readonly values: readonly number[];
  private readonly levels: readonly (readonly number[])[];
  private readonly length: number;

  public constructor(values: readonly number[]) {
    this.values = Object.freeze([...values]);
    this.length = this.values.length;

    const levels: number[][] = [];
    if (this.length > 0) {
      levels.push([...this.values]);
      for (let span = 2; span <= this.length; span *= 2) {
        const previous = levels[levels.length - 1];
        const halfSpan = span / 2;
        const current: number[] = [];
        for (let start = 0; start + span <= this.length; start += 1) {
          current.push(Math.min(previous[start], previous[start + halfSpan]));
        }
        levels.push(current);
      }
    }

    this.levels = Object.freeze(levels.map((level) => Object.freeze(level)));
    Object.freeze(this);
  }

  /** Return the minimum value in the nonempty range [start, end). */
  public query(start: number, end: number): number {
    if (!Number.isInteger(start) || !Number.isInteger(end)) {
      throw new RangeError("Query endpoints must be integers.");
    }
    if (start < 0 || end > this.length || start > end) {
      throw new RangeError(`Query range [${start}, ${end}) is outside [0, ${this.length}).`);
    }
    if (start === end) {
      throw new RangeError("Query range must be nonempty (start must be less than end).");
    }

    const rangeLength = end - start;
    const level = Math.floor(Math.log2(rangeLength));
    const span = 2 ** level;
    const row = this.levels[level];
    return Math.min(row[start], row[end - span]);
  }
}

// Minimal local assertions keep this file executable without external test dependencies.
function assertEqual(actual: number, expected: number, message: string): void {
  if (!Object.is(actual, expected)) {
    throw new Error(`${message}: expected ${expected}, got ${actual}`);
  }
}

function assertThrows(action: () => unknown, message: string): void {
  let didThrow = false;
  try {
    action();
  } catch (error: unknown) {
    if (!(error instanceof RangeError)) {
      throw new Error(`${message}: expected RangeError, got ${String(error)}`);
    }
    didThrow = true;
  }
  if (!didThrow) {
    throw new Error(`${message}: expected RangeError, but no error was thrown`);
  }
}

function runTests(): void {
  const empty = new SparseTable([]);
  assertThrows(() => empty.query(0, 1), "empty table query");

  const singleton = new SparseTable([7]);
  assertEqual(singleton.query(0, 1), 7, "singleton range");

  const negatives = new SparseTable([-8, -3, -11, -2]);
  assertEqual(negatives.query(0, 4), -11, "negative values across whole range");
  assertEqual(negatives.query(1, 3), -11, "negative values in subrange");

  const duplicates = new SparseTable([4, 1, 1, 6, 1]);
  assertEqual(duplicates.query(1, 4), 1, "duplicate minima");
  assertEqual(duplicates.query(3, 5), 1, "duplicate minimum at range end");

  const multipleRanges = new SparseTable([9, 5, 8, 2, 7, 3, 6]);
  assertEqual(multipleRanges.query(0, 1), 9, "first element");
  assertEqual(multipleRanges.query(0, 7), 2, "whole range");
  assertEqual(multipleRanges.query(2, 6), 2, "interior range");
  assertEqual(multipleRanges.query(4, 7), 3, "suffix range");

  assertThrows(() => multipleRanges.query(-1, 2), "negative start");
  assertThrows(() => multipleRanges.query(0, 8), "end beyond array length");
  assertThrows(() => multipleRanges.query(4, 3), "reversed range");
  assertThrows(() => multipleRanges.query(2, 2), "empty range");
  assertThrows(() => multipleRanges.query(0.5, 2), "fractional endpoint");
  assertThrows(() => multipleRanges.query(0, Number.NaN), "NaN endpoint");

  // The public API deliberately has no mutation method.
  if ("update" in multipleRanges) {
    throw new Error("SparseTable must not expose an update method");
  }
  if (false) {
    // @ts-expect-error Updates are not supported by the immutable API.
    multipleRanges.update(0, 100);
  }
}

runTests();
console.log("SparseTable tests passed.");
