#include <stddef.h>
#include <stdint.h>

/*
 * Return the first byte offset where pattern occurs in text, or SIZE_MAX if
 * there is no match or the arguments are invalid. An empty pattern matches at
 * offset 0. A pointer may be NULL only when its corresponding length is zero.
 * For example, a NULL text pointer with text_length == 0 is valid; it can match
 * an empty pattern but cannot contain a non-empty one. Since SIZE_MAX is both
 * the not-found result and the invalid-argument result, callers that need to
 * distinguish those cases must validate their pointers before calling.
 *
 * The rolling hash is modulo 1,000,000,007 with base 257. Hash-equal windows
 * are always compared byte-for-byte, so collisions cannot produce false
 * matches. Expected time is O(n + m) under the usual hashing assumption;
 * collision checks make the worst-case time O(nm). Auxiliary space is O(1).
 */
#define RK_MODULUS UINT64_C(1000000007)
#define RK_BASE UINT64_C(257)

size_t rabin_karp_search(const unsigned char *text, size_t text_length,
                         const unsigned char *pattern,
                         size_t pattern_length)
{
    uint64_t pattern_hash = 0;
    uint64_t window_hash = 0;
    uint64_t highest_power = 1;
    size_t i;
    size_t last_start;

    if ((text == NULL && text_length != 0U) ||
        (pattern == NULL && pattern_length != 0U)) {
        return SIZE_MAX;
    }
    if (pattern_length == 0U) {
        return 0U;
    }
    if (pattern_length > text_length) {
        return SIZE_MAX;
    }

    for (i = 0U; i < pattern_length; ++i) {
        pattern_hash =
            (pattern_hash * RK_BASE + (uint64_t)pattern[i] + UINT64_C(1)) %
            RK_MODULUS;
        window_hash =
            (window_hash * RK_BASE + (uint64_t)text[i] + UINT64_C(1)) %
            RK_MODULUS;
        if (i + 1U < pattern_length) {
            highest_power = (highest_power * RK_BASE) % RK_MODULUS;
        }
    }

    last_start = text_length - pattern_length;
    for (i = 0U;; ++i) {
        if (window_hash == pattern_hash) {
            size_t j = 0U;
            while (j < pattern_length && text[i + j] == pattern[j]) {
                ++j;
            }
            if (j == pattern_length) {
                return i;
            }
        }

        if (i == last_start) {
            break;
        }

        /*
         * Remove the leading byte modulo RK_MODULUS, then append the next
         * byte. Each product is bounded by (RK_MODULUS - 1) * 257, which
         * fits comfortably in uint64_t; additions are also kept in range.
         */
        {
            uint64_t leading =
                ((uint64_t)text[i] + UINT64_C(1)) * highest_power %
                RK_MODULUS;
            uint64_t remaining =
                (window_hash + RK_MODULUS - leading) % RK_MODULUS;
            window_hash =
                (remaining * RK_BASE + (uint64_t)text[i + pattern_length] +
                 UINT64_C(1)) %
                RK_MODULUS;
        }
    }

    return SIZE_MAX;
}

#ifdef RABIN_KARP_TEST
#include <assert.h>

int main(void)
{
    static const unsigned char overlap_text[] = "aaaaa";
    static const unsigned char overlap_pattern[] = "aaa";
    static const unsigned char repeated_text[] = "bbbbbbbc";
    static const unsigned char repeated_pattern[] = "bbbbc";
    static const unsigned char ordinary_text[] = "the quick brown fox";
    static const unsigned char absent_pattern[] = "wolf";
    static const unsigned char longer_pattern[] = "a pattern longer than text";
    static const unsigned char nul_text[] = {'x', 0, 'y', 0, 'z'};
    static const unsigned char nul_pattern[] = {0, 'y', 0};
    /* Distinct four-byte strings with the same base-257 hash modulo RK_MODULUS. */
    static const unsigned char collision_text[] = {
        227, 148, 243, 219, 50, 217, 32, 5
    };
    static const unsigned char collision_pattern[] = {50, 217, 32, 5};
    static const unsigned char empty_byte[] = {0};

    assert(rabin_karp_search(overlap_text, sizeof overlap_text - 1U,
                             overlap_pattern, sizeof overlap_pattern - 1U) ==
           0U);
    assert(rabin_karp_search(repeated_text, sizeof repeated_text - 1U,
                             repeated_pattern,
                             sizeof repeated_pattern - 1U) == 3U);
    assert(rabin_karp_search(NULL, 0U, NULL, 0U) == 0U);
    assert(rabin_karp_search(NULL, 0U, empty_byte, 1U) == SIZE_MAX);
    assert(rabin_karp_search(ordinary_text, 0U, empty_byte, 1U) == SIZE_MAX);
    assert(rabin_karp_search(ordinary_text, sizeof ordinary_text - 1U, NULL,
                             0U) == 0U);
    assert(rabin_karp_search(ordinary_text, sizeof ordinary_text - 1U,
                             absent_pattern,
                             sizeof absent_pattern - 1U) == SIZE_MAX);
    assert(rabin_karp_search(ordinary_text, sizeof ordinary_text - 1U,
                             longer_pattern,
                             sizeof longer_pattern - 1U) == SIZE_MAX);
    assert(rabin_karp_search(nul_text, sizeof nul_text, nul_pattern,
                             sizeof nul_pattern) == 1U);
    assert(rabin_karp_search(collision_text, sizeof collision_text,
                             collision_pattern, sizeof collision_pattern) ==
           4U);
    assert(rabin_karp_search(NULL, 1U, empty_byte, 1U) == SIZE_MAX);
    assert(rabin_karp_search(empty_byte, 1U, NULL, 1U) == SIZE_MAX);

    return 0;
}
#endif
