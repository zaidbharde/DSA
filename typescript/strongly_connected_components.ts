/**
 * Returns the strongly connected components of a directed graph with vertices
 * numbered from `0` through `vertexCount - 1`.
 *
 * Determinism: vertices are started in ascending order, outgoing edges are
 * visited in their input order, and SCCs are emitted when their DFS roots
 * finish. Members within an SCC are returned in Tarjan stack-pop order; the
 * result is a partition, not a sorted or topologically ordered listing.
 *
 * Throws `RangeError` if `vertexCount` is not a nonnegative integer or if an
 * edge endpoint is not an integer in `[0, vertexCount)`. All edge endpoints
 * are validated before traversal begins.
 *
 * Time: O(V + E), where V is `vertexCount` and E is the number of edges.
 * Storage: O(V + E) total, including adjacency lists, Tarjan recursion/state,
 * and the returned components.
 */
export function stronglyConnectedComponents(
  vertexCount: number,
  edges: readonly (readonly [from: number, to: number])[],
): number[][] {
  if (!Number.isInteger(vertexCount) || vertexCount < 0) {
    throw new RangeError("vertexCount must be a nonnegative integer");
  }

  const adjacency: number[][] = Array.from(
    { length: vertexCount },
    (): number[] => [],
  );

  for (const [from, to] of edges) {
    if (
      !Number.isInteger(from) ||
      from < 0 ||
      from >= vertexCount ||
      !Number.isInteger(to) ||
      to < 0 ||
      to >= vertexCount
    ) {
      throw new RangeError(
        `edge endpoints must be integers in [0, ${vertexCount})`,
      );
    }
    adjacency[from].push(to);
  }

  const discoveryIndex: number[] = new Array<number>(vertexCount).fill(-1);
  const lowLink: number[] = new Array<number>(vertexCount).fill(-1);
  const onStack: boolean[] = new Array<boolean>(vertexCount).fill(false);
  const stack: number[] = [];
  const components: number[][] = [];
  let nextIndex: number = 0;

  function visit(vertex: number): void {
    discoveryIndex[vertex] = nextIndex;
    lowLink[vertex] = nextIndex;
    nextIndex += 1;
    stack.push(vertex);
    onStack[vertex] = true;

    for (const neighbor of adjacency[vertex]) {
      if (discoveryIndex[neighbor] === -1) {
        visit(neighbor);
        lowLink[vertex] = Math.min(lowLink[vertex], lowLink[neighbor]);
      } else if (onStack[neighbor]) {
        lowLink[vertex] = Math.min(lowLink[vertex], discoveryIndex[neighbor]);
      }
    }

    if (lowLink[vertex] === discoveryIndex[vertex]) {
      const component: number[] = [];
      let member: number;
      do {
        member = stack.pop() as number;
        onStack[member] = false;
        component.push(member);
      } while (member !== vertex);
      components.push(component);
    }
  }

  for (let vertex: number = 0; vertex < vertexCount; vertex += 1) {
    if (discoveryIndex[vertex] === -1) {
      visit(vertex);
    }
  }

  return components;
}

interface TestCase {
  readonly name: string;
  readonly vertexCount: number;
  readonly edges: readonly (readonly [from: number, to: number])[];
  readonly expected: readonly (readonly number[])[];
}

const testCases: readonly TestCase[] = [
  { name: "empty graph", vertexCount: 0, edges: [], expected: [] },
  {
    name: "disconnected graph",
    vertexCount: 4,
    edges: [[0, 1], [2, 3]],
    expected: [[1], [0], [3], [2]],
  },
  {
    name: "one SCC",
    vertexCount: 3,
    edges: [[0, 1], [1, 2], [2, 0]],
    expected: [[2, 1, 0]],
  },
  {
    name: "multiple SCCs with cross edges",
    vertexCount: 5,
    edges: [[0, 1], [1, 0], [1, 2], [2, 3], [3, 2], [0, 4], [3, 4]],
    expected: [[4], [3, 2], [1, 0]],
  },
  {
    name: "self-loops",
    vertexCount: 2,
    edges: [[0, 0], [0, 1], [1, 1]],
    expected: [[1], [0]],
  },
  {
    name: "DAG",
    vertexCount: 4,
    edges: [[0, 1], [0, 2], [1, 3], [2, 3]],
    expected: [[3], [1], [2], [0]],
  },
];

function assertEqual(
  name: string,
  actual: unknown,
  expected: unknown,
): void {
  if (JSON.stringify(actual) !== JSON.stringify(expected)) {
    throw new Error(
      `${name}: expected ${JSON.stringify(expected)}, received ${JSON.stringify(actual)}`,
    );
  }
}

function assertRangeError(name: string, operation: () => unknown): void {
  let threwRangeError: boolean = false;
  try {
    operation();
  } catch (error: unknown) {
    threwRangeError = error instanceof RangeError;
  }
  if (!threwRangeError) {
    throw new Error(`${name}: expected a RangeError`);
  }
}

function runSelfTests(): void {
  for (const testCase of testCases) {
    const actual: number[][] = stronglyConnectedComponents(
      testCase.vertexCount,
      testCase.edges,
    );
    assertEqual(testCase.name, actual, testCase.expected);
  }

  assertRangeError("negative vertex count", (): number[][] =>
    stronglyConnectedComponents(-1, []),
  );
  assertRangeError("fractional vertex count", (): number[][] =>
    stronglyConnectedComponents(1.5, []),
  );
  assertRangeError("negative endpoint", (): number[][] =>
    stronglyConnectedComponents(2, [[-1, 0]]),
  );
  assertRangeError("out-of-range endpoint", (): number[][] =>
    stronglyConnectedComponents(2, [[0, 2]]),
  );
  assertRangeError("fractional endpoint", (): number[][] =>
    stronglyConnectedComponents(2, [[0, 1.5]]),
  );
}

runSelfTests();
