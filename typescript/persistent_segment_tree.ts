/**
 * Persistent segment tree for immutable versions of a BigInt array.
 *
 * Ranges are half-open: query(version, left, right) sums [left, right).
 * The initial build takes O(n); each point update and range query takes
 * O(log n). After u updates, total storage is O(n + u log n): each update
 * allocates only the nodes along one root-to-leaf path and shares all others.
 *
 * Empty arrays are supported: version 0 exists, query(0, 0, 0) is 0n, and
 * point updates are invalid. Empty ranges at any valid boundary sum to 0n.
 */

interface Node {
  readonly sum: bigint;
  readonly left: Node | null;
  readonly right: Node | null;
}

function build(values: readonly bigint[], start: number, end: number): Node {
  if (end - start === 1) {
    return { sum: values[start], left: null, right: null };
  }

  const middle = start + Math.floor((end - start) / 2);
  const left = build(values, start, middle);
  const right = build(values, middle, end);
  return { sum: left.sum + right.sum, left, right };
}

function replace(
  node: Node,
  start: number,
  end: number,
  index: number,
  value: bigint,
): Node {
  if (end - start === 1) {
    return { sum: value, left: null, right: null };
  }

  const middle = start + Math.floor((end - start) / 2);
  let left = node.left;
  let right = node.right;
  if (index < middle) {
    if (left === null) throw new Error("Internal tree invariant violated");
    left = replace(left, start, middle, index, value);
  } else {
    if (right === null) throw new Error("Internal tree invariant violated");
    right = replace(right, middle, end, index, value);
  }
  return {
    sum: (left?.sum ?? 0n) + (right?.sum ?? 0n),
    left,
    right,
  };
}

function rangeSum(
  node: Node,
  start: number,
  end: number,
  queryStart: number,
  queryEnd: number,
): bigint {
  if (queryStart <= start && end <= queryEnd) return node.sum;

  const middle = start + Math.floor((end - start) / 2);
  let sum = 0n;
  if (queryStart < middle) {
    if (node.left === null) throw new Error("Internal tree invariant violated");
    sum += rangeSum(node.left, start, middle, queryStart, queryEnd);
  }
  if (queryEnd > middle) {
    if (node.right === null) throw new Error("Internal tree invariant violated");
    sum += rangeSum(node.right, middle, end, queryStart, queryEnd);
  }
  return sum;
}

/** A point-updateable collection of immutable array versions. */
export class PersistentSegmentTree {
  readonly length: number;
  private readonly roots: (Node | null)[];

  /** Construct version 0 from the supplied BigInt values. */
  constructor(values: readonly bigint[]) {
    if (!Array.isArray(values)) {
      throw new TypeError("Initial values must be an array of BigInts");
    }
    for (let i = 0; i < values.length; i += 1) {
      if (typeof values[i] !== "bigint") {
        throw new TypeError(`Initial value at index ${i} must be a BigInt`);
      }
    }
    this.length = values.length;
    this.roots = [values.length === 0 ? null : build(values, 0, values.length)];
  }

  /** Number of versions currently available, including version 0. */
  get versionCount(): number {
    return this.roots.length;
  }

  /**
   * Create and return a new version by replacing index with value in baseVersion.
   * Updates may branch from any existing version; no existing version is changed.
   */
  update(baseVersion: number, index: number, value: bigint): number {
    this.validateVersion(baseVersion);
    this.validateIndex(index);
    if (typeof value !== "bigint") {
      throw new TypeError("Updated value must be a BigInt");
    }

    const root = this.roots[baseVersion];
    if (root === null) throw new Error("Cannot update an empty array");
    const newRoot = replace(root, 0, this.length, index, value);
    this.roots.push(newRoot);
    return this.roots.length - 1;
  }

  /** Return the sum of values in [left, right) in the selected version. */
  query(version: number, left: number, right: number): bigint {
    this.validateVersion(version);
    if (!Number.isInteger(left) || !Number.isInteger(right)) {
      throw new RangeError("Range bounds must be integers");
    }
    if (left < 0 || right < left || right > this.length) {
      throw new RangeError(
        `Invalid range [${left}, ${right}) for length ${this.length}`,
      );
    }
    if (left === right) return 0n;

    const root = this.roots[version];
    if (root === null) throw new Error("Internal tree invariant violated");
    return rangeSum(root, 0, this.length, left, right);
  }

  private validateVersion(version: number): void {
    if (!Number.isInteger(version) || version < 0 || version >= this.roots.length) {
      throw new RangeError(`Invalid version: ${version}`);
    }
  }

  private validateIndex(index: number): void {
    if (!Number.isInteger(index) || index < 0 || index >= this.length) {
      throw new RangeError(`Invalid index: ${index} for length ${this.length}`);
    }
  }
}

