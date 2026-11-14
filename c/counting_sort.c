/*
 * Stable-range counting sort for signed int arrays, sorted in place.
 *
 * counting_sort(NULL, 0) is valid; a NULL array with nonzero length is an
 * invalid argument. For nonempty input the implementation determines the
 * value range, checks range and allocation arithmetic, and allocates the full
 * frequency table before changing the input. Thus invalid arguments, range
 * overflow, and allocation failure leave the input unchanged. Negative values
 * are supported without signed subtraction or negation overflow.
 *
 * Time: O(n + R), where n is the number of elements and
 * R = max(array) - min(array) + 1. Auxiliary space: O(R) for the frequency
 * table. As with all counting sort implementations, very wide value ranges
 * can require impractical time and memory even for small n.
 */
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum CountingSortStatus {
    COUNTING_SORT_OK = 0,
    COUNTING_SORT_INVALID_ARGUMENT,
    COUNTING_SORT_ALLOCATION_FAILURE,
    COUNTING_SORT_RANGE_OVERFLOW
} CountingSortStatus;

typedef void *(*CountingSortAllocate)(size_t bytes, void *context);
typedef void (*CountingSortDeallocate)(void *memory, void *context);

typedef struct CountingSortAllocator {
    CountingSortAllocate allocate;
    CountingSortDeallocate deallocate;
    void *context;
} CountingSortAllocator;

CountingSortStatus counting_sort(int *array, size_t length);
CountingSortStatus counting_sort_with_allocator(
    int *array, size_t length, const CountingSortAllocator *allocator);

static size_t counting_sort_index(int value, int minimum)
{
    /* Unsigned conversion/subtraction computes the nonnegative distance
       without overflowing even when minimum is INT_MIN. */
    return (size_t)((uintmax_t)value - (uintmax_t)minimum);
}

CountingSortStatus counting_sort_with_allocator(
    int *array, size_t length, const CountingSortAllocator *allocator)
{
    int minimum;
    int maximum;
    uintmax_t distance;
    size_t range;
    size_t index;
    size_t *counts;
    size_t output;
    int value;

    if (array == NULL) {
        return length == 0U ? COUNTING_SORT_OK
                            : COUNTING_SORT_INVALID_ARGUMENT;
    }
    if (length < 2U) {
        return COUNTING_SORT_OK;
    }
    if (allocator == NULL || allocator->allocate == NULL ||
        allocator->deallocate == NULL) {
        return COUNTING_SORT_INVALID_ARGUMENT;
    }

    minimum = array[0];
    maximum = array[0];
    for (index = 1U; index < length; ++index) {
        if (array[index] < minimum) {
            minimum = array[index];
        }
        if (array[index] > maximum) {
            maximum = array[index];
        }
    }

    /* Unsigned subtraction is modulo UINTMAX_MAX+1. The mathematical
       max-min distance fits uintmax_t for int values, including full-range
       INT_MIN..INT_MAX on conforming implementations. */
    distance = (uintmax_t)maximum - (uintmax_t)minimum;
    if (distance >= (uintmax_t)SIZE_MAX) {
        return COUNTING_SORT_RANGE_OVERFLOW;
    }
    range = (size_t)distance + 1U;
    if (range > SIZE_MAX / sizeof(*counts)) {
        return COUNTING_SORT_RANGE_OVERFLOW;
    }

    counts = (size_t *)allocator->allocate(range * sizeof(*counts),
                                             allocator->context);
    if (counts == NULL) {
        return COUNTING_SORT_ALLOCATION_FAILURE;
    }
    (void)memset(counts, 0, range * sizeof(*counts));

    for (index = 0U; index < length; ++index) {
        ++counts[counting_sort_index(array[index], minimum)];
    }

    output = 0U;
    value = minimum;
    for (index = 0U; index < range; ++index) {
        size_t occurrences = counts[index];
        while (occurrences > 0U) {
            array[output] = value;
            ++output;
            --occurrences;
        }
        /* Increment only when another in-range bucket remains, so this never
           attempts to evaluate INT_MAX + 1. */
        if (index < range - 1U) {
            ++value;
        }
    }

    allocator->deallocate(counts, allocator->context);
    return COUNTING_SORT_OK;
}

static void *counting_sort_default_allocate(size_t bytes, void *context)
{
    (void)context;
    return malloc(bytes);
}

static void counting_sort_default_deallocate(void *memory, void *context)
{
    (void)context;
    free(memory);
}

CountingSortStatus counting_sort(int *array, size_t length)
{
    static const CountingSortAllocator allocator = {
        counting_sort_default_allocate,
        counting_sort_default_deallocate,
        NULL
    };
    return counting_sort_with_allocator(array, length, &allocator);
}

