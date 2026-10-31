#include <inttypes.h>
#include <stddef.h>
#include <stdio.h>

/*
 * Reverse array[first..last], inclusive. The caller supplies a non-empty,
 * valid range within the array.
 */
static void reverse_range(int *array, size_t first, size_t last)
{
    while (first < last) {
        const int temporary = array[first];
        array[first] = array[last];
        array[last] = temporary;
        ++first;
        --last;
    }
}

/*
 * Rotate array left by k positions in place.
 *
 * Negative k is normalized as a right rotation: for example, k == -1 moves
 * the last element to the front. Its magnitude is computed without negating
 * k directly, so LLONG_MIN is safe. Empty arrays and null pointers are no-ops.
 *
 * Three reversals perform the rotation in O(n) time with O(1) extra space.
 */
static void rotate_left(int *array, size_t n, long long k)
{
    uintmax_t shift;
    const uintmax_t length = (uintmax_t)n;

    if (array == NULL || n < 2U) {
        return;
    }

    if (k < 0) {
        const uintmax_t magnitude = (uintmax_t)(-(k + 1LL)) + UINTMAX_C(1);
        const uintmax_t right_shift = magnitude % length;
        shift = (right_shift == 0U) ? 0U : length - right_shift;
    } else {
        shift = (uintmax_t)k % length;
    }

    if (shift == 0U) {
        return;
    }

    {
        const size_t split = (size_t)shift;
        reverse_range(array, 0U, split - 1U);
        reverse_range(array, split, n - 1U);
        reverse_range(array, 0U, n - 1U);
    }
}

static int array_matches(const int *actual, const int *expected, size_t n)
{
    size_t index;

    for (index = 0U; index < n; ++index) {
        if (actual[index] != expected[index]) {
            return 0;
        }
    }
    return 1;
}

static int run_self_tests(void)
{
    int ordinary[] = {1, 2, 3, 4, 5};
    const int ordinary_expected[] = {3, 4, 5, 1, 2};
    int wraparound[] = {1, 2, 3, 4, 5};
    const int wraparound_expected[] = {3, 4, 5, 1, 2};
    int zero[] = {7, 8, 9};
    const int zero_expected[] = {7, 8, 9};
    int negative[] = {1, 2, 3, 4, 5};
    const int negative_expected[] = {5, 1, 2, 3, 4};
    int empty_sentinel = 42;

    rotate_left(ordinary, sizeof ordinary / sizeof ordinary[0], 2);
    rotate_left(wraparound, sizeof wraparound / sizeof wraparound[0], 7);
    rotate_left(zero, sizeof zero / sizeof zero[0], 0);
    rotate_left(negative, sizeof negative / sizeof negative[0], -1);
    rotate_left(&empty_sentinel, 0U, 3);

    if (!array_matches(ordinary, ordinary_expected,
                       sizeof ordinary / sizeof ordinary[0])) {
        (void)fprintf(stderr, "FAIL: ordinary rotation\n");
        return 0;
    }
    if (!array_matches(wraparound, wraparound_expected,
                       sizeof wraparound / sizeof wraparound[0])) {
        (void)fprintf(stderr, "FAIL: wraparound rotation\n");
        return 0;
    }
    if (!array_matches(zero, zero_expected, sizeof zero / sizeof zero[0])) {
        (void)fprintf(stderr, "FAIL: zero rotation\n");
        return 0;
    }
    if (!array_matches(negative, negative_expected,
                       sizeof negative / sizeof negative[0])) {
        (void)fprintf(stderr, "FAIL: negative rotation normalization\n");
        return 0;
    }
    if (empty_sentinel != 42) {
        (void)fprintf(stderr, "FAIL: empty rotation\n");
        return 0;
    }

    (void)puts("All array left-rotation tests passed.");
    return 1;
}

int main(void)
{
    return run_self_tests() ? 0 : 1;
}
