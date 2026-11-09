import java.util.ArrayList;
import java.util.List;

/**
 * A set of integers backed by an AVL tree.
 *
 * <p>Insertion, search, and deletion take O(log n) worst-case time because the
 * tree remains height-balanced. The tree uses O(n) storage.</p>
 */
public class AvlTree {
    private static final class Node {
        private int key;
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

    /** Removes {@code key} from this set; absent keys are ignored. */
    public void delete(int key) {
        root = delete(root, key);
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

    private Node delete(Node node, int key) {
        if (node == null) {
            return null;
        }
        if (key < node.key) {
            node.left = delete(node.left, key);
        } else if (key > node.key) {
            node.right = delete(node.right, key);
        } else {
            if (node.left == null) {
                return node.right;
            }
            if (node.right == null) {
                return node.left;
            }

            Node successor = minimum(node.right);
            node.key = successor.key;
            node.right = delete(node.right, successor.key);
        }

        updateHeight(node);
        int balance = balanceFactor(node);
        if (balance > 1) {
            if (balanceFactor(node.left) < 0) {
                node.left = rotateLeft(node.left);
            }
            return rotateRight(node);
        }
        if (balance < -1) {
            if (balanceFactor(node.right) > 0) {
                node.right = rotateRight(node.right);
            }
            return rotateLeft(node);
        }
        return node;
    }

    private static Node minimum(Node node) {
        while (node.left != null) {
            node = node.left;
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

    /** Assertion-enabled tests: run with {@code java -ea java/AvlTree.java}. */
    public static void main(String[] args) {
        assertRotation(new int[] {30, 20, 10}); // LL
        assertRotation(new int[] {10, 20, 30}); // RR
        assertRotation(new int[] {30, 10, 20}); // LR
        assertRotation(new int[] {10, 30, 20}); // RL

        assertDeletionRotations();
        assertDeletionCases();
        assertDeterministicDeletionSequence();

        AvlTree sorted = new AvlTree();
        assert sorted.height() == 0;
        for (int key = 0; key < 1_000; key++) {
            sorted.insert(key);
        }
        assert sorted.height() <= 20 : "sorted insertions should remain balanced";
        assert validTree(sorted);
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
        assert validTree(sorted);

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
        assert validTree(tree);
    }

    private static void assertDeletionCases() {
        AvlTree leaf = treeWith(20, 10, 30);
        leaf.delete(10);
        assert leaf.inorder().equals(java.util.Arrays.asList(20, 30));
        assert validTree(leaf);

        AvlTree oneChild = treeWith(20, 10, 30, 25);
        oneChild.delete(30);
        assert oneChild.inorder().equals(java.util.Arrays.asList(10, 20, 25));
        assert !oneChild.contains(30);
        assert validTree(oneChild);

        AvlTree twoChildren = treeWith(20, 10, 30, 25, 35);
        twoChildren.delete(20);
        assert twoChildren.inorder().equals(java.util.Arrays.asList(10, 25, 30, 35));
        assert !twoChildren.contains(20);
        assert validTree(twoChildren);

        AvlTree root = treeWith(2, 1, 3);
        root.delete(2);
        assert root.inorder().equals(java.util.Arrays.asList(1, 3));
        assert root.root != null && root.root.key == 3;
        assert validTree(root);

        List<Integer> beforeAbsentDelete = root.inorder();
        root.delete(99);
        assert root.inorder().equals(beforeAbsentDelete) : "deleting an absent key must do nothing";
        assert validTree(root);

        AvlTree duplicates = treeWith(2, 2, 1, 3, 2);
        assert duplicates.inorder().equals(java.util.Arrays.asList(1, 2, 3));
        duplicates.delete(2);
        assert duplicates.inorder().equals(java.util.Arrays.asList(1, 3))
                : "one deletion removes the sole set entry despite duplicate insertions";
        assert !duplicates.contains(2);
        assert validTree(duplicates);
        duplicates.delete(2);
        assert duplicates.inorder().equals(java.util.Arrays.asList(1, 3));
        assert validTree(duplicates);
    }

    private static void assertDeletionRotations() {
        // Deleting a leaf makes the root left-heavy; its left child is balanced (LL).
        assertDeletionRotation(new int[] {4, 2, 5, 1, 3}, 5, 2);
        // Mirror case: deleting a leaf makes the root right-heavy (RR).
        assertDeletionRotation(new int[] {2, 1, 4, 3, 5}, 1, 4);
        // The left subtree is right-heavy when deletion makes the root left-heavy (LR).
        assertDeletionRotation(new int[] {5, 2, 7, 1, 4, 6, 3}, 6, 4);
        // Mirror case (RL).
        assertDeletionRotation(new int[] {5, 8, 3, 9, 6, 4, 7}, 4, 6);
    }

    private static void assertDeletionRotation(int[] keys, int deletedKey, int expectedRoot) {
        AvlTree tree = treeWith(keys);
        tree.delete(deletedKey);
        assert !tree.contains(deletedKey);
        assert tree.root != null && tree.root.key == expectedRoot
                : "deletion should perform the expected rebalance";
        assert validTree(tree);
    }

    private static void assertDeterministicDeletionSequence() {
        AvlTree tree = new AvlTree();
        int[] insertions = {40, 20, 60, 10, 30, 50, 70, 25, 35, 45, 55, 5, 15};
        for (int key : insertions) {
            tree.insert(key);
            assert validTree(tree);
        }
        int[] deletions = {5, 60, 20, 40, 15, 70, 25, 55, 10, 30, 35, 45, 50};
        for (int key : deletions) {
            tree.delete(key);
            assert !tree.contains(key);
            assert validTree(tree);
        }
        assert tree.inorder().isEmpty();
        assert tree.height() == 0;
    }

    private static AvlTree treeWith(int... keys) {
        AvlTree tree = new AvlTree();
        for (int key : keys) {
            tree.insert(key);
        }
        assert validTree(tree);
        return tree;
    }

    private static boolean validTree(AvlTree tree) {
        assert validBst(tree.root, Long.MIN_VALUE, Long.MAX_VALUE) : "BST order is incorrect";
        return validHeightAndBalance(tree.root) == tree.height();
    }

    private static boolean validBst(Node node, long lowerBound, long upperBound) {
        if (node == null) {
            return true;
        }
        return node.key > lowerBound && node.key < upperBound
                && validBst(node.left, lowerBound, node.key)
                && validBst(node.right, node.key, upperBound);
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