#ifndef COUNTING_SORT_NO_MAIN
static int check_case(const char *name, const int *input, size_t length,
                      const int *expected)
{
    int actual[16];
    if (length > sizeof actual / sizeof actual[0]) {
        (void)fprintf(stderr, "Test case too large: %s\n", name);
        return 0;
    }
    if (length > 0U) {
        (void)memcpy(actual, input, length * sizeof actual[0]);
    }
    if (counting_sort(length > 0U ? actual : NULL, length) != COUNTING_SORT_OK) {
        (void)fprintf(stderr, "FAIL: %s returned an error\n", name);
        return 0;
    }
    if (length > 0U &&
        memcmp(actual, expected, length * sizeof actual[0]) != 0) {
        (void)fprintf(stderr, "FAIL: %s produced incorrect order\n", name);
        return 0;
    }
    (void)printf("PASS: %s\n", name);
    return 1;
}

static void *always_fail_allocate(size_t bytes, void *context)
{
    size_t *requested_bytes = (size_t *)context;
    *requested_bytes = bytes;
    return NULL;
}

static void unused_deallocate(void *memory, void *context)
{
    (void)memory;
    (void)context;
}

int main(void)
{
    const int singleton[] = {-7};
    const int sorted[] = {-4, -1, 0, 3, 9};
    const int reverse[] = {9, 3, 0, -1, -4};
    const int expected_ordered[] = {-4, -1, 0, 3, 9};
    const int duplicates[] = {5, -2, 5, 3, -2, 5, 0};
    const int expected_duplicates[] = {-2, -2, 0, 3, 5, 5, 5};
    const int extremes[] = {INT_MAX, INT_MAX - 2, INT_MAX};
    const int expected_extremes[] = {INT_MAX - 2, INT_MAX, INT_MAX};
    const int min_extremes[] = {INT_MIN, INT_MIN + 2, INT_MIN};
    const int expected_min_extremes[] = {INT_MIN, INT_MIN, INT_MIN + 2};
    int failure_input[] = {INT_MIN, 0, INT_MAX};
    const int failure_original[] = {INT_MIN, 0, INT_MAX};
    int empty_sentinel = 17;
    size_t requested_bytes = 0U;
    int failures = 0;
    CountingSortAllocator failing_allocator = {
        always_fail_allocate, unused_deallocate, &requested_bytes
    };

    if (counting_sort(NULL, 0U) != COUNTING_SORT_OK ||
        counting_sort(NULL, 4U) != COUNTING_SORT_INVALID_ARGUMENT ||
        counting_sort(&empty_sentinel, 0U) != COUNTING_SORT_OK ||
        empty_sentinel != 17) {
        (void)fprintf(stderr, "FAIL: empty and invalid-pointer behavior\n");
        ++failures;
    } else {
        (void)puts("PASS: empty and invalid-pointer behavior");
    }
    failures += !check_case("singleton", singleton, 1U, singleton);
    failures += !check_case("already sorted", sorted, 5U, expected_ordered);
    failures += !check_case("reverse sorted", reverse, 5U, expected_ordered);
    failures += !check_case("duplicates and negatives", duplicates, 7U,
                            expected_duplicates);
    failures += !check_case("INT_MAX boundary values", extremes, 3U,
                            expected_extremes);
    failures += !check_case("INT_MIN boundary values", min_extremes, 3U,
                            expected_min_extremes);

    {
        const CountingSortStatus status = counting_sort_with_allocator(
            failure_input, 3U, &failing_allocator);
        if ((status != COUNTING_SORT_ALLOCATION_FAILURE &&
             status != COUNTING_SORT_RANGE_OVERFLOW) ||
            (status == COUNTING_SORT_ALLOCATION_FAILURE &&
             requested_bytes == 0U) ||
            (status == COUNTING_SORT_RANGE_OVERFLOW &&
             requested_bytes != 0U) ||
        memcmp(failure_input, failure_original, sizeof failure_input) != 0) {
            (void)fprintf(stderr, "FAIL: wide-range failure preserves input\n");
            ++failures;
        } else {
            (void)puts("PASS: wide-range allocation/range failure preserves input");
        }
    }

    if (counting_sort_with_allocator(failure_input, 3U, NULL) !=
        COUNTING_SORT_INVALID_ARGUMENT) {
        (void)fprintf(stderr, "FAIL: invalid allocator behavior\n");
        ++failures;
    } else {
        (void)puts("PASS: invalid allocator behavior");
    }

    if (failures != 0) {
        (void)fprintf(stderr, "%d self-test(s) failed\n", failures);
        return 1;
    }
    (void)puts("All Counting sort self-tests passed.");
    return 0;
}
#endif
