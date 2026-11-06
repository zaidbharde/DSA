import java.util.Arrays;

/**
 * Counts four-directionally connected land components in a rectangular grid.
 * Land is represented by '1' and water by '0'. The input grid is never modified.
 *
 * <p>Complexity: O(R*C) time, since validation and traversal inspect each cell
 * at most a constant number of times. Auxiliary memory is O(R*C) in the worst
 * case for the visited matrix and reusable primitive work stack.</p>
 */
public class NumberOfIslands {
    /**
     * Returns the number of four-directional islands in {@code grid}.
     *
     * @param grid rectangular grid containing '1' (land) and '0' (water); null
     *             and zero-row grids are treated as empty
     * @throws IllegalArgumentException if any row is null or the grid is jagged
     */
    public static int countIslands(char[][] grid) {
        if (grid == null || grid.length == 0) {
            return 0;
        }

        if (grid[0] == null) {
            throw new IllegalArgumentException("Grid rows must not be null");
        }
        int rows = grid.length;
        int columns = grid[0].length;
        for (int row = 1; row < rows; row++) {
            if (grid[row] == null || grid[row].length != columns) {
                throw new IllegalArgumentException("Grid must be rectangular with non-null rows");
            }
        }
        if (columns == 0) {
            return 0;
        }

        boolean[][] visited = new boolean[rows][columns];
        // Reuse one primitive stack for all components, avoiding boxed cells
        // and per-island queue allocations.
        int[] worklist = new int[16];
        int islands = 0;
        int[] rowStep = {-1, 1, 0, 0};
        int[] columnStep = {0, 0, -1, 1};

        for (int row = 0; row < rows; row++) {
            for (int column = 0; column < columns; column++) {
                if (grid[row][column] == '1' && !visited[row][column]) {
                    islands++;
                    int size = 0;
                    visited[row][column] = true;
                    worklist = push(worklist, row * columns + column, size++);

                    while (size > 0) {
                        int cell = worklist[--size];
                        int currentRow = cell / columns;
                        int currentColumn = cell % columns;
                        for (int direction = 0; direction < 4; direction++) {
                            int nextRow = currentRow + rowStep[direction];
                            int nextColumn = currentColumn + columnStep[direction];
                            if (nextRow >= 0 && nextRow < rows
                                    && nextColumn >= 0 && nextColumn < columns
                                    && grid[nextRow][nextColumn] == '1'
                                    && !visited[nextRow][nextColumn]) {
                                // Mark on push so a cell can never be added twice.
                                visited[nextRow][nextColumn] = true;
                                worklist = push(worklist,
                                        nextRow * columns + nextColumn, size++);
                            }
                        }
                    }
                }
            }
        }
        return islands;
    }

    private static int[] push(int[] worklist, int cell, int index) {
        if (index == worklist.length) {
            int newLength = worklist.length <= Integer.MAX_VALUE / 2
                    ? worklist.length * 2 : Integer.MAX_VALUE;
            worklist = Arrays.copyOf(worklist, newLength);
        }
        worklist[index] = cell;
        return worklist;
    }

    private static void expectJaggedGridRejected() {
        char[][] jagged = { {'1', '0'}, {'1'} };
        boolean rejected = false;
        try {
            countIslands(jagged);
        } catch (IllegalArgumentException expected) {
            rejected = true;
        }
        assert rejected : "Jagged grids must be rejected";
    }

    private static void runSelfTests() {
        assert countIslands(null) == 0 : "null grid";
        assert countIslands(new char[0][0]) == 0 : "empty grid";
        assert countIslands(new char[][] { {}, {} }) == 0 : "empty rows";
        assert countIslands(new char[][] {
                {'0', '0', '0'},
                {'0', '0', '0'}
        }) == 0 : "all-water grid";
        assert countIslands(new char[][] {
                {'1', '1', '0'},
                {'0', '1', '0'},
                {'0', '0', '0'}
        }) == 1 : "one island";
        char[][] multipleIslands = {
                {'1', '1', '0'},
                {'0', '1', '0'},
                {'0', '0', '1'}
        };
        char[][] original = {
                {'1', '1', '0'},
                {'0', '1', '0'},
                {'0', '0', '1'}
        };
        assert countIslands(multipleIslands) == 2 : "multiple islands";
        assert java.util.Arrays.deepEquals(multipleIslands, original) : "input must not be mutated";
        assert countIslands(new char[][] {
                {'1', '0', '1', '0'},
                {'0', '1', '0', '1'},
                {'1', '0', '1', '0'}
        }) == 6 : "multiple diagonally separated islands";
        assert countIslands(new char[][] {
                {'1', '0', '1', '1', '0', '1'}
        }) == 3 : "single-row boundary islands";
        assert countIslands(new char[][] {
                {'1'}, {'0'}, {'1'}, {'1'}, {'0'}, {'1'}
        }) == 3 : "single-column boundary islands";
        assert countIslands(new char[][] {
                {'1', '0', '1'},
                {'0', '0', '0'},
                {'1', '0', '1'}
        }) == 4 : "four corner islands";

        // Large connected component guards against recursive stack overflow.
        char[][] largeLandMass = new char[800][800];
        for (char[] row : largeLandMass) {
            Arrays.fill(row, '1');
        }
        assert countIslands(largeLandMass) == 1 : "large all-land grid";
        expectJaggedGridRejected();
    }

    public static void main(String[] args) {
        runSelfTests();
        System.out.println("All NumberOfIslands self-tests passed.");
    }
}
