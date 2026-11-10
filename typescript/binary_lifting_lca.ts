/**
 * Lowest common ancestor queries for a rooted, undirected tree.
 *
 * Vertices are numbered 0 through vertexCount - 1. The constructor requires a
 * positive safe-integer vertex count, exactly vertexCount - 1 two-vertex edges,
 * valid distinct endpoints, and a valid root. Cyclic or disconnected inputs are
 * rejected with RangeError. Construction validates and preprocesses iteratively,
 * so even highly skewed trees do not use the JavaScript call stack.
 *
 * Construction takes O(n log n) time and storage after O(n) input validation,
 * where n is the vertex count. lca(u, v) validates both vertices and returns
 * their lowest common ancestor in O(log n) time. Inputs and queries use
 * zero-based integer vertex IDs; invalid values throw RangeError.
 */
export class BinaryLiftingLCA {
  private readonly size: number;
  private readonly depth: number[];
  private readonly ancestors: number[][];

  /**
   * Build an LCA index for an undirected tree.
   * @param vertexCount Positive number of vertices, identified by [0, n).
   * @param edges Exactly n - 1 pairs [u, v] describing undirected edges.
   * @param root Vertex from which ancestor relationships are rooted (default 0).
   * @throws RangeError If the count, root, edges, or tree structure is invalid.
   */
  constructor(
    vertexCount: number,
    edges: readonly (readonly [number, number])[],
    root = 0,
  ) {
    if (!Number.isSafeInteger(vertexCount) || vertexCount < 1) {
      throw new RangeError("vertexCount must be a positive safe integer.");
    }
    this.validateVertex(root, vertexCount, "root");
    if (!Array.isArray(edges) || edges.length !== vertexCount - 1) {
      throw new RangeError(`edges must contain exactly ${vertexCount - 1} entries.`);
    }

    const adjacency: number[][] = Array.from({ length: vertexCount }, () => []);
    const disjointSet = Array.from({ length: vertexCount }, (_, vertex) => vertex);
    const componentSize = new Array<number>(vertexCount).fill(1);

    const find = (vertex: number): number => {
      let representative = vertex;
      while (disjointSet[representative] !== representative) {
        representative = disjointSet[representative];
      }
      let current = vertex;
      while (disjointSet[current] !== current) {
        const next = disjointSet[current];
        disjointSet[current] = representative;
        current = next;
      }
      return representative;
    };

    for (let index = 0; index < edges.length; index += 1) {
      const edge: unknown = edges[index];
      if (!Array.isArray(edge) || edge.length !== 2) {
        throw new RangeError(`Edge ${index} must contain exactly two vertices.`);
      }
      const first: unknown = edge[0];
      const second: unknown = edge[1];
      this.validateVertex(first, vertexCount, `edge ${index} endpoint`);
      this.validateVertex(second, vertexCount, `edge ${index} endpoint`);
      if (first === second) {
        throw new RangeError(`Edge ${index} must not be a self-loop.`);
      }

      const firstRoot = find(first);
      const secondRoot = find(second);
      if (firstRoot === secondRoot) {
        throw new RangeError("edges contain a cycle.");
      }
      if (componentSize[firstRoot] < componentSize[secondRoot]) {
        disjointSet[firstRoot] = secondRoot;
        componentSize[secondRoot] += componentSize[firstRoot];
      } else {
        disjointSet[secondRoot] = firstRoot;
        componentSize[firstRoot] += componentSize[secondRoot];
      }
      adjacency[first].push(second);
      adjacency[second].push(first);
    }

    const representative = find(0);
    for (let vertex = 1; vertex < vertexCount; vertex += 1) {
      if (find(vertex) !== representative) {
        throw new RangeError("edges do not connect all vertices.");
      }
    }

    this.size = vertexCount;
    this.depth = new Array<number>(vertexCount).fill(0);
    const parent = new Array<number>(vertexCount).fill(root);
    const visited = new Array<boolean>(vertexCount).fill(false);
    const stack = [root];
    visited[root] = true;

    while (stack.length > 0) {
      const vertex = stack.pop() as number;
      for (const neighbor of adjacency[vertex]) {
        if (visited[neighbor]) {
          continue;
        }
        visited[neighbor] = true;
        parent[neighbor] = vertex;
        this.depth[neighbor] = this.depth[vertex] + 1;
        stack.push(neighbor);
      }
    }

    const levels = Math.floor(Math.log2(vertexCount)) + 1;
    this.ancestors = new Array<number[]>(levels);
    this.ancestors[0] = parent;
    for (let level = 1; level < levels; level += 1) {
      const previous = this.ancestors[level - 1];
      const current = new Array<number>(vertexCount);
      for (let vertex = 0; vertex < vertexCount; vertex += 1) {
        current[vertex] = previous[previous[vertex]];
      }
      this.ancestors[level] = current;
    }
  }

