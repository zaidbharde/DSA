/**
 * Returns the Levenshtein edit distance using insertion, deletion, and
 * substitution operations (each with cost 1).
 *
 * Strings are compared by UTF-16 code units, as reflected by JavaScript's
 * `length` and indexed code-unit values; a supplementary Unicode character
 * therefore consists of two units for this calculation.
 *
 * Complexity: O(mn) time and O(min(m, n)) auxiliary space, where m and n
 * are the input string lengths in UTF-16 code units.
 */
export function editDistance(first: string, second: string): number {
  // Keep the shorter string on the columns so each DP row is minimal.
  const longer: string = first.length >= second.length ? first : second;
  const shorter: string = first.length >= second.length ? second : first;
  const shorterLength: number = shorter.length;

  let previousRow: number[] = new Array<number>(shorterLength + 1);
  let currentRow: number[] = new Array<number>(shorterLength + 1);

  for (let column: number = 0; column <= shorterLength; column += 1) {
    previousRow[column] = column;
  }

  for (let row: number = 1; row <= longer.length; row += 1) {
    currentRow[0] = row;
    const longerCodeUnit: number = longer.charCodeAt(row - 1);

    for (let column: number = 1; column <= shorterLength; column += 1) {
      const substitutionCost: number =
        longerCodeUnit === shorter.charCodeAt(column - 1) ? 0 : 1;
      currentRow[column] = Math.min(
        previousRow[column] + 1,
        currentRow[column - 1] + 1,
        previousRow[column - 1] + substitutionCost,
      );
    }

    const completedRow: number[] = previousRow;
    previousRow = currentRow;
    currentRow = completedRow;
  }

  return previousRow[shorterLength];
}

// Small local assertion helpers keep the self-tests dependency-free.
function assertEqual(actual: number, expected: number, description: string): void {
  if (actual !== expected) {
    throw new Error(
      `${description}: expected ${String(expected)}, received ${String(actual)}`,
    );
  }
}

function runSelfTests(): void {
  assertEqual(editDistance("same", "same"), 0, "equal strings");
  assertEqual(editDistance("", ""), 0, "two empty strings");
  assertEqual(editDistance("", "abc"), 3, "empty first string");
  assertEqual(editDistance("wxyz", ""), 4, "empty second string");
  assertEqual(editDistance("kitten", "sitting"), 3, "kitten to sitting");
  assertEqual(editDistance("ab", "ba"), 2, "transposition needs two edits");
  assertEqual(editDistance("aaaaa", "aaa"), 2, "repeated characters");
  assertEqual(editDistance("😀", ""), 2, "supplementary character has two UTF-16 units");
  console.log("Edit-distance self-tests passed.");
}

runSelfTests();
