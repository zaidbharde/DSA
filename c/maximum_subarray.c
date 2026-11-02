#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <limits.h>

/*
 * Result for a maximum contiguous subarray. Indices are zero-based and
 * inclusive. Ties are resolved by earliest start, then earliest end.
 */
typedef struct {
    int64_t sum;
    size_t start;
    size_t end;
} maximum_subarray_result;

/*
 * Find the maximum-sum non-empty contiguous subarray in values[0..length).
 * Returns false for a null pointer, zero length, null result, or when the
 * maximum sum cannot be represented as int64_t. On failure, *result is
 * unchanged. Runs in O(n) time and O(1) auxiliary space.
 */
bool maximum_subarray(const int64_t *values, size_t length,
                      maximum_subarray_result *result)
{
    int64_t current_sum;
    size_t current_start = 0;
    int64_t best_sum;
    size_t best_start = 0;
    size_t best_end = 0;
    size_t i;

    if (values == NULL || length == 0 || result == NULL) {
        return false;
    }

    current_sum = values[0];
    best_sum = values[0];

    for (i = 1; i < length; ++i) {
        const int64_t value = values[i];
        int64_t extended_sum;
        size_t extended_start;

        /* A positive overflow proves the true maximum is not representable. */
        if (current_sum > 0 && value > INT64_MAX - current_sum) {
            return false;
        }

        /* A negative underflowed extension is always worse than value alone. */
        if (current_sum < 0 && value < INT64_MIN - current_sum) {
            extended_sum = value;
            extended_start = i;
        } else {
            extended_sum = current_sum + value;
            extended_start = current_start;
            if (extended_sum < value) {
                extended_sum = value;
                extended_start = i;
            }
        }

        current_sum = extended_sum;
        current_start = extended_start;

        if (current_sum > best_sum ||
            (current_sum == best_sum &&
             (current_start < best_start ||
              (current_start == best_start && i < best_end)))) {
            best_sum = current_sum;
            best_start = current_start;
            best_end = i;
        }
    }

    result->sum = best_sum;
    result->start = best_start;
    result->end = best_end;
    return true;
}

#ifdef MAXIMUM_SUBARRAY_TESTING
#include <assert.h>
#include <stdio.h>

static void expect_result(const int64_t *values, size_t length,
                          int64_t expected_sum, size_t expected_start,
                          size_t expected_end)
{
    maximum_subarray_result result = {0, 0, 0};

    assert(maximum_subarray(values, length, &result));
    assert(result.sum == expected_sum);
    assert(result.start == expected_start);
    assert(result.end == expected_end);
}

int main(void)
{
    const int64_t mixed[] = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
    const int64_t all_negative[] = {-8, -3, -6, -2, -5};
    const int64_t single[] = {INT64_MIN};
    const int64_t earliest_start_tie[] = {1, -1, 2};
    const int64_t earliest_end_tie[] = {2, -2, 2};
    const int64_t negative_underflow[] = {INT64_MIN, -1};
    const int64_t largest_representable[] = {INT64_MAX, 0};
    const int64_t positive_overflow[] = {INT64_MAX, 1};
    maximum_subarray_result untouched = {17, 18, 19};

    expect_result(mixed, sizeof(mixed) / sizeof(mixed[0]), 6, 3, 6);
    expect_result(all_negative, sizeof(all_negative) / sizeof(all_negative[0]),
                  -2, 3, 3);
    expect_result(single, sizeof(single) / sizeof(single[0]), INT64_MIN, 0, 0);
    expect_result(earliest_start_tie,
                  sizeof(earliest_start_tie) / sizeof(earliest_start_tie[0]),
                  2, 0, 2);
    expect_result(earliest_end_tie,
                  sizeof(earliest_end_tie) / sizeof(earliest_end_tie[0]),
                  2, 0, 0);
    expect_result(negative_underflow,
                  sizeof(negative_underflow) / sizeof(negative_underflow[0]),
                  -1, 1, 1);
    expect_result(largest_representable,
                  sizeof(largest_representable) / sizeof(largest_representable[0]),
                  INT64_MAX, 0, 0);

    assert(!maximum_subarray(NULL, 1, &untouched));
    assert(!maximum_subarray(mixed, 0, &untouched));
    assert(!maximum_subarray(mixed, sizeof(mixed) / sizeof(mixed[0]), NULL));
    assert(!maximum_subarray(positive_overflow,
                             sizeof(positive_overflow) /
                                 sizeof(positive_overflow[0]),
                             &untouched));
    assert(untouched.sum == 17 && untouched.start == 18 && untouched.end == 19);

    puts("maximum_subarray: all tests passed");
    return 0;
}
#endif
