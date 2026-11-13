'use strict';

/**
 * Binary-tree node.
 */
class Node {
  constructor(value, left = null, right = null) {
    this.value = value;
    this.left = left;
    this.right = right;
  }
}

/**
 * Return values grouped by vertical column, from leftmost to rightmost.
 * Within a column, values appear by level and then in left-to-right BFS
 * discovery order for ties. Returns [] for an empty tree.
 *
 * Time: O(n + k log k), where n is the number of nodes and k is the number
 * of occupied columns. BFS and periodic queue compaction take O(n) total;
 * sorting the columns takes O(k log k).
 * Space: O(n) for column groups and the returned result. Queue storage is
 * O(w + 1), where w is the maximum BFS frontier width, because consumed
 * entries are periodically compacted.
 *
 * @param {Node|null|undefined} root
 * @returns {Array<Array<*>>}
 */
function verticalOrderTraversal(root) {
  if (root == null) return [];

  const columns = new Map();
  const queue = [{ node: root, column: 0 }];
  let head = 0;

  while (head < queue.length) {
    const { node, column } = queue[head++];

    // Drop consumed references in batches. Requiring the consumed prefix to
    // be at least as large as the pending suffix keeps total compaction work
    // linear, and the fixed threshold avoids frequent tiny copies.
    if (head >= 1024 && head * 2 >= queue.length) {
      queue.splice(0, head);
      head = 0;
    }

    if (!columns.has(column)) columns.set(column, []);
    columns.get(column).push(node.value);

    if (node.left != null) queue.push({ node: node.left, column: column - 1 });
    if (node.right != null) queue.push({ node: node.right, column: column + 1 });
  }

  return [...columns.keys()]
    .sort((a, b) => a - b)
    .map((column) => columns.get(column));
}

module.exports = { Node, verticalOrderTraversal };

if (require.main === module) {
  const assert = require('node:assert/strict');

  // Null tree.
  assert.deepEqual(verticalOrderTraversal(null), []);
  assert.deepEqual(verticalOrderTraversal(undefined), []);

  // Singleton tree.
  assert.deepEqual(verticalOrderTraversal(new Node(7)), [[7]]);

  // Balanced tree with collisions: 5 and 6 share a column and level;
  // their order follows left-to-right BFS discovery. Nodes 8 and 9 do too.
  const balanced = new Node(
    1,
    new Node(2, new Node(4), new Node(5, new Node(8))),
    new Node(3, new Node(6, new Node(9)), new Node(7)),
  );
  assert.deepEqual(verticalOrderTraversal(balanced), [
    [4],
    [2, 8, 9],
    [1, 5, 6],
    [3],
    [7],
  ]);

  // Duplicate values are retained, including when tied at the same level;
  // their relative order is still left-to-right BFS discovery order.
  const duplicates = new Node(
    4,
    new Node(4, null, new Node(4)),
    new Node(4, new Node(4)),
  );
  assert.deepEqual(verticalOrderTraversal(duplicates), [
    [4],
    [4, 4, 4],
    [4],
  ]);

  // Right-skewed tree.
  const rightSkewed = new Node(1, null, new Node(2, null, new Node(3)));
  assert.deepEqual(verticalOrderTraversal(rightSkewed), [[1], [2], [3]]);

  // Left-skewed tree verifies that negative columns are ordered ascending.
  const leftSkewed = new Node(1, new Node(2, new Node(3)));
  assert.deepEqual(verticalOrderTraversal(leftSkewed), [[3], [2], [1]]);

  // Large skewed input exercises iterative traversal and repeated queue
  // compaction without recursion or retaining the entire processed prefix.
  const skewedNodeCount = 20_000;
  let largeRightSkewed = null;
  for (let value = skewedNodeCount - 1; value >= 0; value -= 1) {
    largeRightSkewed = new Node(value, null, largeRightSkewed);
  }
  assert.deepEqual(
    verticalOrderTraversal(largeRightSkewed),
    Array.from({ length: skewedNodeCount }, (_, value) => [value]),
  );

  console.log('All binary_tree_vertical_order tests passed.');
}
