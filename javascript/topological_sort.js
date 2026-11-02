'use strict';

/**
 * Return a topological ordering of a directed graph using Kahn's algorithm.
 *
 * @param {number} vertexCount Number of vertices, indexed from 0 to vertexCount - 1.
 * @param {Array<[number, number]>} edges Directed edges represented as [from, to].
 * @returns {number[] | null} A topological ordering, or null if the graph has a cycle.
 * @throws {TypeError|RangeError} If the vertex count or edge list is invalid.
 *
 * Vertices are emitted in deterministic FIFO order: zero-indegree vertices are
 * initially queued in ascending order, and newly ready vertices follow edge order.
 * Time complexity: O(V + E). Space complexity: O(V + E).
 */
function topologicalSort(vertexCount, edges) {
  if (!Number.isInteger(vertexCount) || vertexCount < 0) {
    throw new RangeError('vertexCount must be a non-negative integer');
  }
  if (!Array.isArray(edges)) {
    throw new TypeError('edges must be an array of [from, to] pairs');
  }

  const adjacency = Array.from({ length: vertexCount }, () => []);
  const inDegree = new Array(vertexCount).fill(0);

  for (let i = 0; i < edges.length; i += 1) {
    const edge = edges[i];
    if (!Array.isArray(edge) || edge.length !== 2) {
      throw new TypeError(`edge at index ${i} must be a [from, to] pair`);
    }

    const [from, to] = edge;
    if (!Number.isInteger(from) || !Number.isInteger(to)) {
      throw new TypeError(`edge at index ${i} must contain integer vertex indices`);
    }
    if (from < 0 || from >= vertexCount || to < 0 || to >= vertexCount) {
      throw new RangeError(`edge at index ${i} contains an out-of-range vertex index`);
    }

    adjacency[from].push(to);
    inDegree[to] += 1;
  }

  // Each vertex enters the queue at most once; fixed capacity avoids resizing.
  const ready = new Array(vertexCount);
  let tail = 0;
  for (let vertex = 0; vertex < vertexCount; vertex += 1) {
    if (inDegree[vertex] === 0) ready[tail++] = vertex;
  }

  const ordering = [];
  for (let head = 0; head < tail; head += 1) {
    const vertex = ready[head];
    ordering.push(vertex);

    for (const neighbor of adjacency[vertex]) {
      inDegree[neighbor] -= 1;
      if (inDegree[neighbor] === 0) ready[tail++] = neighbor;
    }
  }

  return ordering.length === vertexCount ? ordering : null;
}

module.exports = topologicalSort;

if (require.main === module) {
  const assert = require('node:assert/strict');

  function assertValidOrdering(vertexCount, edges, ordering) {
    assert.equal(ordering.length, vertexCount);
    const position = new Array(vertexCount);
    ordering.forEach((vertex, index) => {
      assert.ok(Number.isInteger(vertex) && vertex >= 0 && vertex < vertexCount);
      assert.equal(position[vertex], undefined, `vertex ${vertex} appears more than once`);
      position[vertex] = index;
    });
    for (const [from, to] of edges) {
      assert.ok(position[from] < position[to], `edge ${from}->${to} violates the ordering`);
    }
  }

  const dagEdges = [[5, 2], [5, 0], [4, 0], [4, 1], [2, 3], [3, 1]];
  const dagOrdering = topologicalSort(6, dagEdges);
  assert.deepEqual(dagOrdering, [4, 5, 2, 0, 3, 1]);
  assert.notEqual(dagOrdering, null);
  assertValidOrdering(6, dagEdges, dagOrdering);

  assert.deepEqual(topologicalSort(0, []), []);
  assert.equal(topologicalSort(3, [[0, 1], [1, 2], [2, 0]]), null);
  assert.equal(topologicalSort(1, [[0, 0]]), null);

  // Exercise a large graph with many disconnected components and isolated vertices.
  const largeVertexCount = 12000;
  const largeEdges = [];
  for (let vertex = 0; vertex + 1 < largeVertexCount; vertex += 3) {
    largeEdges.push([vertex, vertex + 1]);
  }
  const largeOrdering = topologicalSort(largeVertexCount, largeEdges);
  assertValidOrdering(largeVertexCount, largeEdges, largeOrdering);
  assert.deepEqual(topologicalSort(largeVertexCount, largeEdges), largeOrdering);

  assert.throws(() => topologicalSort(2, [[0, 2]]), RangeError);
  console.log('topological_sort.js: all self-tests passed');
}
