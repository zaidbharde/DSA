#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

/*
 * Return true and store the strict majority value (> length / 2 occurrences)
 * through majority_out if one exists. The input array is never modified.
 *
 * Return false when majority_out is NULL, when values is NULL (including for
 * a positive length), when the array is empty, or when there is no strict
 * majority. On every false return, a non-NULL majority_out is left unchanged.
 * A non-NULL input pointer must designate at least length readable integers.
 *
 * Complexity: O(n) time and O(1) auxiliary space, where n is length. The
 * Boyer-Moore candidate pass is followed by a verification pass.
 */
bool majority_element(const int *values, size_t length, int *majority_out)
{
    size_t candidate_count = 0;
    int candidate = 0;
    size_t occurrences = 0;

    if (values == NULL || length == 0 || majority_out == NULL) {
        return false;
    }

    for (size_t i = 0; i < length; ++i) {
        if (candidate_count == 0) {
            candidate = values[i];
            candidate_count = 1;
        } else if (values[i] == candidate) {
            ++candidate_count;
        } else {
            --candidate_count;
        }
    }

    for (size_t i = 0; i < length; ++i) {
        if (values[i] == candidate) {
            ++occurrences;
        }
    }

    if (occurrences <= length / 2) {
        return false;
    }

    *majority_out = candidate;
    return true;
}

static void test_majority(const int *values, size_t length, int expected)
{
    int result = 12345;

    assert(majority_element(values, length, &result));
    assert(result == expected);
}

static void test_no_majority(const int *values, size_t length)
{
    int result = 12345;

    assert(!majority_element(values, length, &result));
    assert(result == 12345);
}

int main(void)
{
    const int majority[] = {1, 2, 1, 1, 3, 1, 1};
    const int no_majority[] = {1, 2, 3, 2, 1, 3};
    const int one[] = {-7};
    const int duplicates[] = {4, 4, 9, 4, 9, 4};
    const int negatives[] = {-2, -1, -2, -2, 3};
    const int unchanged[] = {6, 6, 8, 6, 9};
    const int unchanged_copy[] = {6, 6, 8, 6, 9};

    test_majority(majority, sizeof(majority) / sizeof(majority[0]), 1);
    test_no_majority(no_majority,
                     sizeof(no_majority) / sizeof(no_majority[0]));
    test_majority(one, sizeof(one) / sizeof(one[0]), -7);
    test_majority(duplicates, sizeof(duplicates) / sizeof(duplicates[0]), 4);
    test_majority(negatives, sizeof(negatives) / sizeof(negatives[0]), -2);
    test_no_majority(NULL, 0);
    test_no_majority(NULL, 3);

    {
        int result = 99;
        assert(!majority_element(unchanged, 0, &result));
        assert(result == 99);
        assert(!majority_element(unchanged, sizeof(unchanged) / sizeof(unchanged[0]),
                                 NULL));
        assert(!majority_element(NULL, 1, &result));
        assert(result == 99);
    }

    assert(majority_element(unchanged, sizeof(unchanged) / sizeof(unchanged[0]),
                            &(int){0}));
    for (size_t i = 0; i < sizeof(unchanged) / sizeof(unchanged[0]); ++i) {
        assert(unchanged[i] == unchanged_copy[i]);
    }

    puts("majority_element: all self-tests passed");
    return 0;
}