/** Run dependency-free assertions when this file is executed directly. */
export function runSelfTests(): string[] {
  const passed: string[] = [];
  const test = (name: string, body: () => void): void => {
    body();
    passed.push(name);
  };
  const equal = (actual: bigint | number, expected: bigint | number, label: string): void => {
    if (actual !== expected) {
      throw new Error(`${label}: expected ${String(expected)}, got ${String(actual)}`);
    }
  };
  const throws = (body: () => unknown, expected: "RangeError" | "TypeError", label: string): void => {
    try {
      body();
    } catch (error) {
      if (error instanceof Error && error.name === expected) return;
      throw new Error(`${label}: expected ${expected}, got ${String(error)}`);
    }
    throw new Error(`${label}: expected ${expected} to be thrown`);
  };

  test("initial build and half-open range sums", () => {
    const tree = new PersistentSegmentTree([2n, -3n, 7n, 11n]);
    equal(tree.versionCount, 1, "initial version count");
    equal(tree.query(0, 0, 4), 17n, "whole-array sum");
    equal(tree.query(0, 1, 3), 4n, "interior sum");
    equal(tree.query(0, 0, 2), -1n, "left boundary sum");
    equal(tree.query(0, 2, 4), 18n, "right boundary sum");
  });

  test("empty array and empty ranges", () => {
    const empty = new PersistentSegmentTree([]);
    equal(empty.length, 0, "empty length");
    equal(empty.versionCount, 1, "empty initial version count");
    equal(empty.query(0, 0, 0), 0n, "empty array query");
    const tree = new PersistentSegmentTree([5n, 8n, 13n]);
    equal(tree.query(0, 0, 0), 0n, "empty range at left boundary");
    equal(tree.query(0, 2, 2), 0n, "empty range inside array");
    equal(tree.query(0, 3, 3), 0n, "empty range at right boundary");
  });

  test("singleton and boundary point updates", () => {
    const singleton = new PersistentSegmentTree([42n]);
    const updated = singleton.update(0, 0, -9n);
    equal(singleton.query(0, 0, 1), 42n, "singleton original");
    equal(singleton.query(updated, 0, 1), -9n, "singleton updated");
    const boundaries = new PersistentSegmentTree([1n, 2n, 3n]);
    const first = boundaries.update(0, 0, 10n);
    const last = boundaries.update(first, 2, 30n);
    equal(boundaries.query(last, 0, 3), 42n, "updated boundary values");
    equal(boundaries.query(0, 0, 3), 6n, "original boundary values");
  });

  test("old versions remain unchanged", () => {
    const tree = new PersistentSegmentTree([1n, 2n, 3n, 4n]);
    const next = tree.update(0, 1, 20n);
    equal(tree.query(0, 0, 4), 10n, "version 0 unchanged");
    equal(tree.query(next, 0, 4), 28n, "updated version sum");
    equal(tree.query(0, 1, 2), 2n, "version 0 point unchanged");
    equal(tree.query(next, 1, 2), 20n, "new version point value");
  });

  test("updates branch from any previous version", () => {
    const tree = new PersistentSegmentTree([1n, 2n, 3n]);
    const firstBranch = tree.update(0, 0, 10n);
    const secondBranch = tree.update(0, 2, 30n);
    const descendant = tree.update(firstBranch, 1, 20n);
    equal(firstBranch, 1, "first branch id");
    equal(secondBranch, 2, "second branch id");
    equal(descendant, 3, "descendant id");
    equal(tree.query(0, 0, 3), 6n, "root version sum");
    equal(tree.query(firstBranch, 0, 3), 15n, "first branch sum");
    equal(tree.query(secondBranch, 0, 3), 33n, "second branch sum");
    equal(tree.query(descendant, 0, 3), 33n, "descendant sum");
    equal(tree.query(secondBranch, 0, 1), 1n, "sibling did not inherit branch update");
  });

  test("invalid versions are rejected", () => {
    const tree = new PersistentSegmentTree([1n]);
    throws(() => tree.query(-1, 0, 0), "RangeError", "negative query version");
    throws(() => tree.query(1, 0, 0), "RangeError", "future query version");
    throws(() => tree.query(0.5, 0, 0), "RangeError", "fractional query version");
    throws(() => tree.update(1, 0, 2n), "RangeError", "future update version");
  });

  test("invalid indices are rejected", () => {
    const tree = new PersistentSegmentTree([1n, 2n]);
    throws(() => tree.update(-1, 0, 3n), "RangeError", "negative update index");
    throws(() => tree.update(0, 2, 3n), "RangeError", "out-of-bounds update index");
    throws(() => tree.update(0, 0.5, 3n), "RangeError", "fractional update index");
    throws(() => new PersistentSegmentTree([]).update(0, 0, 3n), "RangeError", "empty array update");
  });

  test("invalid ranges are rejected", () => {
    const tree = new PersistentSegmentTree([1n, 2n, 3n]);
    throws(() => tree.query(0, -1, 1), "RangeError", "negative range start");
    throws(() => tree.query(0, 0, 4), "RangeError", "range end past length");
    throws(() => tree.query(0, 2, 1), "RangeError", "reversed range");
    throws(() => tree.query(0, 0.5, 1), "RangeError", "fractional range start");
    throws(() => tree.query(0, 0, Number.NaN), "RangeError", "NaN range end");
  });

  test("exact sums beyond Number precision", () => {
    const a = 9_007_199_254_740_993n;
    const b = 123_456_789_012_345_678_901_234_567_890n;
    const tree = new PersistentSegmentTree([a, b, -a]);
    equal(tree.query(0, 0, 3), b, "large exact whole sum");
    equal(tree.query(0, 0, 2), a + b, "large exact prefix sum");
    const changed = tree.update(0, 2, a * 2n);
    equal(tree.query(changed, 0, 3), b + 3n * a, "large exact updated sum");
    equal(tree.query(0, 0, 3), b, "large original sum remains exact");
  });

  return passed;
}

// Avoid Node type dependencies while allowing compiled JS or ts-node direct runs.
const runtime = globalThis as unknown as {
  process?: { argv?: string[] };
};
const entryPoint = runtime.process?.argv?.[1];
if (entryPoint && /(?:^|[\\/])persistent_segment_tree\.(?:js|ts)$/.test(entryPoint)) {
  const passed = runSelfTests();
  console.log(`All ${passed.length} persistent segment tree self-tests passed:`);
  for (const name of passed) console.log(`- ${name}`);
}
