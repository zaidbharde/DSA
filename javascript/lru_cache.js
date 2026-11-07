'use strict';

class Node {
  constructor(key = undefined, value = undefined) {
    this.key = key;
    this.value = value;
    this.prev = null;
    this.next = null;
  }
}

/**
 * Least-recently-used cache with expected O(1) get/put operations and
 * O(capacity) memory usage (one map entry and list node per cached key).
 * A successful get or put makes the key the most recently used entry.
 */
class LRUCache {
  constructor(capacity) {
    if (!Number.isSafeInteger(capacity) || capacity < 0) {
      throw new RangeError('capacity must be a nonnegative safe integer');
    }

    this.capacity = capacity;
    this.entries = new Map();
    this.head = new Node(); // Least-recently-used side (sentinel)
    this.tail = new Node(); // Most-recently-used side (sentinel)
    this.head.next = this.tail;
    this.tail.prev = this.head;
  }

  /** Return the value for key and mark it most-recently used, or -1 on miss. */
  get(key) {
    const node = this.entries.get(key);
    if (node === undefined) return -1;

    this._moveToMostRecent(node);
    return node.value;
  }

  /** Insert or update key. If full, evict the least-recently used entry. */
  put(key, value) {
    if (this.capacity === 0) return;

    const existing = this.entries.get(key);
    if (existing !== undefined) {
      existing.value = value;
      this._moveToMostRecent(existing);
      return;
    }

    // Evict before inserting so the map never transiently exceeds capacity.
    if (this.entries.size === this.capacity) {
      const leastRecent = this.head.next;
      this._unlink(leastRecent);
      this.entries.delete(leastRecent.key);
    }

    // Match Map's SameValueZero key handling, including canonicalizing -0.
    const node = new Node(key === 0 ? 0 : key, value);
    this.entries.set(key, node);
    this._insertBeforeTail(node);
  }

  _moveToMostRecent(node) {
    if (node.next === this.tail) return;
    this._unlink(node);
    this._insertBeforeTail(node);
  }

  _unlink(node) {
    node.prev.next = node.next;
    node.next.prev = node.prev;
  }

  _insertBeforeTail(node) {
    node.prev = this.tail.prev;
    node.next = this.tail;
    this.tail.prev.next = node;
    this.tail.prev = node;
  }
}

module.exports = LRUCache;

if (require.main === module) {
  const assert = require('node:assert/strict');

  // Recency promotion on get controls the eviction order.
  const recency = new LRUCache(2);
  recency.put('a', 1);
  recency.put('b', 2);
  assert.equal(recency.get('a'), 1);
  recency.put('c', 3);
  assert.equal(recency.get('b'), -1);
  assert.equal(recency.get('a'), 1);
  assert.equal(recency.get('c'), 3);

  // Updating an existing key changes its value and makes it most recent.
  const updates = new LRUCache(2);
  updates.put(1, 'old');
  updates.put(2, 'two');
  updates.put(1, 'new');
  updates.put(3, 'three');
  assert.equal(updates.get(1), 'new');
  assert.equal(updates.get(2), -1);
  assert.equal(updates.get(3), 'three');

  // Capacity one retains only the newest entry; capacity zero retains none.
  const one = new LRUCache(1);
  one.put('first', 1);
  one.put('second', 2);
  assert.equal(one.get('first'), -1);
  assert.equal(one.get('second'), 2);

  const zero = new LRUCache(0);
  zero.put('discarded', 9);
  assert.equal(zero.get('discarded'), -1);
  assert.equal(zero.entries.size, 0);

  // Invalid capacities are rejected at construction.
  for (const invalid of [-1, 1.5, NaN, Infinity, Number.MAX_SAFE_INTEGER + 1,
    1e100, '2', null, undefined, 2n]) {
    assert.throws(() => new LRUCache(invalid), RangeError);
  }

  // Deterministic differential stress test against a deliberately simple model.
  const sameValueZero = (left, right) => left === right || (left !== left && right !== right);
  const referenceCache = {
    order: [],
    get(key) {
      const index = this.order.findIndex((entry) => sameValueZero(entry.key, key));
      if (index === -1) return -1;
      const [entry] = this.order.splice(index, 1);
      this.order.push(entry);
      return entry.value;
    },
    put(key, value, capacity) {
      const index = this.order.findIndex((entry) => sameValueZero(entry.key, key));
      if (index !== -1) this.order.splice(index, 1);
      else if (capacity === 0) return;
      else if (this.order.length === capacity) this.order.shift();
      this.order.push({ key: key === 0 ? 0 : key, value });
    },
  };
  const keyObjects = [{ id: 'object-key-1' }, { id: 'object-key-2' }];
  const keySymbols = [Symbol('key-1'), Symbol('key-2')];
  const keys = [undefined, null, NaN, -0, 0, 'alpha', 'beta', 1, 2, ...keyObjects, ...keySymbols];
  const values = [undefined, -1, null, 0, 'value', { id: 'object-value' }];
  let randomState = 0x6d2b79f5;
  const nextRandom = () => {
    randomState = (Math.imul(randomState ^ (randomState >>> 15), 1 | randomState) + 0x6d2b79f5) | 0;
    randomState ^= randomState + Math.imul(randomState ^ (randomState >>> 7), 61 | randomState);
    return ((randomState ^ (randomState >>> 14)) >>> 0) / 0x100000000;
  };

  for (const capacity of [0, 1, 2, 7, 32]) {
    const actual = new LRUCache(capacity);
    referenceCache.order = [];
    for (let step = 0; step < 5000; step += 1) {
      const key = keys[Math.floor(nextRandom() * keys.length)];
      if (nextRandom() < 0.55) {
        assert.equal(actual.get(key), referenceCache.get(key), `get mismatch at capacity ${capacity}, step ${step}`);
      } else {
        const value = values[Math.floor(nextRandom() * values.length)];
        actual.put(key, value);
        referenceCache.put(key, value, capacity);
      }

      assert.equal(actual.entries.size, referenceCache.order.length,
        `size mismatch at capacity ${capacity}, step ${step}`);
      const actualOrder = [];
      for (let node = actual.head.next; node !== actual.tail; node = node.next) {
        assert.equal(node.prev.next, node, 'broken previous link');
        assert.equal(actual.entries.get(node.key), node, 'list node missing from map');
        actualOrder.push(node.key);
      }
      assert.equal(actual.tail.prev.next, actual.tail, 'broken tail link');
      assert.deepEqual(actualOrder, referenceCache.order.map((entry) => entry.key),
        `recency mismatch at capacity ${capacity}, step ${step}`);
    }
  }

  console.log('LRUCache self-tests and deterministic differential stress tests passed');
}
