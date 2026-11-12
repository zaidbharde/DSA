'use strict';

/**
 * Return the canonical skyline key points for buildings [left, right, height].
 * Each building must be a three-number array with finite coordinates, right >
 * left, and height > 0; malformed tuples raise TypeError and nonpositive
 * widths/heights raise RangeError. Empty input returns [].
 *
 * The sweep sorts 2n endpoint events, then inserts each building once into a
 * max-heap and lazily removes ended buildings when they reach the top. Time is
 * O(n log n); auxiliary space is O(n), excluding the returned key-point list.
 * @param {Array<[number, number, number]>} buildings
 * @returns {Array<[number, number]>}
 */
function getSkyline(buildings) {
  if (!Array.isArray(buildings)) {
    throw new TypeError('buildings must be an array');
  }

  const events = [];
  for (let id = 0; id < buildings.length; id += 1) {
    const building = buildings[id];
    if (!Array.isArray(building) || building.length !== 3) {
      throw new TypeError(`buildings[${id}] must be a [left, right, height] array`);
    }

    const [left, right, height] = building;
    if (![left, right, height].every((value) => typeof value === 'number' && Number.isFinite(value))) {
      throw new TypeError(`buildings[${id}] values must be finite numbers`);
    }
    if (right <= left) {
      throw new RangeError(`buildings[${id}] must have right > left`);
    }
    if (height <= 0) {
      throw new RangeError(`buildings[${id}] must have positive height`);
    }

    events.push({ x: left, id, starts: true });
    events.push({ x: right, id, starts: false });
  }

  if (events.length === 0) {
    return [];
  }

  events.sort((a, b) => a.x - b.x);
  const active = new Array(buildings.length).fill(false);
  // Heap entries are [height, buildingId], ordered by descending height.
  const heap = [];

  const higherPriority = (a, b) => a[0] > b[0] || (a[0] === b[0] && a[1] > b[1]);
  const push = (entry) => {
    let index = heap.length;
    heap.push(entry);
    while (index > 0) {
      const parent = Math.floor((index - 1) / 2);
      if (!higherPriority(heap[index], heap[parent])) {
        break;
      }
      [heap[index], heap[parent]] = [heap[parent], heap[index]];
      index = parent;
    }
  };
  const pop = () => {
    const top = heap[0];
    const last = heap.pop();
    if (heap.length > 0) {
      heap[0] = last;
      let index = 0;
      while (true) {
        const leftChild = 2 * index + 1;
        const rightChild = leftChild + 1;
        let best = index;
        if (leftChild < heap.length && higherPriority(heap[leftChild], heap[best])) {
          best = leftChild;
        }
        if (rightChild < heap.length && higherPriority(heap[rightChild], heap[best])) {
          best = rightChild;
        }
        if (best === index) {
          break;
        }
        [heap[index], heap[best]] = [heap[best], heap[index]];
        index = best;
      }
    }
    return top;
  };

  const skyline = [];
  let index = 0;
  let previousHeight = 0;
  while (index < events.length) {
    const x = events[index].x;
    let end = index;
    while (end < events.length && events[end].x === x) {
      end += 1;
    }

    // Apply every edge at this coordinate before observing the new skyline.
    // End events invalidate heap entries; starts add a new entry.
    for (let eventIndex = index; eventIndex < end; eventIndex += 1) {
      const event = events[eventIndex];
      if (!event.starts) {
        active[event.id] = false;
      }
    }
    for (let eventIndex = index; eventIndex < end; eventIndex += 1) {
      const event = events[eventIndex];
      if (event.starts) {
        active[event.id] = true;
        push([buildings[event.id][2], event.id]);
      }
    }

    while (heap.length > 0 && !active[heap[0][1]]) {
      pop();
    }
    const height = heap.length > 0 ? heap[0][0] : 0;
    if (height !== previousHeight) {
      skyline.push([x, height]);
      previousHeight = height;
    }
    index = end;
  }

  return skyline;
}

module.exports = getSkyline;
module.exports.getSkyline = getSkyline;

if (require.main === module) {
  const assert = require('node:assert/strict');

  assert.deepEqual(
    getSkyline([[2, 9, 10], [3, 7, 15], [5, 12, 12], [15, 20, 10], [19, 24, 8]]),
    [[2, 10], [3, 15], [7, 12], [12, 0], [15, 10], [20, 8], [24, 0]],
    'classic overlapping skyline',
  );
  assert.deepEqual(getSkyline([]), [], 'empty input');
  assert.deepEqual(getSkyline([[0, 2, 3], [2, 5, 3]]), [[0, 3], [5, 0]], 'touching equal heights');
  assert.deepEqual(getSkyline([[0, 2, 3], [2, 5, 4]]), [[0, 3], [2, 4], [5, 0]], 'touching different heights');
  assert.deepEqual(getSkyline([[0, 5, 4], [1, 3, 4]]), [[0, 4], [5, 0]], 'equal-height overlap');
  assert.deepEqual(getSkyline([[0, 10, 2], [2, 8, 7], [3, 6, 1]]), [[0, 2], [2, 7], [8, 2], [10, 0]], 'nested overlap');
  assert.deepEqual(getSkyline([[0, 2, 4], [0, 3, 4], [2, 4, 4]]), [[0, 4], [4, 0]], 'equal-height shared endpoints');
  assert.deepEqual(getSkyline([[-2, -1, 5], [1, 3, 2]]), [[-2, 5], [-1, 0], [1, 2], [3, 0]], 'disjoint negative coordinates');

  assert.throws(() => getSkyline(null), TypeError, 'input must be an array');
  assert.throws(() => getSkyline([[0, 1]]), TypeError, 'tuple must have three entries');
  assert.throws(() => getSkyline([[0, 1, '2']]), TypeError, 'entries must be numbers');
  assert.throws(() => getSkyline([[0, Infinity, 2]]), TypeError, 'coordinates must be finite');
  assert.throws(() => getSkyline([[2, 2, 3]]), RangeError, 'zero-width buildings are invalid');
  assert.throws(() => getSkyline([[3, 2, 3]]), RangeError, 'negative-width buildings are invalid');
  assert.throws(() => getSkyline([[0, 2, 0]]), RangeError, 'zero-height buildings are invalid');
  assert.throws(() => getSkyline([[0, 2, -1]]), RangeError, 'negative heights are invalid');

  // Deterministic small inputs are checked against a direct endpoint scan.
  function referenceSkyline(buildings) {
    const endpoints = [...new Set(buildings.flatMap(([left, right]) => [left, right]))].sort((a, b) => a - b);
    const expected = [];
    let previous = 0;
    for (const x of endpoints) {
      let height = 0;
      for (const [left, right, buildingHeight] of buildings) {
        if (left <= x && x < right) {
          height = Math.max(height, buildingHeight);
        }
      }
      if (height !== previous) {
        expected.push([x, height]);
        previous = height;
      }
    }
    return expected;
  }

  let seed = 24681357;
  const random = () => {
    seed = (seed * 48271) % 2147483647;
    return seed;
  };
  for (let trial = 0; trial < 100; trial += 1) {
    const buildings = [];
    const count = random() % 9;
    for (let i = 0; i < count; i += 1) {
      const left = (random() % 15) - 7;
      const right = left + (random() % 6) + 1;
      const height = (random() % 10) + 1;
      buildings.push([left, right, height]);
    }
    assert.deepEqual(getSkyline(buildings), referenceSkyline(buildings), `reference check ${trial}`);
  }

  console.log('All skyline outline tests passed.');
}
