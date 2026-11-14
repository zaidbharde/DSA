/**
 * Rollback Disjoint Set Union (Union-Find), using union by size and no path
 * compression so every successful union can be undone exactly.
 *
 * Indices are zero-based in [0, length). find, union, connected, and
 * componentSize take O(log n) worst-case time for n elements; union by size
 * keeps tree height logarithmic. snapshot takes O(1). rollback takes O(k),
 * where k is the number of successful unions undone (plus O(k) to validate
 * that the token belongs to the current history branch). The current history
 * stores O(u) records for u successful unions since the last rollback; no-op
 * unions add no records. The DSU arrays use O(n) space. Snapshot tokens are
 * opaque, O(1)-sized values and are not retained internally.
 */

interface HistoryEntry {
  readonly childRoot: number;
  readonly parentRoot: number;
  readonly previousParentSize: number;
  readonly previous: HistoryEntry | null;
}

const snapshotBrand: unique symbol = Symbol("RollbackSnapshot");

/** An opaque checkpoint produced by a particular RollbackDisjointSetUnion. */
export interface SnapshotToken {
  readonly [snapshotBrand]: {
    readonly owner: object;
    readonly state: HistoryEntry | null;
  };
}

/** Disjoint-set structure that can restore prior states using snapshot tokens. */
export class RollbackDisjointSetUnion {
  private readonly parent: number[];
  private readonly sizes: number[];
  private readonly identity: object = {};
  private history: HistoryEntry | null = null;
  private components: number;

  constructor(size: number) {
    if (!Number.isSafeInteger(size) || size < 0) {
      throw new RangeError("Size must be a non-negative safe integer");
    }
    this.parent = Array.from({ length: size }, (_, index) => index);
    this.sizes = new Array<number>(size).fill(1);
    this.components = size;
  }

  /** Number of elements in the structure. */
  get length(): number {
    return this.parent.length;
  }

  /** Number of connected components currently represented. */
  get componentCount(): number {
    return this.components;
  }

  /** Find an element's representative without modifying parent pointers. */
  find(index: number): number {
    this.validateIndex(index);
    let root = index;
    while (this.parent[root] !== root) {
      root = this.parent[root];
    }
    return root;
  }

  /**
   * Merge the components containing a and b. Return true when they were
   * distinct; a redundant union returns false and creates no history entry.
   */
  union(a: number, b: number): boolean {
    let rootA = this.find(a);
    let rootB = this.find(b);
    if (rootA === rootB) return false;

    if (this.sizes[rootA] < this.sizes[rootB]) {
      [rootA, rootB] = [rootB, rootA];
    }

    this.history = {
      childRoot: rootB,
      parentRoot: rootA,
      previousParentSize: this.sizes[rootA],
      previous: this.history,
    };
    this.parent[rootB] = rootA;
    this.sizes[rootA] += this.sizes[rootB];
    this.components -= 1;
    return true;
  }

  /** Whether two valid element indices currently belong to the same set. */
  connected(a: number, b: number): boolean {
    return this.find(a) === this.find(b);
  }

  /** Return the size of the component containing a valid element index. */
  componentSize(index: number): number {
    return this.sizes[this.find(index)];
  }

  /** Capture the current history state in an opaque, O(1) snapshot token. */
  snapshot(): SnapshotToken {
    const metadata = Object.freeze({ owner: this.identity, state: this.history });
    return Object.freeze({ [snapshotBrand]: metadata });
  }

  /**
   * Restore the exact state represented by a snapshot from this DSU's current
   * history branch. Repeating rollback to the same token is safe. Tokens from
   * another DSU, malformed tokens, and tokens on discarded branches throw
   * RangeError without changing the current state.
   */
  rollback(token: SnapshotToken): void {
    if (typeof token !== "object" || token === null) {
      throw new RangeError("Invalid rollback snapshot token");
    }
    const metadata = (token as SnapshotToken)[snapshotBrand];
    if (
      metadata === undefined
      || metadata.owner !== this.identity
    ) {
      throw new RangeError("Snapshot token does not belong to this DSU");
    }

    // Check reachability before mutating, so an invalid/stale token is atomic.
    let cursor = this.history;
    while (cursor !== metadata.state && cursor !== null) {
      cursor = cursor.previous;
    }
    if (cursor !== metadata.state) {
      throw new RangeError("Snapshot token is not on the current history branch");
    }

    while (this.history !== metadata.state) {
      const entry = this.history;
      if (entry === null) throw new Error("Internal history invariant violated");
      this.parent[entry.childRoot] = entry.childRoot;
      this.sizes[entry.parentRoot] = entry.previousParentSize;
      this.components += 1;
      this.history = entry.previous;
    }
  }

  private validateIndex(index: number): void {
    if (!Number.isInteger(index) || index < 0 || index >= this.parent.length) {
      throw new RangeError(`Invalid element index: ${index}`);
    }
  }
}

