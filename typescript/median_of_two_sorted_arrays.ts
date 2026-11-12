/**
 * Return the median of two nondecreasing arrays of finite numbers.
 *
 * Both arrays must already be sorted in ascending order; inputs are not
 * scanned for sortedness so the logarithmic time bound is preserved. Throws
 * RangeError when both arrays are empty. For an even total length, the result
 * is the arithmetic mean of the two middle values.
 *
 * Complexity: O(log(min(m, n))) time and O(1) auxiliary space, where m and n
 * are the input lengths. The arrays are only referenced, never copied.
 */
export function medianOfTwoSortedArrays(
  first: readonly number[],
  second: readonly number[],
): number {
  if (first.length === 0 && second.length === 0) {
    throw new RangeError("The median is undefined for two empty arrays.");
  }

  // Binary-search the shorter input to minimize the number of iterations.
  let shorter: readonly number[] = first;
  let longer: readonly number[] = second;
  if (shorter.length > longer.length) {
    shorter = second;
    longer = first;
  }

  const shorterLength: number = shorter.length;
  const longerLength: number = longer.length;
  const leftPartitionSize: number = Math.floor(
    (shorterLength + longerLength + 1) / 2,
  );
  let low: number = 0;
  let high: number = shorterLength;

  while (low <= high) {
    const shorterPartition: number = Math.floor((low + high) / 2);
    const longerPartition: number = leftPartitionSize - shorterPartition;

    const shorterLeft: number | undefined =
      shorterPartition === 0 ? undefined : shorter[shorterPartition - 1];
    const shorterRight: number | undefined =
      shorterPartition === shorterLength ? undefined : shorter[shorterPartition];
    const longerLeft: number | undefined =
      longerPartition === 0 ? undefined : longer[longerPartition - 1];
    const longerRight: number | undefined =
      longerPartition === longerLength ? undefined : longer[longerPartition];

    if (
      shorterLeft !== undefined &&
      longerRight !== undefined &&
      shorterLeft > longerRight
    ) {
      high = shorterPartition - 1;
    } else if (
      longerLeft !== undefined &&
      shorterRight !== undefined &&
      longerLeft > shorterRight
    ) {
      low = shorterPartition + 1;
    } else {
      const leftMaximum: number =
        shorterLeft === undefined
          ? (longerLeft as number)
          : longerLeft === undefined
            ? shorterLeft
            : Math.max(shorterLeft, longerLeft);

      if ((shorterLength + longerLength) % 2 === 1) {
        return leftMaximum;
      }

      const rightMinimum: number =
        shorterRight === undefined
          ? (longerRight as number)
          : longerRight === undefined
            ? shorterRight
            : Math.min(shorterRight, longerRight);

      // Avoid overflow for same-sign large operands and avoid unnecessary
      // underflow for same-sign tiny operands; opposite-sign addition is safe.
      return (leftMaximum < 0) !== (rightMinimum < 0)
        ? (leftMaximum + rightMinimum) / 2
        : leftMaximum + (rightMinimum - leftMaximum) / 2;
    }
  }

  // Sorted inputs always admit a valid partition; this is defensive only.
  throw new Error("Unable to find a valid partition; inputs may not be sorted.");
}

function assertEqual(actual: number, expected: number, description: string): void {
  if (actual !== expected) {
    throw new Error(
      `${description}: expected ${String(expected)}, received ${String(actual)}`,
    );
  }
}

function assertThrowsRangeError(action: () => number, description: string): void {
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

function runSelfTests(): void {
  assertEqual(medianOfTwoSortedArrays([1, 3], [2]), 2, "odd total length");
  assertEqual(medianOfTwoSortedArrays([1, 2], [3, 4]), 2.5, "even total length");
  assertEqual(medianOfTwoSortedArrays([], [5]), 5, "first input empty");
  assertEqual(medianOfTwoSortedArrays([-5, -3], []), -4, "second input empty");
  assertEqual(medianOfTwoSortedArrays([-10, -2], [-7, -4, 1]), -4, "negative values");
  assertEqual(medianOfTwoSortedArrays([1, 1], [1, 1, 1]), 1, "duplicate values");
  assertEqual(medianOfTwoSortedArrays([1, 2, 3, 4, 5, 6], [7]), 4, "unequal array lengths");
  assertEqual(
    medianOfTwoSortedArrays([-Number.MAX_VALUE], [Number.MAX_VALUE]),
    0,
    "opposite extreme finite values",
  );
  assertEqual(
    medianOfTwoSortedArrays([Number.MAX_VALUE], [Number.MAX_VALUE]),
    Number.MAX_VALUE,
    "same-sign extreme finite values without overflow",
  );
  assertEqual(
    medianOfTwoSortedArrays([Number.MIN_VALUE], [Number.MIN_VALUE]),
    Number.MIN_VALUE,
    "smallest positive finite values without underflow",
  );
  assertThrowsRangeError(
    () => medianOfTwoSortedArrays([], []),
    "both inputs empty",
  );
  console.log("median_of_two_sorted_arrays.ts: all tests passed");
}

runSelfTests();
