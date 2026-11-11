#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

/*
 * Generate every prime less than or equal to limit.
 *
 * On PRIME_SIEVE_OK, *primes_out points to a malloc-allocated array of
 * *count_out size_t values; the caller must release it with free(). If there
 * are no primes, *primes_out is NULL and *count_out is zero. On any error,
 * both outputs are reset to NULL and zero. The output pointers themselves
 * must be non-NULL.
 *
 * Returns one of the prime_sieve_status values below. The odd-only sieve
 * takes O(n log log n) time and O(n) space, where n is limit.
 */
typedef enum prime_sieve_status {
    PRIME_SIEVE_OK = 0,
    PRIME_SIEVE_INVALID_ARGUMENT,
    PRIME_SIEVE_ALLOCATION_FAILURE,
    PRIME_SIEVE_SIZE_OVERFLOW
} prime_sieve_status;

prime_sieve_status prime_sieve(size_t limit, size_t **primes_out,
                               size_t *count_out)
{
    bool *composite = NULL;
    size_t *primes = NULL;
    size_t odd_count = 0;
    size_t prime_count = 0;
    size_t index;

    if (primes_out == NULL || count_out == NULL) {
        return PRIME_SIEVE_INVALID_ARGUMENT;
    }
    *primes_out = NULL;
    *count_out = 0;

    /* The sieve stores only odd candidates 3, 5, 7, ... through limit. */
    if (limit >= 3) {
        odd_count = (limit - 1) / 2;
        if (odd_count > SIZE_MAX / sizeof(*composite)) {
            return PRIME_SIEVE_SIZE_OVERFLOW;
        }
        composite = calloc(odd_count, sizeof(*composite));
        if (composite == NULL) {
            return PRIME_SIEVE_ALLOCATION_FAILURE;
        }

        for (index = 0; index < odd_count; ++index) {
            const size_t p = 2 * index + 3;
            size_t mark;

            /* This division check makes p * p safe from overflow. */
            if (p > limit / p) {
                break;
            }
            if (composite[index]) {
                continue;
            }

            mark = (p * p - 3) / 2;
            while (mark < odd_count) {
                composite[mark] = true;
                /* Advance by p odd slots (equivalent to adding 2*p). */
                if (p > odd_count - 1 - mark) {
                    break;
                }
                mark += p;
            }
        }

        for (index = 0; index < odd_count; ++index) {
            if (!composite[index]) {
                ++prime_count;
            }
        }
    }
    if (limit >= 2) {
        ++prime_count;
    }

    if (prime_count != 0) {
        if (prime_count > SIZE_MAX / sizeof(*primes)) {
            free(composite);
            return PRIME_SIEVE_SIZE_OVERFLOW;
        }
        primes = malloc(prime_count * sizeof(*primes));
        if (primes == NULL) {
            free(composite);
            return PRIME_SIEVE_ALLOCATION_FAILURE;
        }

        index = 0;
        if (limit >= 2) {
            primes[index++] = 2;
        }
        for (size_t candidate = 0; candidate < odd_count; ++candidate) {
            if (!composite[candidate]) {
                primes[index++] = 2 * candidate + 3;
            }
        }
    }

    free(composite);
    *primes_out = primes;
    *count_out = prime_count;
    return PRIME_SIEVE_OK;
}

#ifdef PRIME_SIEVE_TEST
#include <assert.h>
#include <stdio.h>

static void check_primes(size_t limit, const size_t *expected,
                         size_t expected_count)
{
    size_t *actual = NULL;
    size_t actual_count = 0;
    size_t index;

    assert(prime_sieve(limit, &actual, &actual_count) == PRIME_SIEVE_OK);
    assert(actual_count == expected_count);
    for (index = 0; index < expected_count; ++index) {
        assert(actual[index] == expected[index]);
    }
    free(actual);
}

int main(void)
{
    static const size_t expected_2[] = {2};
    static const size_t expected_10[] = {2, 3, 5, 7};
    static const size_t expected_100[] = {
        2,  3,  5,  7,  11, 13, 17, 19, 23, 29, 31, 37, 41,
        43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97
    };

    check_primes(0, NULL, 0);
    check_primes(1, NULL, 0);
    check_primes(2, expected_2, sizeof(expected_2) / sizeof(expected_2[0]));
    check_primes(10, expected_10,
                 sizeof(expected_10) / sizeof(expected_10[0]));
    check_primes(100, expected_100,
                 sizeof(expected_100) / sizeof(expected_100[0]));

    puts("prime_sieve: all self-tests passed");
    return 0;
}
#endif
