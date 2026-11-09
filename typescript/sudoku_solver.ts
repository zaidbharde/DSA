/** A 9×9 Sudoku board; zero denotes an empty cell. */
export type SudokuBoard = readonly (readonly number[])[];

const SIZE = 9;
const FULL_MASK = (1 << SIZE) - 1;

/**
 * Solve a 9×9 Sudoku board using backtracking, bit masks, and the
 * minimum-remaining-values (MRV) heuristic. The input is never mutated.
 *
 * @param board A 9×9 grid of integer digits 0..9, where 0 means blank.
 * @returns A new solved 9×9 board, or `null` if the givens are consistent
 *          but no solution exists.
 * @throws {TypeError} If the board or a row is not an array.
 * @throws {RangeError} If dimensions, digits, or givens are invalid.
 *
 * Worst-case time is O(9^E), where E is the number of blank cells.
 * Auxiliary space is O(81), excluding the returned board.
 */
export function solveSudoku(board: SudokuBoard): number[][] | null {
  if (!Array.isArray(board)) {
    throw new TypeError("Sudoku board must be an array of 9 rows");
  }
  if (board.length !== SIZE) {
    throw new RangeError("Sudoku board must have exactly 9 rows");
  }

  const working: number[][] = [];
  const rowMasks = new Array<number>(SIZE).fill(0);
  const columnMasks = new Array<number>(SIZE).fill(0);
  const boxMasks = new Array<number>(SIZE).fill(0);
  const emptyCells: Array<[number, number]> = [];

  for (let row = 0; row < SIZE; row += 1) {
    const sourceRow: unknown = board[row];
    if (!Array.isArray(sourceRow)) {
      throw new TypeError(`Sudoku row ${row} must be an array`);
    }
    if (sourceRow.length !== SIZE) {
      throw new RangeError(`Sudoku row ${row} must have exactly 9 cells`);
    }

    working[row] = sourceRow.slice() as number[];
    for (let column = 0; column < SIZE; column += 1) {
      const value: unknown = sourceRow[column];
      if (typeof value !== "number" || !Number.isInteger(value) || value < 0 || value > SIZE) {
        throw new RangeError(`Cell (${row}, ${column}) must be an integer digit from 0 to 9`);
      }
      if (value === 0) {
        emptyCells.push([row, column]);
        continue;
      }

      const bit = 1 << (value - 1);
      const box = Math.floor(row / 3) * 3 + Math.floor(column / 3);
      if ((rowMasks[row] & bit) !== 0 || (columnMasks[column] & bit) !== 0 || (boxMasks[box] & bit) !== 0) {
        throw new RangeError(`Duplicate given digit ${value} at (${row}, ${column})`);
      }
      rowMasks[row] |= bit;
      columnMasks[column] |= bit;
      boxMasks[box] |= bit;
    }
  }

  function search(): boolean {
    if (emptyCells.length === 0) return true;

    // Select the unfilled cell with the fewest legal values.
    let bestIndex = -1;
    let bestCandidates = 0;
    let bestCount = SIZE + 1;
    for (let index = 0; index < emptyCells.length; index += 1) {
      const [row, column] = emptyCells[index];
      const box = Math.floor(row / 3) * 3 + Math.floor(column / 3);
      const candidates = FULL_MASK & ~(rowMasks[row] | columnMasks[column] | boxMasks[box]);
      const count = countBits(candidates);
      if (count < bestCount) {
        bestIndex = index;
        bestCandidates = candidates;
        bestCount = count;
        if (count <= 1) break;
      }
    }
    if (bestCount === 0 || bestIndex < 0) return false;

    // Move the selected cell to the end so recursive calls only scan the rest.
    const lastIndex = emptyCells.length - 1;
    [emptyCells[bestIndex], emptyCells[lastIndex]] = [emptyCells[lastIndex], emptyCells[bestIndex]];
    const [row, column] = emptyCells.pop() as [number, number];
    const box = Math.floor(row / 3) * 3 + Math.floor(column / 3);

    for (let candidates = bestCandidates; candidates !== 0; candidates &= candidates - 1) {
      const bit = candidates & -candidates;
      const digit = 32 - Math.clz32(bit);
      working[row][column] = digit;
      rowMasks[row] |= bit;
      columnMasks[column] |= bit;
      boxMasks[box] |= bit;

      if (search()) return true;

      rowMasks[row] ^= bit;
      columnMasks[column] ^= bit;
      boxMasks[box] ^= bit;
      working[row][column] = 0;
    }

    emptyCells.push([row, column]);
    [emptyCells[bestIndex], emptyCells[lastIndex]] = [emptyCells[lastIndex], emptyCells[bestIndex]];
    return false;
  }

  return search() ? working : null;
}

