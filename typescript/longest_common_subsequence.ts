/**
 * Returns the length of the longest common subsequence of `first` and `second`.
 *
 * Strings are compared as UTF-16 code units (the units exposed by JavaScript
 * string indexing), not as Unicode code points or grapheme clusters.
 * Time: O(mn), where m and n are the input string lengths in UTF-16 code units.
 * Auxiliary space: O(min(m, n)) using two rows, with the shorter string
 * providing the row width.
 */
export function longestCommonSubsequence(first: string, second: string): number {
  // Keep the shorter input across the rows to minimize auxiliary storage.
  const rowString: string = first.length <= second.length ? first : second;
  const columnString: string = first.length <= second.length ? second : first;
  const width: number = rowString.length;

  let previous: number[] = new Array<number>(width + 1).fill(0);
  let current: number[] = new Array<number>(width + 1).fill(0);

  for (let row: number = 1; row <= columnString.length; row += 1) {
    current[0] = 0;
    const rowCodeUnit: number = columnString.charCodeAt(row - 1);

    for (let column: number = 1; column <= width; column += 1) {
      if (rowCodeUnit === rowString.charCodeAt(column - 1)) {
        current[column] = previous[column - 1] + 1;
      } else {
        current[column] = Math.max(previous[column], current[column - 1]);
      }
    }

    const completedRow: number[] = previous;
    previous = current;
    current = completedRow;
  }

  return previous[width];
}

interface TestCase {
  readonly name: string;
  readonly first: string;
  readonly second: string;
  readonly expected: number;
}

const testCases: readonly TestCase[] = [
  { name: "both empty", first: "", second: "", expected: 0 },
  { name: "first empty", first: "", second: "abc", expected: 0 },
  { name: "second empty", first: "abc", second: "", expected: 0 },
  { name: "equal strings", first: "subsequence", second: "subsequence", expected: 11 },
  { name: "classic example", first: "ABCBDAB", second: "BDCABA", expected: 4 },
  { name: "classic example 2", first: "AGGTAB", second: "GXTXAYB", expected: 4 },
  { name: "disjoint inputs", first: "abc", second: "XYZ", expected: 0 },
  { name: "repeated characters", first: "aaaaaa", second: "baaab", expected: 3 },
  { name: "short first input", first: "ace", second: "abcdefghij", expected: 3 },
  { name: "short second input", first: "abcdefghij", second: "ace", expected: 3 },
  // A supplementary Unicode character consists of two UTF-16 code units.
  { name: "UTF-16 code-unit semantics", first: "😀", second: "\uD83D", expected: 1 },
];

function runSelfTests(): void {
  for (const testCase of testCases) {
    const actual: number = longestCommonSubsequence(testCase.first, testCase.second);
    if (actual !== testCase.expected) {
      throw new Error(
        `${testCase.name}: expected ${testCase.expected}, received ${actual}`,
      );
    }
  }
}

runSelfTests();
