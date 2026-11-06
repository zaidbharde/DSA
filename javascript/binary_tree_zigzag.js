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
 * Return the values of each tree level, alternating left-to-right and
 * right-to-left order, starting left-to-right. Returns [] for an empty tree.
 *
 * Time: O(n), where n is the number of nodes.
 * Space: O(w) queue/level space, where w is the maximum tree width,
 * excluding the returned result.
 *
 * @param {Node|null|undefined} root
 * @returns {Array<Array<*>>}
 */
function zigzagLevelOrder(root) {
  if (root == null) return [];

  const result = [];
  let queue = [root];
  let head = 0;
  let leftToRight = true;

  while (head < queue.length) {
    const levelEnd = queue.length;
    const levelSize = levelEnd - head;
    const level = new Array(levelSize);

    for (let i = 0; i < levelSize; i += 1) {
      const node = queue[head++];
      const targetIndex = leftToRight ? i : levelSize - 1 - i;
      level[targetIndex] = node.value;

      if (node.left != null) queue.push(node.left);
      if (node.right != null) queue.push(node.right);
    }

    result.push(level);
    leftToRight = !leftToRight;

    // Discard consumed nodes at each level so the backing queue stays O(w).
    queue = queue.slice(head);
    head = 0;
  }

  return result;
}

module.exports = { Node, zigzagLevelOrder };

if (require.main === module) {
  const assert = require('node:assert/strict');

  // Null tree.
  assert.deepEqual(zigzagLevelOrder(null), []);

  // Singleton tree.
  assert.deepEqual(zigzagLevelOrder(new Node(7)), [[7]]);

  // Balanced multi-level tree.
  const balanced = new Node(
    1,
    new Node(2, new Node(4), new Node(5)),
    new Node(3, new Node(6), new Node(7)),
  );
  assert.deepEqual(zigzagLevelOrder(balanced), [[1], [3, 2], [4, 5, 6, 7]]);

  // Skewed tree: alternating direction still produces one value per level.
  const skewed = new Node(1, null, new Node(2, null, new Node(3, null, new Node(4))));
  assert.deepEqual(zigzagLevelOrder(skewed), [[1], [2], [3], [4]]);

  console.log('All binary_tree_zigzag tests passed.');
}