function countBits(mask: number): number {
  let count = 0;
  for (let remaining = mask; remaining !== 0; remaining &= remaining - 1) {
    count += 1;
  }
  return count;
}

function assertSudoku(condition: unknown, message: string): asserts condition {
  if (!condition) {
    throw new Error(`Sudoku self-test failed: ${message}`);
  }
}

function expectSudokuError(
  action: () => void,
  expectedError: typeof TypeError | typeof RangeError,
  message: string,
): void {
  let threwExpectedError = false;
  try {
    action();
  } catch (error: unknown) {
    threwExpectedError = error instanceof expectedError;
  }
  assertSudoku(threwExpectedError, `${message} should throw ${expectedError.name}`);
}

function runSudokuSelfTests(): void {
  const puzzle = [
    [5, 3, 0, 0, 7, 0, 0, 0, 0],
    [6, 0, 0, 1, 9, 5, 0, 0, 0],
    [0, 9, 8, 0, 0, 0, 0, 6, 0],
    [8, 0, 0, 0, 6, 0, 0, 0, 3],
    [4, 0, 0, 8, 0, 3, 0, 0, 1],
    [7, 0, 0, 0, 2, 0, 0, 0, 6],
    [0, 6, 0, 0, 0, 0, 2, 8, 0],
    [0, 0, 0, 4, 1, 9, 0, 0, 5],
    [0, 0, 0, 0, 8, 0, 0, 7, 9],
  ];
  const expected = [
    [5, 3, 4, 6, 7, 8, 9, 1, 2],
    [6, 7, 2, 1, 9, 5, 3, 4, 8],
    [1, 9, 8, 3, 4, 2, 5, 6, 7],
    [8, 5, 9, 7, 6, 1, 4, 2, 3],
    [4, 2, 6, 8, 5, 3, 7, 9, 1],
    [7, 1, 3, 9, 2, 4, 8, 5, 6],
    [9, 6, 1, 5, 3, 7, 2, 8, 4],
    [2, 8, 7, 4, 1, 9, 6, 3, 5],
    [3, 4, 5, 2, 8, 6, 1, 7, 9],
  ];
  const originalPuzzle = JSON.stringify(puzzle);
  const solution = solveSudoku(puzzle);
  assertSudoku(solution !== null, "known puzzle should be solvable");
  assertSudoku(JSON.stringify(solution) === JSON.stringify(expected), "known solution mismatch");
  assertSudoku(JSON.stringify(puzzle) === originalPuzzle, "solver must not mutate its input");

  const solvedBoard = solveSudoku(expected);
  assertSudoku(solvedBoard !== null, "a valid solved board should be accepted");
  assertSudoku(JSON.stringify(solvedBoard) === JSON.stringify(expected), "solved board should be copied unchanged");

  const unsolvable = [
    [0, 1, 2, 3, 0, 0, 0, 0, 0],
    [4, 7, 8, 0, 0, 0, 0, 0, 0],
    [5, 9, 0, 0, 0, 0, 0, 0, 0],
    [6, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 0, 0, 0, 0, 0, 0],
  ];
  assertSudoku(solveSudoku(unsolvable) === null, "consistent but impossible givens should return null");
  const emptyBoard = Array.from({ length: SIZE }, () => new Array<number>(SIZE).fill(0));
  assertSudoku(solveSudoku(emptyBoard) !== null, "empty board should have a solution");

  expectSudokuError(() => solveSudoku([]), RangeError, "wrong board dimensions");
  expectSudokuError(() => solveSudoku([[0]]), RangeError, "wrong row dimensions");
  const invalidDigit = expected.map((row) => [...row]);
  invalidDigit[0][0] = 10;
  expectSudokuError(() => solveSudoku(invalidDigit), RangeError, "out-of-range digit");
  const duplicate = expected.map((row) => [...row]);
  duplicate[0][0] = duplicate[0][1];
  expectSudokuError(() => solveSudoku(duplicate), RangeError, "duplicate givens");
  expectSudokuError(() => solveSudoku(null as unknown as SudokuBoard), TypeError, "null board");

  console.log("sudoku_solver.ts: all self-tests passed");
}

runSudokuSelfTests();
