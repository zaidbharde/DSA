'use strict';

/**
 * Disjoint-set union (union-find) over the integer elements `0..len`.
 *
 * `find`, `union`, and `connected` use path compression and union by size,
 * giving amortized O(alpha(n)) time per operation, where alpha is the inverse
 * Ackermann function. Construction takes O(n) time; the structure uses O(n)
 * space. Invalid indices return `null` instead of throwing.
 */
class DisjointSet {
  /** Create `length` singleton components. */
  constructor(length) {
    if (!Number.isSafeInteger(length) || length < 0) {
      throw new RangeError('length must be a nonnegative safe integer');
    }

    this._parent = Array.from({ length }, (_, index) => index);
    this._size = new Array(length).fill(1);
    this._components = length;
  }

  /** Return the number of elements. */
  len() {
    return this._parent.length;
  }

  /** Return whether the structure contains no elements. */
  isEmpty() {
    return this._parent.length === 0;
  }

  /** Return the current number of connected components in O(1) time. */
  componentCount() {
    return this._components;
  }

  /**
   * Find an element's representative and compress its path, or return `null`
   * when the index is outside `0..len`.
   */
  find(element) {
    if (!this._isValidIndex(element)) return null;

    let root = element;
    while (this._parent[root] !== root) {
      root = this._parent[root];
    }

    let current = element;
    while (this._parent[current] !== current) {
      const next = this._parent[current];
      this._parent[current] = root;
      current = next;
    }

    return root;
  }

  /**
   * Merge the components containing `a` and `b`.
   * Return `true` if merged, `false` if already connected, or `null` if either
   * index is invalid.
   */
  union(a, b) {
    if (!this._isValidIndex(a) || !this._isValidIndex(b)) return null;

    let rootA = this.find(a);
    let rootB = this.find(b);
    if (rootA === rootB) return false;

    if (this._size[rootA] < this._size[rootB]) {
      [rootA, rootB] = [rootB, rootA];
    }
    this._parent[rootB] = rootA;
    this._size[rootA] += this._size[rootB];
    this._components -= 1;
    return true;
  }

  /** Return whether `a` and `b` share a component, or `null` for invalid indices. */
  connected(a, b) {
    if (!this._isValidIndex(a) || !this._isValidIndex(b)) return null;
    return this.find(a) === this.find(b);
  }

  _isValidIndex(index) {
    return Number.isSafeInteger(index) && index >= 0 && index < this._parent.length;
  }
}

module.exports = DisjointSet;
module.exports.DisjointSet = DisjointSet;

// Directly runnable tests: node javascript/disjoint_set.js
if (require.main === module) {
  const assert = require('node:assert/strict');
  const { test } = require('node:test');

  test('supports empty and singleton structures', () => {
    const empty = new DisjointSet(0);
    assert.equal(empty.len(), 0);
    assert.equal(empty.isEmpty(), true);
    assert.equal(empty.componentCount(), 0);
    assert.equal(empty.find(0), null);

    const singleton = new DisjointSet(1);
    assert.equal(singleton.len(), 1);
    assert.equal(singleton.isEmpty(), false);
    assert.equal(singleton.componentCount(), 1);
    assert.equal(singleton.find(0), 0);
    assert.equal(singleton.connected(0, 0), true);
    assert.equal(singleton.union(0, 0), false);
  });

  test('unions components and tracks their count', () => {
    const sets = new DisjointSet(6);
    assert.equal(sets.componentCount(), 6);
    assert.equal(sets.union(0, 1), true);
    assert.equal(sets.union(1, 2), true);
    assert.equal(sets.union(3, 4), true);
    assert.equal(sets.componentCount(), 3);
    assert.equal(sets.union(2, 4), true);
    assert.equal(sets.componentCount(), 2);
    assert.equal(sets.connected(0, 4), true);
    assert.equal(sets.connected(0, 5), false);
    assert.equal(sets.find(0), sets.find(4));
  });

  test('redundant unions do not alter component count', () => {
    const sets = new DisjointSet(3);
    assert.equal(sets.union(0, 1), true);
    assert.equal(sets.union(1, 0), false);
    assert.equal(sets.union(0, 0), false);
    assert.equal(sets.componentCount(), 2);
  });

  test('find compresses paths', () => {
    const sets = new DisjointSet(4);
    sets.union(0, 1);
    sets.union(2, 3);
    sets.union(0, 2);
    assert.equal(sets._parent[3], 2);
    assert.equal(sets.find(3), 0);
    assert.equal(sets._parent[3], 0);
  });

  test('invalid indices return null without changing component count', () => {
    const sets = new DisjointSet(2);
    assert.equal(sets.find(2), null);
    assert.equal(sets.find(-1), null);
    assert.equal(sets.union(0, 2), null);
    assert.equal(sets.union(3, 0), null);
    assert.equal(sets.connected(1, 2), null);
    assert.equal(sets.connected(Number.MAX_SAFE_INTEGER, 0), null);
    assert.equal(sets.connected(0.5, 0), null);
    assert.equal(sets.componentCount(), 2);

    const empty = new DisjointSet(0);
    assert.equal(empty.find(0), null);
    assert.equal(empty.union(0, 0), null);
    assert.equal(empty.connected(0, 0), null);
  });

  test('rejects invalid lengths', () => {
    for (const length of [-1, 1.5, NaN, Infinity, Number.MAX_SAFE_INTEGER + 1]) {
      assert.throws(() => new DisjointSet(length), RangeError);
    }
  });
}
