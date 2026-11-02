import java.util.ArrayList;
import java.util.List;

/**
 * A set of integers backed by an AVL tree.
 *
 * <p>Insertion and search take O(log n) worst-case time because the tree remains
 * height-balanced. The tree uses O(n) storage.</p>
 */
public class AvlTree {
    private static final class Node {
        private final int key;
        private Node left;
        private Node right;
        private int height = 1;

        private Node(int key) {
            this.key = key;
        }
    }

    private Node root;

    /** Adds {@code key} to this set; duplicate keys are ignored. */
    public void insert(int key) {
        root = insert(root, key);
    }

    /** Returns whether this set contains {@code key}. */
    public boolean contains(int key) {
        Node current = root;
        while (current != null) {
            if (key == current.key) {
                return true;
            }
            current = key < current.key ? current.left : current.right;
        }
        return false;
    }

    /** Returns the tree height, with an empty tree having height zero. */
    public int height() {
        return nodeHeight(root);
    }

    /** Returns all keys in ascending order. */
    public List<Integer> inorder() {
        List<Integer> result = new ArrayList<>();
        appendInorder(root, result);
        return result;
    }

    private Node insert(Node node, int key) {
        if (node == null) {
            return new Node(key);
        }
        if (key < node.key) {
            node.left = insert(node.left, key);
        } else if (key > node.key) {
            node.right = insert(node.right, key);
        } else {
            return node;
        }

        updateHeight(node);
        int balance = balanceFactor(node);

        // Left-left case.
        if (balance > 1 && key < node.left.key) {
            return rotateRight(node);
        }
        // Right-right case.
        if (balance < -1 && key > node.right.key) {
            return rotateLeft(node);
        }
        // Left-right case.
        if (balance > 1 && key > node.left.key) {
            node.left = rotateLeft(node.left);
            return rotateRight(node);
        }
        // Right-left case.
        if (balance < -1 && key < node.right.key) {
            node.right = rotateRight(node.right);
            return rotateLeft(node);
        }
        return node;
    }

    private static Node rotateRight(Node top) {
        Node newTop = top.left;
        Node movedSubtree = newTop.right;
        newTop.right = top;
        top.left = movedSubtree;
        updateHeight(top);
        updateHeight(newTop);
        return newTop;
    }

    private static Node rotateLeft(Node top) {
        Node newTop = top.right;
        Node movedSubtree = newTop.left;
        newTop.left = top;
        top.right = movedSubtree;
        updateHeight(top);
        updateHeight(newTop);
        return newTop;
    }

    private static int nodeHeight(Node node) {
        return node == null ? 0 : node.height;
    }

    private static int balanceFactor(Node node) {
        return node == null ? 0 : nodeHeight(node.left) - nodeHeight(node.right);
    }

    private static void updateHeight(Node node) {
        node.height = 1 + Math.max(nodeHeight(node.left), nodeHeight(node.right));
    }

    private static void appendInorder(Node node, List<Integer> result) {
        if (node == null) {
            return;
        }
        appendInorder(node.left, result);
        result.add(node.key);
        appendInorder(node.right, result);
    }

    /** Assertion-enabled smoke tests: run with {@code java -ea java/AvlTree.java}. */
    public static void main(String[] args) {
        assertRotation(new int[] {30, 20, 10}); // LL
        assertRotation(new int[] {10, 20, 30}); // RR
        assertRotation(new int[] {30, 10, 20}); // LR
        assertRotation(new int[] {10, 30, 20}); // RL

        AvlTree sorted = new AvlTree();
        assert sorted.height() == 0;
        for (int key = 0; key < 1_000; key++) {
            sorted.insert(key);
        }
        assert sorted.height() <= 20 : "sorted insertions should remain balanced";
        assert validHeightAndBalance(sorted.root) == sorted.height();
        assert sorted.inorder().size() == 1_000;
        assert sorted.inorder().get(0) == 0;
        assert sorted.inorder().get(999) == 999;
        for (int key = 0; key < 1_000; key++) {
            assert sorted.contains(key);
        }
        assert !sorted.contains(-1);
        assert !sorted.contains(1_000);

        List<Integer> beforeDuplicates = sorted.inorder();
        for (int key = 0; key < 1_000; key++) {
            sorted.insert(key);
        }
        assert sorted.inorder().equals(beforeDuplicates) : "duplicates must be ignored";
        assert validHeightAndBalance(sorted.root) == sorted.height();

        System.out.println("All AVL assertions passed.");
    }

    private static void assertRotation(int[] keys) {
        AvlTree tree = new AvlTree();
        for (int key : keys) {
            tree.insert(key);
        }
        assert tree.root != null && tree.root.key == 20 : "rotation should promote the median";
        assert tree.height() == 2;
        assert tree.inorder().equals(java.util.Arrays.asList(10, 20, 30));
        assert validHeightAndBalance(tree.root) == tree.height();
    }

    /** Checks every stored height and balance factor; returns the actual subtree height. */
    private static int validHeightAndBalance(Node node) {
        if (node == null) {
            return 0;
        }
        int leftHeight = validHeightAndBalance(node.left);
        int rightHeight = validHeightAndBalance(node.right);
        assert Math.abs(leftHeight - rightHeight) <= 1 : "AVL balance factor out of range";
        int actualHeight = 1 + Math.max(leftHeight, rightHeight);
        assert node.height == actualHeight : "stored height is incorrect";
        return actualHeight;
    }
}
