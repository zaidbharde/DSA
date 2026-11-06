#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>

/*
 * Sort `length` integers in ascending order, in place.
 *
 * A NULL pointer or an array with fewer than two elements is a no-op.
 *
 * Average time: O(n log n); worst-case time: O(n^2).
 * Auxiliary space: O(log n) for recursion. The implementation uses
 * three-way partitioning and recursively processes the smaller partition,
 * keeping the call stack logarithmic even when partitions are unbalanced.
 */
void quick_sort(int *array, size_t length);

static int median_of_three(int first, int middle, int last)
{
    if (first < middle) {
        if (middle < last) {
            return middle;
        }
        return first < last ? last : first;
    }

    if (first < last) {
        return first;
    }
    return middle < last ? last : middle;
}

static void quick_sort_range(int *array, size_t low, size_t high)
{
    while (high - low > 1U) {
        const size_t middle = low + (high - low) / 2U;
        const int pivot = median_of_three(array[low], array[middle],
                                          array[high - 1U]);
        size_t less = low;
        size_t current = low;
        size_t greater = high;

        /* Partition [low, high) into values < pivot, == pivot, and > pivot. */
        while (current < greater) {
            if (array[current] < pivot) {
                const int temporary = array[less];
                array[less] = array[current];
                array[current] = temporary;
                ++less;
                ++current;
            } else if (array[current] > pivot) {
                --greater;
                {
                    const int temporary = array[current];
                    array[current] = array[greater];
                    array[greater] = temporary;
                }
            } else {
                ++current;
            }
        }

        /* Recurse only into the smaller side; iterate over the larger side. */
        if (less - low < high - greater) {
            quick_sort_range(array, low, less);
            low = greater;
        } else {
            quick_sort_range(array, greater, high);
            high = less;
        }
    }
}

void quick_sort(int *array, size_t length)
{
    if (array == NULL || length < 2U) {
        return;
    }

    quick_sort_range(array, 0U, length);
}

#ifndef QUICK_SORT_NO_MAIN
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
    quick_sort(length > 0U ? actual : NULL, length);

    if (length > 0U && memcmp(actual, expected, length * sizeof actual[0]) != 0) {
        (void)fprintf(stderr, "FAIL: %s\n", name);
        return 0;
    }

    (void)printf("PASS: %s\n", name);
    return 1;
}

int main(void)
{
    const int singleton[] = {42};
    const int sorted[] = {-4, -1, 0, 3, 9};
    const int reverse[] = {9, 3, 0, -1, -4};
    const int expected_ordered[] = {-4, -1, 0, 3, 9};
    const int duplicates[] = {5, -2, 5, 3, -2, 5, 0};
    const int expected_duplicates[] = {-2, -2, 0, 3, 5, 5, 5};
    const int all_equal[] = {7, 7, 7, 7, 7, 7};
    const int extremes[] = {INT_MAX, 0, INT_MIN, INT_MAX, INT_MIN};
    const int expected_extremes[] = {INT_MIN, INT_MIN, 0, INT_MAX, INT_MAX};
    int empty_sentinel = 17;
    int failures = 0;

    quick_sort(NULL, 0U);
    quick_sort(NULL, 4U);
    quick_sort(&empty_sentinel, 0U);
    if (empty_sentinel != 17) {
        (void)fprintf(stderr, "FAIL: NULL/empty no-op behavior\n");
        ++failures;
    } else {
        (void)printf("PASS: NULL/empty no-op behavior\n");
    }

    failures += !check_case("singleton", singleton, 1U, singleton);
    failures += !check_case("already sorted", sorted, 5U, expected_ordered);
    failures += !check_case("reverse sorted", reverse, 5U, expected_ordered);
    failures += !check_case("duplicates", duplicates, 7U, expected_duplicates);
    failures += !check_case("all equal", all_equal, 6U, all_equal);
    failures += !check_case("INT_MIN/INT_MAX", extremes, 5U, expected_extremes);

    if (failures != 0) {
        (void)fprintf(stderr, "%d self-test(s) failed\n", failures);
        return 1;
    }

    (void)puts("All self-tests passed.");
    return 0;
}
#endif
