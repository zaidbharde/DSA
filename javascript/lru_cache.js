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
 * Least-recently-used cache with O(1) get/put operations.
 * A successful get or put makes the key the most recently used entry.
 */
class LRUCache {
  constructor(capacity) {
    if (!Number.isInteger(capacity) || capacity < 0) {
      throw new RangeError('capacity must be a nonnegative integer');
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

    const node = new Node(key, value);
    this.entries.set(key, node);
    this._insertBeforeTail(node);

    if (this.entries.size > this.capacity) {
      const leastRecent = this.head.next;
      this._unlink(leastRecent);
      this.entries.delete(leastRecent.key);
    }
  }

  _moveToMostRecent(node) {
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
  for (const invalid of [-1, 1.5, NaN, Infinity, '2', null, undefined]) {
    assert.throws(() => new LRUCache(invalid), RangeError);
  }

  console.log('LRUCache self-tests passed');
}
