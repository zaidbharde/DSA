import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

/**
 * Enumerates all N-Queens boards in deterministic row-wise order.
 *
 * <p>Rows are assigned from top to bottom and candidate columns are tried from
 * left to right. Each board is represented by n strings of length n, with 'Q'
 * for a queen and '.' for an empty square. For n == 0 the sole solution is an
 * empty board (an empty list of rows).</p>
 *
 * <p>Let T be the number of search-tree states visited. The search takes
 * O(n * T) time, since each state may scan up to n columns; T is at most
 * O(n!) for this row-by-row permutation search, giving an O(n * n!) worst-case
 * upper bound. Returning solutions additionally requires O(S * n^2) time and
 * space to materialize S boards as row strings. Auxiliary search state is
 * O(n) (occupancy arrays, current placement, and recursion stack), excluding
 * the returned boards.</p>
 */
public final class NQueens {
    private NQueens() {
        // Utility class.
    }

    /**
     * Returns every n-by-n N-Queens board in stable row-wise order.
     *
     * @param n board dimension; must be nonnegative
     * @return solutions, each represented as a list of row strings
     * @throws IllegalArgumentException if n is negative
     */
    public static List<List<String>> solve(int n) {
        if (n < 0) {
            throw new IllegalArgumentException("n must be nonnegative");
        }

        List<List<String>> solutions = new ArrayList<>();
        if (n == 0) {
            solutions.add(new ArrayList<String>());
            return solutions;
        }

        int diagonalCount = Math.subtractExact(Math.multiplyExact(2, n), 1);
        boolean[] occupiedColumns = new boolean[n];
        boolean[] occupiedDescendingDiagonals = new boolean[diagonalCount];
        boolean[] occupiedAscendingDiagonals = new boolean[diagonalCount];
        int[] queenColumnByRow = new int[n];
        search(0, n, occupiedColumns, occupiedDescendingDiagonals,
                occupiedAscendingDiagonals, queenColumnByRow, solutions);
        return solutions;
    }

    private static void search(
            int row,
            int n,
            boolean[] occupiedColumns,
            boolean[] occupiedDescendingDiagonals,
            boolean[] occupiedAscendingDiagonals,
            int[] queenColumnByRow,
            List<List<String>> solutions) {
        if (row == n) {
            solutions.add(materializeBoard(n, queenColumnByRow));
            return;
        }

        for (int column = 0; column < n; column++) {
            int descendingDiagonal = row - column + n - 1;
            int ascendingDiagonal = row + column;
            if (occupiedColumns[column]
                    || occupiedDescendingDiagonals[descendingDiagonal]
                    || occupiedAscendingDiagonals[ascendingDiagonal]) {
                continue;
            }

            occupiedColumns[column] = true;
            occupiedDescendingDiagonals[descendingDiagonal] = true;
            occupiedAscendingDiagonals[ascendingDiagonal] = true;
            queenColumnByRow[row] = column;

            search(row + 1, n, occupiedColumns, occupiedDescendingDiagonals,
                    occupiedAscendingDiagonals, queenColumnByRow, solutions);

            occupiedColumns[column] = false;
            occupiedDescendingDiagonals[descendingDiagonal] = false;
            occupiedAscendingDiagonals[ascendingDiagonal] = false;
        }
    }

    private static List<String> materializeBoard(int n, int[] queenColumnByRow) {
        List<String> board = new ArrayList<>(n);
        for (int row = 0; row < n; row++) {
            char[] cells = new char[n];
            Arrays.fill(cells, '.');
            cells[queenColumnByRow[row]] = 'Q';
            board.add(new String(cells));
        }
        return board;
    }

    /** Runs the requested checks when invoked with assertions enabled. */
    public static void main(String[] args) {
        assert solve(0).equals(Arrays.asList(new ArrayList<String>()))
                : "n=0 must have one empty board";
        assert solve(1).equals(Arrays.asList(Arrays.asList("Q")))
                : "n=1 must have one solution";
        assert solve(2).isEmpty() : "n=2 has no solutions";
        assert solve(3).isEmpty() : "n=3 has no solutions";

        List<List<String>> fourQueens = solve(4);
        assert fourQueens.size() == 2 : "n=4 must have exactly two solutions";
        assert fourQueens.equals(Arrays.asList(
                Arrays.asList(".Q..", "...Q", "Q...", "..Q."),
                Arrays.asList("..Q.", "Q...", "...Q", ".Q..")))
                : "solutions must have stable row-wise ordering";

        assertRejectsNegativeSize();
        for (int n = 0; n <= 4; n++) {
            for (List<String> board : solve(n)) {
                assert hasNoAttackingQueens(board)
                        : "attacking queens found for n=" + n + ": " + board;
            }
        }
        System.out.println("NQueens assertions passed.");
    }

    private static void assertRejectsNegativeSize() {
        boolean rejected = false;
        try {
            solve(-1);
        } catch (IllegalArgumentException expected) {
            rejected = true;
        }
        assert rejected : "negative n must be rejected";
    }

    private static boolean hasNoAttackingQueens(List<String> board) {
        int n = board.size();
        boolean[] columns = new boolean[n];
        boolean[] descendingDiagonals = new boolean[n == 0 ? 0 : 2 * n - 1];
        boolean[] ascendingDiagonals = new boolean[n == 0 ? 0 : 2 * n - 1];

        for (int row = 0; row < n; row++) {
            String line = board.get(row);
            if (line.length() != n) {
                return false;
            }
            int queensInRow = 0;
            for (int column = 0; column < n; column++) {
                char cell = line.charAt(column);
                if (cell != 'Q' && cell != '.') {
                    return false;
                }
                if (cell == 'Q') {
                    queensInRow++;
                    int descendingDiagonal = row - column + n - 1;
                    int ascendingDiagonal = row + column;
                    if (columns[column]
                            || descendingDiagonals[descendingDiagonal]
                            || ascendingDiagonals[ascendingDiagonal]) {
                        return false;
                    }
                    columns[column] = true;
                    descendingDiagonals[descendingDiagonal] = true;
                    ascendingDiagonals[ascendingDiagonal] = true;
                }
            }
            if (queensInRow != 1) {
                return false;
            }
        }
        return true;
    }
}
