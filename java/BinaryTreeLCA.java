/**
 * Lowest common ancestor (LCA) for a general binary tree.
 *
 * Matching is by node identity (reference equality), not by {@link Node#value}.
 * If either requested node is not present in the tree, {@code lowestCommonAncestor}
 * returns {@code null}. If both target references are the same, that node is returned
 * if it is present in the tree; otherwise the result is {@code null}.
 *
 * The search takes O(n) time in the worst case and uses O(h) recursion space, where
 * n is the number of tree nodes and h is the tree height.
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

        SearchResult result = search(root, first, second);
        return result.foundFirst && result.foundSecond ? result.candidate : null;
    }

    /** Searches a subtree and records target presence independently of its candidate. */
    private static SearchResult search(Node node, Node first, Node second) {
        if (node == null) {
            return SearchResult.EMPTY;
        }

        SearchResult left = search(node.left, first, second);
        SearchResult right = search(node.right, first, second);
        boolean isFirst = node == first;
        boolean isSecond = node == second;
        boolean foundFirst = isFirst || left.foundFirst || right.foundFirst;
        boolean foundSecond = isSecond || left.foundSecond || right.foundSecond;

        Node candidate;
        if (isFirst || isSecond) {
            // A target is an ancestor of any other target in this subtree.
            candidate = node;
        } else if (left.candidate != null && right.candidate != null) {
            // The targets (or their already-computed LCA candidates) lie on both sides.
            candidate = node;
        } else {
            candidate = left.candidate != null ? left.candidate : right.candidate;
        }

        return new SearchResult(candidate, foundFirst, foundSecond);
    }

    private static final class SearchResult {
        private static final SearchResult EMPTY = new SearchResult(null, false, false);

        private final Node candidate;
        private final boolean foundFirst;
        private final boolean foundSecond;

        private SearchResult(Node candidate, boolean foundFirst, boolean foundSecond) {
            this.candidate = candidate;
            this.foundFirst = foundFirst;
            this.foundSecond = foundSecond;
        }
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