  /** Number of vertices in the indexed tree. */
  get vertexCount(): number {
    return this.size;
  }

  /**
   * Return the lowest common ancestor of u and v in the tree rooted at the
   * constructor's root. Both vertices must be integers in [0, vertexCount).
   */
  lca(u: number, v: number): number {
    this.validateVertex(u, this.size, "u");
    this.validateVertex(v, this.size, "v");

    let first = u;
    let second = v;
    if (this.depth[first] < this.depth[second]) {
      [first, second] = [second, first];
    }

    let difference = this.depth[first] - this.depth[second];
    let level = 0;
    while (difference > 0) {
      if (difference % 2 === 1) {
        first = this.ancestors[level][first];
      }
      difference = Math.floor(difference / 2);
      level += 1;
    }

    if (first === second) {
      return first;
    }

    for (let bit = this.ancestors.length - 1; bit >= 0; bit -= 1) {
      if (this.ancestors[bit][first] !== this.ancestors[bit][second]) {
        first = this.ancestors[bit][first];
        second = this.ancestors[bit][second];
      }
    }
    return this.ancestors[0][first];
  }

  private validateVertex(vertex: unknown, vertexCount: number, name: string): asserts vertex is number {
    if (
      typeof vertex !== "number" ||
      !Number.isSafeInteger(vertex) ||
      vertex < 0 ||
      vertex >= vertexCount
    ) {
      throw new RangeError(`${name} must be an integer in [0, ${vertexCount}).`);
    }
  }
}

// Dependency-free executable tests; compile to ES2020 CommonJS and run with Node.js.
function assert(condition: unknown, message: string): asserts condition {
  if (!condition) {
    throw new Error(`Assertion failed: ${message}`);
  }
}

function assertEqual(actual: number, expected: number, message: string): void {
  assert(actual === expected, `${message}; expected ${expected}, got ${actual}`);
}

function assertThrows(action: () => void, message: string): void {
  let threw = false;
  try {
    action();
  } catch (error) {
    threw = error instanceof RangeError;
  }
  assert(threw, `${message}; expected RangeError`);
}

function referenceLca(
  adjacency: readonly (readonly number[])[],
  root: number,
  first: number,
  second: number,
): number {
  const parent = new Array<number>(adjacency.length).fill(root);
  const depth = new Array<number>(adjacency.length).fill(0);
  const stack = [root];
  const seen = new Array<boolean>(adjacency.length).fill(false);
  seen[root] = true;
  while (stack.length > 0) {
    const vertex = stack.pop() as number;
    for (const neighbor of adjacency[vertex]) {
      if (!seen[neighbor]) {
        seen[neighbor] = true;
        parent[neighbor] = vertex;
        depth[neighbor] = depth[vertex] + 1;
        stack.push(neighbor);
      }
    }
  }
  let a = first;
  let b = second;
  while (depth[a] > depth[b]) {
    a = parent[a];
  }
  while (depth[b] > depth[a]) {
    b = parent[b];
  }
  while (a !== b) {
    a = parent[a];
    b = parent[b];
  }
  return a;
}