/** Dependency-free executable tests; the default runner compiles and runs this file. */
function assert(condition: unknown, message: string): asserts condition {
  if (!condition) throw new Error(`Assertion failed: ${message}`);
}

function assertEqual(actual: number | boolean, expected: number | boolean, message: string): void {
  assert(actual === expected, `${message}; expected ${expected}, got ${actual}`);
}

function assertThrows(action: () => unknown, message: string): void {
  let threw = false;
  try {
    action();
  } catch (error) {
    threw = error instanceof RangeError;
  }
  assert(threw, `${message}; expected RangeError`);
}

function runSelfTests(): void {
  const empty = new RollbackDisjointSetUnion(0);
  assertEqual(empty.length, 0, "empty length");
  assertEqual(empty.componentCount, 0, "empty component count");
  const emptySnapshot = empty.snapshot();
  empty.rollback(emptySnapshot);
  assertThrows(() => empty.find(0), "empty find index");

  const dsu = new RollbackDisjointSetUnion(6);
  const initial = dsu.snapshot();
  assertEqual(dsu.componentCount, 6, "initial component count");
  assertEqual(dsu.union(0, 1), true, "first merge succeeds");
  const afterFirst = dsu.snapshot();
  assertEqual(dsu.union(1, 2), true, "second merge succeeds");
  assertEqual(dsu.union(3, 4), true, "independent merge succeeds");
  const nested = dsu.snapshot();
  assertEqual(dsu.union(2, 4), true, "merge joins larger components");
  assertEqual(dsu.connected(0, 4), true, "merged components are connected");
  assertEqual(dsu.componentSize(3), 5, "merged component size");
  assertEqual(dsu.componentCount, 2, "component count after merges");

  dsu.rollback(nested);
  assertEqual(dsu.connected(0, 2), true, "rollback retains earlier connection");
  assertEqual(dsu.connected(0, 3), false, "rollback removes latest connection");
  assertEqual(dsu.componentSize(1), 3, "rollback restores earlier component size");
  assertEqual(dsu.componentCount, 3, "rollback restores component count");
  dsu.rollback(nested);
  assertEqual(dsu.componentCount, 3, "repeated rollback is idempotent");

  assertEqual(dsu.union(0, 2), false, "redundant union reports no merge");
  const afterNoOp = dsu.snapshot();
  dsu.rollback(afterNoOp);
  assertEqual(dsu.componentSize(4), 2, "no-op union changes no component size");
  dsu.rollback(afterFirst);
  assertEqual(dsu.connected(0, 1), true, "rollback to middle snapshot preserves its merge");
  assertEqual(dsu.connected(0, 2), false, "rollback to middle snapshot removes later merge");
  assertEqual(dsu.componentCount, 5, "middle snapshot component count");

  // Same history depth as the discarded nested token, but on a different branch.
  assertEqual(dsu.union(0, 5), true, "new branch first merge");
  assertEqual(dsu.union(3, 4), true, "new branch second merge");
  assertEqual(dsu.union(0, 3), true, "new branch reaches old token depth");
  assertThrows(() => dsu.rollback(nested), "snapshot from discarded branch");
  assertEqual(dsu.connected(0, 4), true, "invalid rollback leaves state unchanged");
  assertEqual(dsu.componentSize(5), 5, "invalid rollback preserves component size");

  dsu.rollback(initial);
  assertEqual(dsu.componentCount, 6, "rollback to initial state restores all components");
  assertEqual(dsu.connected(0, 1), false, "initial rollback clears all connections");
  dsu.rollback(initial);
  assertEqual(dsu.componentCount, 6, "repeated initial rollback is safe");

  const other = new RollbackDisjointSetUnion(6);
  assertThrows(() => other.rollback(initial), "snapshot from another DSU");
  assertThrows(() => dsu.rollback({} as SnapshotToken), "malformed snapshot token");
  assertThrows(() => dsu.rollback(123 as unknown as SnapshotToken), "non-object snapshot token");

  assertThrows(() => new RollbackDisjointSetUnion(-1), "negative DSU size");
  assertThrows(() => new RollbackDisjointSetUnion(1.5), "fractional DSU size");
  assertThrows(() => new RollbackDisjointSetUnion(Number.NaN), "NaN DSU size");
  assertThrows(() => dsu.find(-1), "negative find index");
  assertThrows(() => dsu.find(6), "find index past end");
  assertThrows(() => dsu.find(1.5), "fractional find index");
  assertThrows(() => dsu.find(Number.NaN), "NaN find index");
  assertThrows(() => dsu.union(0, 6), "union index past end");
  assertThrows(() => dsu.connected(-1, 0), "connected with invalid index");
  assertThrows(() => dsu.componentSize(6), "component size with invalid index");

  console.log("rollback_dsu.ts: all tests passed");
}

runSelfTests();
