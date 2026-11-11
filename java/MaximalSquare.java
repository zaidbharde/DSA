/**
 * Computes the area of the largest axis-aligned square containing only '1' cells
 * in a rectangular character matrix. The input matrix is never modified.
 *
 * <p>Complexity: O(rows*cols) time for validation and dynamic programming, and
 * O(cols) auxiliary space for the row-compressed DP array.</p>
 */
public final class MaximalSquare {
    private MaximalSquare() {
        // Utility class.
    }

    /**
     * Returns the area of the largest all-'1' square in {@code matrix}.
     *
     * <p>An empty (zero-row) matrix and a rectangular matrix with zero columns
     * have area zero.</p>
     *
     * @param matrix rectangular matrix containing only '0' and '1'
     * @return the largest square area, as a {@code long}
     * @throws IllegalArgumentException if the matrix is null, any row is null,
     *         the matrix is jagged, or any cell is neither '0' nor '1'
     */
    public static long maximalSquareArea(char[][] matrix) {
        if (matrix == null) {
            throw new IllegalArgumentException("matrix must not be null");
        }
        if (matrix.length == 0) {
            return 0L;
        }
        if (matrix[0] == null) {
            throw new IllegalArgumentException("matrix rows must not be null");
        }

        int rows = matrix.length;
        int columns = matrix[0].length;
        for (int row = 0; row < rows; row++) {
            if (matrix[row] == null) {
                throw new IllegalArgumentException("matrix rows must not be null");
            }
            if (matrix[row].length != columns) {
                throw new IllegalArgumentException("matrix must be rectangular");
            }
            for (int column = 0; column < columns; column++) {
                char cell = matrix[row][column];
                if (cell != '0' && cell != '1') {
                    throw new IllegalArgumentException(
                            "matrix cells must be either '0' or '1'");
                }
            }
        }
        if (columns == 0) {
            return 0L;
        }

        // dp[column] stores the square side ending at the current cell.
        // Before updating it is the value from the previous row; northwest
        // preserves the previous row's previous-column value.
        int[] dp = new int[columns];
        int largestSide = 0;
        for (int row = 0; row < rows; row++) {
            int northwest = 0;
            for (int column = 0; column < columns; column++) {
                int north = dp[column];
                if (matrix[row][column] == '1') {
                    int west = column == 0 ? 0 : dp[column - 1];
                    dp[column] = 1 + Math.min(northwest, Math.min(north, west));
                    largestSide = Math.max(largestSide, dp[column]);
                } else {
                    dp[column] = 0;
                }
                northwest = north;
            }
        }
        return (long) largestSide * largestSide;
    }

    private static void expectIllegalArgument(Runnable action, String message) {
        boolean rejected = false;
        try {
            action.run();
        } catch (IllegalArgumentException expected) {
            rejected = true;
        }
        assert rejected : message;
    }

    /** Assertion-based tests; run with {@code java -ea java/MaximalSquare.java}. */
    public static void main(String[] args) {
        assert maximalSquareArea(new char[0][0]) == 0L : "zero rows";
        assert maximalSquareArea(new char[][] {{}, {}, {}}) == 0L : "zero columns";

        char[][] example = {
                {'1', '0', '1', '0', '0'},
                {'1', '0', '1', '1', '1'},
                {'1', '1', '1', '1', '1'},
                {'1', '0', '0', '1', '0'}
        };
        assert maximalSquareArea(example) == 4L : "known example";
        assert maximalSquareArea(new char[][] {
                {'0', '1'},
                {'1', '0'}
        }) == 1L : "second known example";

        assert maximalSquareArea(new char[][] {
                {'1', '1', '1'},
                {'1', '1', '1'}
        }) == 4L : "all ones in a rectangular matrix";
        assert maximalSquareArea(new char[][] {
                {'1'}, {'1'}, {'1'}, {'1'}
        }) == 1L : "all ones in a tall narrow matrix";
        assert maximalSquareArea(new char[][] {
                {'0', '0', '0', '0'},
                {'0', '0', '0', '0'}
        }) == 0L : "all zeros";
        assert maximalSquareArea(new char[][] {
                {'1', '1', '1', '1', '1'}
        }) == 1L : "single-row rectangle";
        assert maximalSquareArea(new char[][] {
                {'0'}, {'1'}, {'1'}, {'1'}, {'1'}
        }) == 1L : "single-column rectangle";

        expectIllegalArgument(() -> maximalSquareArea(null), "null matrix rejected");
        expectIllegalArgument(() -> maximalSquareArea(new char[][] {{'1'}, null}),
                "null row rejected");
        expectIllegalArgument(() -> maximalSquareArea(new char[][] {{'1', '0'}, {'1'}}),
                "jagged matrix rejected");
        expectIllegalArgument(() -> maximalSquareArea(new char[][] {{'1', 'x'}}),
                "invalid cell rejected");

        System.out.println("MaximalSquare assertions passed.");
    }
}
