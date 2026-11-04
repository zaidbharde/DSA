#include <stddef.h>
#include <stdio.h>
#include <stdint.h>

/*
 * Result returned by longest_unique_substring().
 *
 * `length` is the number of bytes in the longest contiguous substring with
 * no repeated byte, and `start` is its zero-based offset in the input.
 * When the input is empty or NULL, both fields are zero. If multiple longest
 * substrings exist, the earliest start offset is returned.
 */
typedef struct {
    size_t length;
    size_t start;
} longest_unique_substring_result;

/*
 * Find the longest contiguous byte substring without repeated bytes.
 *
 * A NULL input is treated as empty regardless of `length`; a non-NULL input
 * with length zero is also empty. In either case the result is {0, 0}.
 * The input is interpreted as `length` bytes, including any NUL or
 * non-ASCII bytes. The fixed 256-entry last-seen table gives O(n) time and
 * O(1) auxiliary space with respect to input length.
 */
longest_unique_substring_result
longest_unique_substring(const unsigned char *input, size_t length)
{
    size_t last_seen[256];
    size_t window_start = 0U;
    longest_unique_substring_result result = {0U, 0U};

    if (input == NULL || length == 0U) {
        return result;
    }

    for (size_t byte = 0U; byte < 256U; ++byte) {
        last_seen[byte] = SIZE_MAX;
    }

    for (size_t i = 0U; i < length; ++i) {
        const unsigned char byte = input[i];
        const size_t previous = last_seen[byte];

        if (previous != SIZE_MAX && previous >= window_start) {
            window_start = previous + 1U;
        }
        last_seen[byte] = i;

        {
            const size_t window_length = i - window_start + 1U;
            if (window_length > result.length) {
                result.length = window_length;
                result.start = window_start;
            }
        }
    }

    return result;
}

#ifdef LONGEST_UNIQUE_SUBSTRING_SELF_TEST
static int
expect_result(const char *name,
              const unsigned char *input,
              size_t input_length,
              size_t expected_length,
              size_t expected_start)
{
    const longest_unique_substring_result result =
        longest_unique_substring(input, input_length);

    if (result.length != expected_length || result.start != expected_start) {
        (void)fprintf(stderr,
                      "FAIL: %s: got {length=%zu, start=%zu}, expected "
                      "{length=%zu, start=%zu}\n",
                      name,
                      result.length,
                      result.start,
                      expected_length,
                      expected_start);
        return 0;
    }

    (void)printf("PASS: %s\n", name);
    return 1;
}

int
main(void)
{
    const unsigned char repeated[] = "bbbbb";
    const unsigned char tie[] = "abcabcbb";
    const unsigned char high_bytes[] = {0x80U, 0xffU, 0x80U, 0x81U};
    const unsigned char empty[] = "";
    unsigned char all_bytes[256];
    int passed = 1;

    for (size_t i = 0U; i < sizeof all_bytes; ++i) {
        all_bytes[i] = (unsigned char)i;
    }

    passed &= expect_result("repeated characters", repeated,
                            sizeof repeated - 1U, 1U, 0U);
    passed &= expect_result("all unique byte values", all_bytes,
                            sizeof all_bytes, 256U, 0U);
    passed &= expect_result("empty input", empty, 0U, 0U, 0U);
    passed &= expect_result("NULL input", NULL, 5U, 0U, 0U);
    passed &= expect_result("earliest tied substring", tie,
                            sizeof tie - 1U, 3U, 0U);
    passed &= expect_result("bytes above ASCII", high_bytes,
                            sizeof high_bytes, 3U, 1U);

    if (!passed) {
        return 1;
    }

    (void)puts("All self-tests passed.");
    return 0;
}
#endif
