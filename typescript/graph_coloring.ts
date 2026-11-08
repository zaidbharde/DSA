/**
 * Determine whether a simple undirected graph has a proper coloring using at
 * most `colorCount` colors. The adjacency matrix must be square and symmetric;
 * entries may be booleans or numeric 0/1 values. A self-loop is valid input but
 * makes a proper coloring impossible, so it returns false.
 *
 * Vertices are assigned in ascending index order, and candidate colors are
 * tried in ascending order. Invalid matrix structure/entries throw TypeError;
 * a negative or non-integer color count throws RangeError.
 *
 * Time: O(m^V * V^2) worst case, where V is the vertex count and m is
 * `colorCount` (including O(V^2) validation). Space: O(V^2 + V), including the
 * normalized adjacency matrix and O(V) color/recursion state.
 */
export function graphColoring(
  adjacencyMatrix: readonly (readonly (boolean | number)[])[],
  colorCount: number,
): boolean {
  if (!Number.isInteger(colorCount) || colorCount < 0) {
    throw new RangeError("colorCount must be a nonnegative integer");
  }
  if (!Array.isArray(adjacencyMatrix)) {
    throw new TypeError("adjacencyMatrix must be an array of rows");
  }

  const vertexCount: number = adjacencyMatrix.length;
  const adjacent: boolean[][] = new Array<boolean[]>(vertexCount);

  for (let vertex: number = 0; vertex < vertexCount; vertex += 1) {
    const row: unknown = adjacencyMatrix[vertex];
    if (!Array.isArray(row) || row.length !== vertexCount) {
      throw new TypeError("adjacencyMatrix must be square");
    }

    adjacent[vertex] = new Array<boolean>(vertexCount);
    for (let neighbor: number = 0; neighbor < vertexCount; neighbor += 1) {
      const entry: unknown = row[neighbor];
      if (
        entry !== true &&
        entry !== false &&
        entry !== 0 &&
        entry !== 1
      ) {
        throw new TypeError("adjacencyMatrix entries must be booleans or 0/1");
      }
      adjacent[vertex][neighbor] = entry === true || entry === 1;
    }
  }

  for (let vertex: number = 0; vertex < vertexCount; vertex += 1) {
    for (let neighbor: number = vertex + 1; neighbor < vertexCount; neighbor += 1) {
      if (adjacent[vertex][neighbor] !== adjacent[neighbor][vertex]) {
        throw new TypeError("adjacencyMatrix must be symmetric");
      }
    }
  }

  for (let vertex: number = 0; vertex < vertexCount; vertex += 1) {
    if (adjacent[vertex][vertex]) {
      return false;
    }
  }

  if (vertexCount === 0) {
    return true;
  }
  if (colorCount === 0) {
    return false;
  }

  const colors: number[] = new Array<number>(vertexCount).fill(-1);

  function assignColor(vertex: number): boolean {
    if (vertex === vertexCount) {
      return true;
    }

    for (let color: number = 0; color < colorCount; color += 1) {
      let valid: boolean = true;
      for (let previous: number = 0; previous < vertex; previous += 1) {
        if (adjacent[vertex][previous] && colors[previous] === color) {
          valid = false;
          break;
        }
      }

      if (valid) {
        colors[vertex] = color;
        if (assignColor(vertex + 1)) {
          return true;
        }
        colors[vertex] = -1;
      }
    }

    return false;
  }

  return assignColor(0);
}

function assert(condition: unknown, message: string): asserts condition {
  if (!condition) {
    throw new Error(`Assertion failed: ${message}`);
  }
}

function assertEqual(actual: boolean, expected: boolean, message: string): void {
  assert(actual === expected, `${message}; expected ${expected}, got ${actual}`);
}

function assertThrows(
  action: () => void,
  expectedError: typeof TypeError | typeof RangeError,
  message: string,
): void {
  let threwExpectedError: boolean = false;
  try {
    action();
  } catch (error: unknown) {
    threwExpectedError = error instanceof expectedError;
  }
  assert(threwExpectedError, `${message}; expected ${expectedError.name}`);
}

function runSelfTests(): void {
  assertEqual(graphColoring([], 0), true, "empty graph with no colors");
  assertEqual(graphColoring([[0, 1], [1, 0]], 2), true, "single-edge path");
  assertEqual(
    graphColoring([[false, true, false], [1, false, 1], [0, 1, false]], 2),
    true,
    "path with boolean and numeric matrix entries",
  );
  assertEqual(
    graphColoring(
      [
        [0, 1, 0, 1],
        [1, 0, 1, 0],
        [0, 1, 0, 1],
        [1, 0, 1, 0],
      ],
      2,
    ),
    true,
    "even cycle with two colors",
  );
  assertEqual(
    graphColoring([[0, 1, 1], [1, 0, 1], [1, 1, 0]], 2),
    false,
    "odd cycle with two colors",
  );
  assertEqual(
    graphColoring([[0, 1, 1], [1, 0, 1], [1, 1, 0]], 3),
    true,
    "odd cycle with three colors",
  );
  assertEqual(
    graphColoring(
      [
        [0, 1, 1, 1],
        [1, 0, 1, 1],
        [1, 1, 0, 1],
        [1, 1, 1, 0],
      ],
      3,
    ),
    false,
    "complete graph with insufficient colors",
  );
  assertEqual(
    graphColoring(
      [
        [0, 1, 0, 0, 0],
        [1, 0, 0, 0, 0],
        [0, 0, 0, 1, 1],
        [0, 0, 1, 0, 1],
        [0, 0, 1, 1, 0],
      ],
      3,
    ),
    true,
    "disconnected graph",
  );
  assertEqual(graphColoring([[1]], 1), false, "self-loop");
  assertEqual(graphColoring([[0, 1], [1, 0]], 0), false, "nonempty graph with zero colors");

  assertThrows(
    () => graphColoring([[0, 1], [1]], 2),
    TypeError,
    "ragged matrix",
  );
  assertThrows(
    () => graphColoring([[0, 1], [0, 0]], 2),
    TypeError,
    "asymmetric matrix",
  );
  assertThrows(
    () => graphColoring([[0, 2], [2, 0]], 2),
    TypeError,
    "invalid matrix entries",
  );
  assertThrows(
    () => graphColoring([[0, false], [true, 0]], 2),
    TypeError,
    "asymmetric boolean/numeric entries",
  );
  assertThrows(
    () => graphColoring([[1, 1], [0, 0]], 2),
    TypeError,
    "asymmetric matrix with a self-loop",
  );
  assertThrows(
    () => graphColoring([[0]], -1),
    RangeError,
    "negative color count",
  );
  assertThrows(
    () => graphColoring([[0]], 1.5),
    RangeError,
    "fractional color count",
  );
  assertThrows(
    () => graphColoring([[0]], Number.NaN),
    RangeError,
    "NaN color count",
  );

  console.log("graph_coloring.ts: all tests passed");
}

runSelfTests();
