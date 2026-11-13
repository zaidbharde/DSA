import java.util.ArrayDeque;
import java.util.Collections;
import java.util.Deque;
import java.util.IdentityHashMap;
import java.util.Set;

/**
 * Lowest common ancestor (LCA) for a general binary tree.
 *
 * Matching is by node identity (reference equality), not by {@link Node#value}.
 * If either requested node is not present in the tree, {@code lowestCommonAncestor}
 * returns {@code null}. If both target references are the same, that node is returned
 * if it is present in the tree; otherwise the result is {@code null}.
 *
 * The search takes O(n) time and O(n) auxiliary space in the worst case, where n is
 * the number of tree nodes. An explicit traversal stack avoids recursion-depth risk
 * on highly skewed trees.
 */
public final class BinaryTreeLCA {
    private BinaryTreeLCA() {
        // Utility class.
    }

    /** A binary-tree node. Values need not be unique. */
    public static final class Node {
        public final int value;
        public Node left;
        public Node right;

        public Node(int value) {
            this.value = value;
        }
    }

    /**
     * Finds the lowest common ancestor of {@code first} and {@code second}.
     * The targets must be actual node references in the tree; equal values do not
     * make distinct nodes interchangeable.
     *
     * @return the LCA node, or {@code null} when the tree is empty or a target is absent
     */
    public static Node lowestCommonAncestor(Node root, Node first, Node second) {
        if (root == null || first == null || second == null) {
            return null;
        }

        Deque<Node> pending = new ArrayDeque<>();
        IdentityHashMap<Node, Node> parent = new IdentityHashMap<>();
        parent.put(root, null);
        pending.push(root);

        boolean foundFirst = false;
        boolean foundSecond = false;
        while (!pending.isEmpty()) {
            Node node = pending.pop();
            foundFirst |= node == first;
            foundSecond |= node == second;
            if (foundFirst && foundSecond) {
                break;
            }

            if (node.right != null && !parent.containsKey(node.right)) {
                parent.put(node.right, node);
                pending.push(node.right);
            }
            if (node.left != null && !parent.containsKey(node.left)) {
                parent.put(node.left, node);
                pending.push(node.left);
            }
        }

        if (!foundFirst || !foundSecond) {
            return null;
        }

        Set<Node> firstAncestors = Collections.newSetFromMap(new IdentityHashMap<Node, Boolean>());
        for (Node ancestor = first; ancestor != null; ancestor = parent.get(ancestor)) {
            firstAncestors.add(ancestor);
        }
        for (Node ancestor = second; ancestor != null; ancestor = parent.get(ancestor)) {
            if (firstAncestors.contains(ancestor)) {
                return ancestor;
            }
        }
        return null;
    }

    private static void runTests() {
        // Opposite subtrees: their parent is the LCA.
        Node root = new Node(1);
        Node left = new Node(2);
        Node right = new Node(3);
        Node leftLeaf = new Node(4);
        Node rightLeaf = new Node(5);
        root.left = left;
        root.right = right;
        left.left = leftLeaf;
        right.right = rightLeaf;
        assert lowestCommonAncestor(root, leftLeaf, rightLeaf) == root;

        // Ancestor/descendant: the ancestor itself is the LCA.
        assert lowestCommonAncestor(root, left, leftLeaf) == left;

        // Duplicate values must not confuse distinct node identities.
        Node duplicate = new Node(leftLeaf.value);
        right.left = duplicate;
        assert lowestCommonAncestor(root, leftLeaf, duplicate) == root;
        Node absentDuplicate = new Node(leftLeaf.value);
        assert lowestCommonAncestor(root, leftLeaf, absentDuplicate) == null;

        // An absent target causes null even when the other target is in the tree.
        Node absent = new Node(99);
        assert lowestCommonAncestor(root, leftLeaf, absent) == null;

        // Same reference: return it if present, and null if absent.
        assert lowestCommonAncestor(root, right, right) == right;
        assert lowestCommonAncestor(root, absent, absent) == null;

        // Empty tree.
        assert lowestCommonAncestor(null, leftLeaf, rightLeaf) == null;

        // A deep skewed tree must not consume the Java call stack.
        Node deepRoot = new Node(0);
        Node middle = deepRoot;
        Node deepest = deepRoot;
        for (int i = 1; i < 40_000; i++) {
            deepest.right = new Node(i);
            deepest = deepest.right;
            if (i == 20_000) {
                middle = deepest;
            }
        }
        assert lowestCommonAncestor(deepRoot, middle, deepest) == middle;
        assert lowestCommonAncestor(deepRoot, deepest, deepest) == deepest;
        assert lowestCommonAncestor(deepRoot, middle, absent) == null;
    }

    public static void main(String[] args) {
        boolean assertionsEnabled = false;
        assert assertionsEnabled = true;
        if (!assertionsEnabled) {
            throw new IllegalStateException("Run tests with assertions enabled: java -ea");
        }
        runTests();
        System.out.println("All BinaryTreeLCA assertions passed.");
    }
}
