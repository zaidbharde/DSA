/**
 * Computes the diameter of a binary tree, measured as the number of edges
 * on the longest path between any two nodes.
 *
 * The empty tree has diameter 0, and a single-node tree also has diameter 0.
 * A postorder traversal returns each subtree's height while updating the
 * longest path seen so far. This visits every node once: O(n) time and O(h)
 * recursion space, where h is the tree height (O(n) in the worst case).
 */
public final class BinaryTreeDiameter {
    private BinaryTreeDiameter() {
        // Utility class.
    }

    /** A node in a binary tree. */
    public static final class Node {
        public final int value;
        public Node left;
        public Node right;

        public Node(int value) {
            this.value = value;
        }
    }

    /**
     * Returns the tree's diameter in edges.
     *
     * @param root root of the tree, or {@code null} for an empty tree
     * @return the maximum number of edges on a path between any two nodes;
     *         0 for an empty or single-node tree
     */
    public static int diameter(Node root) {
        int[] longest = {0};
        height(root, longest);
        return longest[0];
    }

    /** Returns the subtree height in edges and updates the best diameter. */
    private static int height(Node node, int[] longest) {
        if (node == null) {
            return -1;
        }

        int leftHeight = height(node.left, longest);
        int rightHeight = height(node.right, longest);
        longest[0] = Math.max(longest[0], leftHeight + rightHeight + 2);
        return Math.max(leftHeight, rightHeight) + 1;
    }

    private static void runTests() {
        assert diameter(null) == 0 : "empty tree";
        assert diameter(new Node(1)) == 0 : "singleton tree";

        Node balanced = new Node(1);
        balanced.left = new Node(2);
        balanced.right = new Node(3);
        balanced.left.left = new Node(4);
        balanced.left.right = new Node(5);
        balanced.right.left = new Node(6);
        balanced.right.right = new Node(7);
        assert diameter(balanced) == 4 : "balanced tree";

        Node skewed = new Node(1);
        skewed.right = new Node(2);
        skewed.right.right = new Node(3);
        skewed.right.right.right = new Node(4);
        skewed.right.right.right.right = new Node(5);
        assert diameter(skewed) == 4 : "skewed tree";

        // The longest path runs from node 4 to node 6 through node 2;
        // it does not pass through the root (node 1).
        Node notThroughRoot = new Node(1);
        notThroughRoot.left = new Node(2);
        notThroughRoot.left.left = new Node(3);
        notThroughRoot.left.left.left = new Node(4);
        notThroughRoot.left.right = new Node(5);
        notThroughRoot.left.right.right = new Node(6);
        notThroughRoot.right = new Node(7);
        assert diameter(notThroughRoot) == 4 : "diameter not through root";
    }

    public static void main(String[] args) {
        runTests();
        System.out.println("All BinaryTreeDiameter tests passed.");
    }
}
