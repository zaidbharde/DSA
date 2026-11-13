/**
 * Return the largest XOR of any pair of unsigned 32-bit integers.
 *
 * Every input must be an integer in [0, 0xffffffff]. Invalid values throw a
 * RangeError. Fewer than two values also throw a RangeError because no pair
 * exists. The input array is read-only and is not modified.
 *
 * Complexity: O(32n) = O(n) time and O(32n) = O(n) auxiliary space for n
 * values. Each value is validated, inserted into, and/or queried against a
 * 32-level bitwise trie.
 */
export function maximumXorPair(values: readonly number[]): number {
  for (const value of values) {
    if (
      !Number.isInteger(value) ||
      value < 0 ||
      value > 0xffffffff
    ) {
      throw new RangeError(
        `Each value must be an unsigned 32-bit integer; received ${String(value)}.`,
      );
    }
  }
  if (values.length < 2) {
    throw new RangeError("At least two values are required to form a pair.");
  }

  const root = new BitTrieNode();
  insert(root, values[0] as number);
  let maximum = 0;
  for (let index = 1; index < values.length; index += 1) {
    const value = values[index] as number;
    maximum = Math.max(maximum, largestXorWith(root, value));
    insert(root, value);
  }
  return maximum;
}

/** Trie node for one prefix of 32-bit values. */
class BitTrieNode {
  readonly children: [BitTrieNode | undefined, BitTrieNode | undefined] = [
    undefined,
    undefined,
  ];
}

function insert(root: BitTrieNode, value: number): void {
  let node = root;
  for (let bit = 31; bit >= 0; bit -= 1) {
    const branch = (value >>> bit) & 1;
    let child = node.children[branch];
    if (child === undefined) {
      child = new BitTrieNode();
      node.children[branch] = child;
    }
    node = child;
  }
}

/** Return the greatest XOR obtainable between value and a trie value. */
function largestXorWith(root: BitTrieNode, value: number): number {
  let node = root;
  let result = 0;
  for (let bit = 31; bit >= 0; bit -= 1) {
    const valueBit = (value >>> bit) & 1;
    const oppositeBranch = valueBit ^ 1;
    const oppositeChild = node.children[oppositeBranch];
    const chosenBranch = oppositeChild === undefined ? valueBit : oppositeBranch;
    node = node.children[chosenBranch] as BitTrieNode;
    // Arithmetic accumulation keeps the unsigned high bit out of signed
    // 32-bit result semantics while inspecting each trie level.
    result = result * 2 + (chosenBranch === valueBit ? 0 : 1);
  }
  return result;
}

function assertEqual(actual: number, expected: number, description: string): void {
  if (actual !== expected) {
    throw new Error(
      `${description}: expected ${String(expected)}, received ${String(actual)}`,
    );
  }
}

function assertThrowsRangeError(
  action: () => number,
  description: string,
): void {
  let threwRangeError = false;
  try {
    action();
  } catch (error) {
    threwRangeError = error instanceof RangeError;
  }
  if (!threwRangeError) {
    throw new Error(`${description}: expected a RangeError`);
  }
}

function bruteForceMaximumXor(values: readonly number[]): number {
  let maximum = 0;
  for (let first = 0; first < values.length; first += 1) {
    for (let second = first + 1; second < values.length; second += 1) {
      maximum = Math.max(maximum, (values[first]! ^ values[second]!) >>> 0);
    }
  }
  return maximum >>> 0;
}

function runSelfTests(): void {
  assertEqual(maximumXorPair([3, 10, 5, 25, 2, 8]), 28, "standard example");
  assertEqual(maximumXorPair([7, 7]), 0, "duplicate values");
  assertEqual(maximumXorPair([0, 0, 0]), 0, "all zeros");
  assertEqual(maximumXorPair([0, 0xffffffff]), 0xffffffff, "full unsigned range");
  assertEqual(maximumXorPair([0, 0x80000000]), 0x80000000, "top bit set");
  assertEqual(
    maximumXorPair([0xffffffff, 0x7fffffff, 0x80000000]),
    bruteForceMaximumXor([0xffffffff, 0x7fffffff, 0x80000000]),
    "brute force comparison with high bits",
  );

  // Deterministic varied inputs exercise branch choices at every bit level.
  let state = 0x12345678;
  for (let sample = 0; sample < 40; sample += 1) {
    const values: number[] = [];
    const length = 2 + (sample % 12);
    for (let index = 0; index < length; index += 1) {
      state = (Math.imul(state, 1664525) + 1013904223) >>> 0;
      values.push(state);
    }
    assertEqual(
      maximumXorPair(values),
      bruteForceMaximumXor(values),
      `deterministic brute force comparison ${sample}`,
    );
  }

  assertThrowsRangeError(() => maximumXorPair([]), "empty input");
  assertThrowsRangeError(() => maximumXorPair([1]), "single-value input");
  for (const invalid of [-1, 0x100000000, 1.5, Number.NaN, Number.POSITIVE_INFINITY]) {
    assertThrowsRangeError(
      () => maximumXorPair([0, invalid]),
      `invalid value ${String(invalid)}`,
    );
  }
  console.log("maximum_xor_pair.ts: all tests passed");
}

runSelfTests();
