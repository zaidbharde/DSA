#include <stddef.h>
#include <stdio.h>

/*
 * Sort `length` signed integers in ascending order, in place.
 *
 * A NULL pointer or an array with fewer than two elements is a no-op.
 * The gap sequence starts at floor(length / 2) and repeatedly halves with
 * integer division until reaching 1; a final gap-1 insertion pass completes
 * the sort. With this halving sequence, worst-case time is O(n^2), and
 * auxiliary space is O(1). Values are compared directly, so negative values
 * and the full range of int are supported without arithmetic on their values.
 */
void shell_sort(int *array, size_t length);

void shell_sort(int *array, size_t length)
{
    size_t gap;

    if (array == NULL || length < 2U) {
        return;
    }

    for (gap = length / 2U; gap > 0U; gap /= 2U) {
        size_t index;

        for (index = gap; index < length; ++index) {
            const int value = array[index];
            size_t position = index;

            while (position >= gap && array[position - gap] > value) {
                array[position] = array[position - gap];
                position -= gap;
            }
            array[position] = value;
        }
    }
}

#ifndef SHELL_SORT_NO_MAIN
static int check_case(const char *name, const int *input, size_t length,
                      const int *expected)
{
    int actual[16];
    size_t index;

    if (length > sizeof actual / sizeof actual[0]) {
        (void)fprintf(stderr, "Test case too large: %s\n", name);
        return 0;
    }
    for (index = 0U; index < length; ++index) {
        actual[index] = input[index];
    }

    shell_sort(length > 0U ? actual : NULL, length);
    for (index = 0U; index < length; ++index) {
        if (actual[index] != expected[index]) {
            (void)fprintf(stderr, "FAIL: %s at index %zu\n", name, index);
            return 0;
        }
    }

    (void)printf("PASS: %s\n", name);
    return 1;
}

int main(void)
{
    const int singleton[] = {-7};
    const int sorted[] = {-4, -1, 0, 3, 9};
    const int reverse[] = {9, 3, 0, -1, -4};
    const int expected_ordered[] = {-4, -1, 0, 3, 9};
    const int duplicates[] = {5, -2, 5, 3, -2, 5, 0};
    const int expected_duplicates[] = {-2, -2, 0, 3, 5, 5, 5};
    const int all_equal[] = {7, 7, 7, 7, 7, 7};
    int empty_sentinel = 17;
    int failures = 0;

    shell_sort(NULL, 0U);
    shell_sort(NULL, 4U);
    shell_sort(&empty_sentinel, 0U);
    if (empty_sentinel != 17) {
        (void)fprintf(stderr, "FAIL: NULL/empty no-op behavior\n");
        ++failures;
    } else {
        (void)puts("PASS: NULL/empty no-op behavior");
    }

    failures += !check_case("singleton", singleton, 1U, singleton);
    failures += !check_case("already sorted", sorted, 5U, expected_ordered);
    failures += !check_case("reverse sorted", reverse, 5U, expected_ordered);
    failures += !check_case("duplicates and negatives", duplicates, 7U,
                            expected_duplicates);
    failures += !check_case("all equal", all_equal, 6U, all_equal);

    if (failures != 0) {
        (void)fprintf(stderr, "%d self-test(s) failed\n", failures);
        return 1;
    }

    (void)puts("All Shell sort self-tests passed.");
    return 0;
}
#endif
