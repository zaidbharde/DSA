/**
 * Finds a word in a rectangular character board by walking through
 * orthogonally adjacent cells (up, down, left, right). A cell may occur at
 * most once in a path.
 *
 * <p>Input contract: a null board, null word, null row, or ragged board is
 * rejected with {@link IllegalArgumentException}. An empty word is found
 * (returns {@code true}), including on an empty board. An empty board, or a
 * rectangular board with zero columns, contains no non-empty word. The input
 * board is not modified.</p>
 *
 * <p>For a word of length L on an R-by-C board, the worst-case search time is
 * O(R*C*3^L): each start cell can branch to at most four neighbors initially,
 * then at most three new neighbors per step because the previous cell cannot
 * be reused. A single boolean visited matrix is allocated for each search and
 * reused across all starting positions; it is temporary O(R*C) bookkeeping,
 * not recursion space. The DFS recursion stack is O(L), so total auxiliary
 * space is O(R*C + L), of which O(L) is recursion space.</p>
 */
public final class WordSearch {
    private WordSearch() {
        // Utility class.
    }

    /**
     * Returns whether {@code word} can be formed by an orthogonal path in
     * {@code board}, without reusing a cell.
     *
     * @throws IllegalArgumentException if either argument is null, any row is
     *         null, or the board is not rectangular
     */
    public static boolean exists(char[][] board, String word) {
        if (board == null) {
            throw new IllegalArgumentException("board must not be null");
        }
        if (word == null) {
            throw new IllegalArgumentException("word must not be null");
        }

        int rows = board.length;
        int columns = 0;
        if (rows > 0) {
            if (board[0] == null) {
                throw new IllegalArgumentException("board rows must not be null");
            }
            columns = board[0].length;
            for (int row = 1; row < rows; row++) {
                if (board[row] == null) {
                    throw new IllegalArgumentException("board rows must not be null");
                }
                if (board[row].length != columns) {
                    throw new IllegalArgumentException("board must be rectangular");
                }
            }
        }

        if (word.isEmpty()) {
            return true;
        }
        if (rows == 0 || columns == 0 || word.length() > rows * (long) columns) {
            return false;
        }

        boolean[][] visited = new boolean[rows][columns];
        for (int row = 0; row < rows; row++) {
            for (int column = 0; column < columns; column++) {
                if (search(board, word, row, column, 0, visited)) {
                    return true;
                }
            }
        }
        return false;
    }

    private static boolean search(char[][] board, String word, int row, int column,
                                  int index, boolean[][] visited) {
        if (row < 0 || row >= board.length
                || column < 0 || column >= board[0].length
                || visited[row][column]
                || board[row][column] != word.charAt(index)) {
            return false;
        }
        if (index == word.length() - 1) {
            return true;
        }

        visited[row][column] = true;
        boolean found = search(board, word, row - 1, column, index + 1, visited)
                || search(board, word, row + 1, column, index + 1, visited)
                || search(board, word, row, column - 1, index + 1, visited)
                || search(board, word, row, column + 1, index + 1, visited);
        visited[row][column] = false;
        return found;
    }

    private static void expectIllegalArgument(Runnable action) {
        boolean thrown = false;
        try {
            action.run();
        } catch (IllegalArgumentException expected) {
            thrown = true;
        }
        assert thrown : "Expected IllegalArgumentException";
    }

    /** Assertion-based smoke tests; run with {@code java -ea java/WordSearch.java}. */
    public static void main(String[] args) {
        char[][] board = {
                {'A', 'B', 'C', 'E'},
                {'S', 'F', 'C', 'S'},
                {'A', 'D', 'E', 'E'}
        };
        assert exists(board, "ABCCED");
        assert exists(board, "SEE");
        assert !exists(board, "ABCB");

        assert exists(new char[][]{{'Q'}}, "Q");
        assert !exists(new char[][]{{'Q'}}, "R");

        assert exists(new char[0][0], "");
        assert exists(new char[][]{{'X'}}, "");
        assert !exists(new char[0][0], "X");
        assert !exists(new char[][]{{}, {}}, "X");

        // A-B-A would require returning to the only A cell, which is forbidden.
        assert !exists(new char[][]{{'A', 'B'}}, "ABA");

        expectIllegalArgument(() -> exists(null, "A"));
        expectIllegalArgument(() -> exists(new char[0][0], null));
        expectIllegalArgument(() -> exists(new char[][]{{'A'}, {'B', 'C'}}, ""));
        expectIllegalArgument(() -> exists(new char[][]{{'A'}, null}, "A"));

        System.out.println("WordSearch assertions passed.");
    }
}
