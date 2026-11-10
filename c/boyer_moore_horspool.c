#include <stddef.h>
#include <stdint.h>

/*
 * Search a byte sequence for the first occurrence of a byte pattern.
 *
 * `text` and `pattern` must point to readable arrays of `text_len` and
 * `pattern_len` bytes respectively. A NULL pointer is allowed only when its
 * corresponding length is zero. An empty pattern matches at offset 0. Invalid
 * NULL/length combinations and no match both return SIZE_MAX; otherwise the
 * result is the first matching byte offset in `text`.
 *
 * Preprocessing takes O(m + 256) time and uses an O(256) size_t shift table.
 * The search has worst-case O((n - m + 1) * m) time (for 1 <= m <= n), so
 * total worst-case time is O(m + 256 + (n - m + 1) * m). The algorithm is
 * Boyer–Moore–Horspool over the full 256-value byte alphabet.
 */
size_t boyer_moore_horspool_search(const unsigned char *text, size_t text_len,
                                   const unsigned char *pattern,
                                   size_t pattern_len)
{
    size_t shift[256];
    size_t i;
    size_t last_start;

    if ((text == NULL && text_len != 0U) ||
        (pattern == NULL && pattern_len != 0U)) {
        return SIZE_MAX;
    }

    if (pattern_len == 0U) {
        return 0U;
    }
    if (pattern_len > text_len) {
        return SIZE_MAX;
    }

    for (i = 0U; i < 256U; ++i) {
        shift[i] = pattern_len;
    }
    for (i = 0U; i < pattern_len - 1U; ++i) {
        shift[pattern[i]] = pattern_len - 1U - i;
    }

    /* Subtraction avoids forming text_len - pattern_len + 1. */
    last_start = text_len - pattern_len;
    i = 0U;
    while (i <= last_start) {
        size_t j = pattern_len;

        while (j > 0U && pattern[j - 1U] == text[i + j - 1U]) {
            --j;
        }
        if (j == 0U) {
            return i;
        }

        /* i <= text_len - pattern_len and shift <= pattern_len, so this
         * addition cannot exceed text_len (and therefore cannot overflow). */
        i += shift[text[i + pattern_len - 1U]];
    }

    return SIZE_MAX;
}

#ifdef BOYER_MOORE_HORSPOOL_TEST
#include <stdio.h>

#define CHECK_SEARCH(label, expected, text, text_len, pattern, pattern_len)      \
    do {                                                                         \
        size_t actual_ = boyer_moore_horspool_search(                             \
            (text), (text_len), (pattern), (pattern_len));                        \
        size_t expected_ = (expected);                                           \
        if (actual_ != expected_) {                                              \
            fprintf(stderr, "%s: expected %zu, got %zu\n", (label),           \
                    expected_, actual_);                                         \
            return 1;                                                            \
        }                                                                        \
    } while (0)

int main(void)
{
    static const unsigned char overlap_text[] = "aaaaa";
    static const unsigned char overlap_pattern[] = "aaa";
    static const unsigned char repeated_text[] = "bbbbab";
    static const unsigned char repeated_pattern[] = "bab";
    static const unsigned char absent_text[] = "abcdefgh";
    static const unsigned char absent_pattern[] = "xyz";
    static const unsigned char long_pattern[] = "abcdefghi";
    static const unsigned char nul_text[] = { 'x', 0U, 'a', 0U, 'b', 'y' };
    static const unsigned char nul_pattern[] = { 0U, 'a', 0U, 'b' };
    static const unsigned char nul_absent_pattern[] = { 'a', 0U, 'b', 0U };
    static const unsigned char later_text[] = "zzabcabc";
    static const unsigned char later_pattern[] = "abc";

    CHECK_SEARCH("overlapping matches", 0U, overlap_text,
                 sizeof overlap_text - 1U, overlap_pattern,
                 sizeof overlap_pattern - 1U);
    CHECK_SEARCH("repeated bytes", 3U, repeated_text,
                 sizeof repeated_text - 1U, repeated_pattern,
                 sizeof repeated_pattern - 1U);
    CHECK_SEARCH("later first match", 2U, later_text,
                 sizeof later_text - 1U, later_pattern,
                 sizeof later_pattern - 1U);
    CHECK_SEARCH("absent pattern", SIZE_MAX, absent_text,
                 sizeof absent_text - 1U, absent_pattern,
                 sizeof absent_pattern - 1U);
    CHECK_SEARCH("pattern longer than text", SIZE_MAX, absent_text,
                 sizeof absent_text - 1U, long_pattern,
                 sizeof long_pattern - 1U);
    CHECK_SEARCH("empty pattern", 0U, absent_text,
                 sizeof absent_text - 1U, NULL, 0U);
    CHECK_SEARCH("empty text and pattern", 0U, NULL, 0U, NULL, 0U);
    CHECK_SEARCH("empty text with nonempty pattern", SIZE_MAX, NULL, 0U,
                 absent_pattern, sizeof absent_pattern - 1U);
    CHECK_SEARCH("invalid NULL text", SIZE_MAX, NULL, 1U,
                 absent_pattern, sizeof absent_pattern - 1U);
    CHECK_SEARCH("invalid NULL pattern", SIZE_MAX, absent_text,
                 sizeof absent_text - 1U, NULL, 1U);
    CHECK_SEARCH("embedded NUL match", 1U, nul_text, sizeof nul_text,
                 nul_pattern, sizeof nul_pattern);
    CHECK_SEARCH("embedded NUL absent", SIZE_MAX, nul_text, sizeof nul_text,
                 nul_absent_pattern, sizeof nul_absent_pattern);

    puts("boyer_moore_horspool: all self-tests passed");
    return 0;
}
#endif
