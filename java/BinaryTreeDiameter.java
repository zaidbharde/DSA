/**
 * Computes the diameter of a binary tree, measured as the number of edges
 * on the longest path between any two nodes.
 *
 * The empty tree has diameter 0, and a single-node tree also has diameter 0.
 * An iterative postorder traversal returns each subtree's height while
 * updating the longest path seen so far. This visits every node once: O(n)
 * time and O(n) auxiliary storage for the traversal stack and subtree heights.
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
        if (root == null) {
            return 0;
        }

        java.util.ArrayDeque<PostorderFrame> stack = new java.util.ArrayDeque<>();
        java.util.Map<Node, Integer> heights = new java.util.IdentityHashMap<>();
        stack.push(new PostorderFrame(root, false));
        int longest = 0;

        while (!stack.isEmpty()) {
            PostorderFrame frame = stack.pop();
            Node node = frame.node;
            if (frame.expanded) {
                int leftHeight = node.left == null ? -1 : heights.get(node.left);
                int rightHeight = node.right == null ? -1 : heights.get(node.right);
                longest = Math.max(longest, leftHeight + rightHeight + 2);
                heights.put(node, Math.max(leftHeight, rightHeight) + 1);
            } else {
                stack.push(new PostorderFrame(node, true));
                if (node.right != null) {
                    stack.push(new PostorderFrame(node.right, false));
                }
                if (node.left != null) {
                    stack.push(new PostorderFrame(node.left, false));
                }
            }
        }
        return longest;
    }

    /** A node scheduled for either its first visit or postorder completion. */
    private static final class PostorderFrame {
        private final Node node;
        private final boolean expanded;

        private PostorderFrame(Node node, boolean expanded) {
            this.node = node;
            this.expanded = expanded;
        }
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

        final int deepNodeCount = 50_000;
        Node deepSkewed = new Node(0);
        Node tail = deepSkewed;
        for (int i = 1; i < deepNodeCount; i++) {
            tail.right = new Node(i);
            tail = tail.right;
        }
        assert diameter(deepSkewed) == deepNodeCount - 1 : "deep skewed tree";
    }

    public static void main(String[] args) {
        runTests();
        System.out.println("All BinaryTreeDiameter tests passed.");
    }
}