function runSelfTests(): void {
  const chain = new BinaryLiftingLCA(7, [[0, 1], [1, 2], [2, 3], [3, 4], [4, 5], [5, 6]]);
  assertEqual(chain.lca(2, 6), 2, "chain ancestor");
  assertEqual(chain.lca(3, 4), 3, "chain parent and child");
  assertEqual(chain.lca(5, 5), 5, "identical chain vertices");
  assertEqual(chain.lca(0, 6), 0, "chain root and deepest vertex");
  assertEqual(chain.vertexCount, 7, "vertex count getter");

  const branching = new BinaryLiftingLCA(
    9,
    [[0, 1], [0, 2], [1, 3], [1, 4], [2, 5], [2, 6], [5, 7], [5, 8]],
  );
  assertEqual(branching.lca(3, 4), 1, "siblings in first branch");
  assertEqual(branching.lca(3, 7), 0, "vertices in separate branches");
  assertEqual(branching.lca(7, 8), 5, "siblings below an internal node");
  assertEqual(branching.lca(2, 8), 2, "ancestor and descendant in branch");

  const rerooted = new BinaryLiftingLCA(5, [[0, 1], [1, 2], [1, 3], [3, 4]], 4);
  assertEqual(rerooted.lca(2, 0), 1, "nonzero root query");
  assertEqual(rerooted.lca(4, 2), 4, "constructor root is an ancestor");

  const singleton = new BinaryLiftingLCA(1, []);
  assertEqual(singleton.vertexCount, 1, "singleton vertex count");
  assertEqual(singleton.lca(0, 0), 0, "singleton query");

  assertThrows(() => new BinaryLiftingLCA(0, []), "zero vertices");
  assertThrows(() => new BinaryLiftingLCA(-1, []), "negative vertex count");
  assertThrows(() => new BinaryLiftingLCA(2.5, []), "fractional vertex count");
  assertThrows(() => new BinaryLiftingLCA(Number.POSITIVE_INFINITY, []), "infinite vertex count");
  assertThrows(() => new BinaryLiftingLCA(2, [[0, 1]], -1), "negative root");
  assertThrows(() => new BinaryLiftingLCA(2, [[0, 1]], 2), "root past end");
  assertThrows(() => new BinaryLiftingLCA(2, [[0, 1]], 0.5), "fractional root");
  assertThrows(() => new BinaryLiftingLCA(3, [[0, 1]]), "too few edges");
  assertThrows(() => new BinaryLiftingLCA(2, [[0, 1], [0, 1]]), "too many edges");
  assertThrows(() => new BinaryLiftingLCA(2, [[0, 0]]), "self-loop");
  assertThrows(() => new BinaryLiftingLCA(3, [[0, 1], [1, 3]]), "endpoint past end");
  assertThrows(() => new BinaryLiftingLCA(3, [[0, 1], [1, 1.5]]), "fractional endpoint");
  assertThrows(
    () => new BinaryLiftingLCA(4, [[0, 1], [1, 2], [2, 0]]),
    "cycle and disconnected vertex",
  );
  assertThrows(() => new BinaryLiftingLCA(3, [[0, 1], [0, 1]]), "parallel-edge cycle");
  assertThrows(() => chain.lca(-1, 2), "negative query vertex");
  assertThrows(() => chain.lca(1, 7), "query vertex past end");
  assertThrows(() => chain.lca(1.5, 2), "fractional query vertex");

  let seed = 0x12345678;
  const nextRandom = (bound: number): number => {
    seed = (Math.imul(seed, 1664525) + 1013904223) >>> 0;
    return seed % bound;
  };
  for (let trial = 0; trial < 20; trial += 1) {
    const count = 1 + nextRandom(35);
    const edges: [number, number][] = [];
    const adjacency: number[][] = Array.from({ length: count }, () => []);
    for (let vertex = 1; vertex < count; vertex += 1) {
      const parent = nextRandom(vertex);
      edges.push([parent, vertex]);
      adjacency[parent].push(vertex);
      adjacency[vertex].push(parent);
    }
    const root = nextRandom(count);
    const index = new BinaryLiftingLCA(count, edges, root);
    for (let first = 0; first < count; first += 1) {
      for (let second = 0; second < count; second += 1) {
        assertEqual(
          index.lca(first, second),
          referenceLca(adjacency, root, first, second),
          `random tree ${trial} query (${first}, ${second})`,
        );
      }
    }
  }

  // Exercise an iterative traversal on a deep chain without risking recursion.
  const deepCount = 20_000;
  const deepEdges: [number, number][] = [];
  for (let vertex = 1; vertex < deepCount; vertex += 1) {
    deepEdges.push([vertex - 1, vertex]);
  }
  const deepChain = new BinaryLiftingLCA(deepCount, deepEdges);
  assertEqual(deepChain.lca(19_000, 19_999), 19_000, "deep-chain stack safety");

  console.log("binary_lifting_lca.ts: all tests passed");
}

runSelfTests();
