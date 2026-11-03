#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

/*
 * Two Sum API.
 *
 * Returns true and writes two distinct, zero-based indices to result when
 * nums[i] + nums[j] == target. Each input position is considered once, and
 * the current value is looked up before it is inserted; this allows equal
 * values at different positions to form a pair without reusing an index.
 *
 * Returns false for a null nums pointer, an empty array, a null result
 * pointer, no solution, or an allocation/size failure. If result is non-null,
 * its indices are initialized to zero before any other validation.
 *
 * Expected complexity: O(n) time and O(n) auxiliary storage, assuming the
 * usual expected constant-time behavior of open addressing.
 */
typedef struct {
    size_t first_index;
    size_t second_index;
} TwoSumResult;

typedef struct {
    int key;
    size_t index;
    bool occupied;
} HashEntry;

static size_t hash_int(int value)
{
    uint32_t x = (uint32_t)value;

    x ^= x >> 16;
    x *= UINT32_C(0x7feb352d);
    x ^= x >> 15;
    x *= UINT32_C(0x846ca68b);
    x ^= x >> 16;
    return (size_t)x;
}

/*
 * Find a key in a table whose capacity is a power of two and whose load is
 * at most one half. The table is therefore guaranteed to contain an empty
 * slot, so this probe loop terminates.
 */
static bool find_key(const HashEntry *table, size_t capacity, int key,
                     size_t *index_out)
{
    const size_t mask = capacity - 1;
    size_t slot = hash_int(key) & mask;

    while (table[slot].occupied) {
        if (table[slot].key == key) {
            *index_out = table[slot].index;
            return true;
        }
        slot = (slot + 1) & mask;
    }
    return false;
}

static void insert_key(HashEntry *table, size_t capacity, int key,
                       size_t index)
{
    const size_t mask = capacity - 1;
    size_t slot = hash_int(key) & mask;

    while (table[slot].occupied) {
        slot = (slot + 1) & mask;
    }
    table[slot].key = key;
    table[slot].index = index;
    table[slot].occupied = true;
}

bool two_sum(const int *nums, size_t length, int target, TwoSumResult *result)
{
    size_t capacity = 8;
    size_t needed;
    HashEntry *table;

    if (result != NULL) {
        result->first_index = 0;
        result->second_index = 0;
    }
    if (nums == NULL || length == 0 || result == NULL) {
        return false;
    }

    if (length > SIZE_MAX / 2) {
        return false;
    }
    needed = length * 2;
    while (capacity < needed) {
        if (capacity > SIZE_MAX / 2) {
            return false;
        }
        capacity *= 2;
    }
    if (capacity > SIZE_MAX / sizeof(*table)) {
        return false;
    }

    table = calloc(capacity, sizeof(*table));
    if (table == NULL) {
        return false;
    }

    for (size_t i = 0; i < length; ++i) {
        /* Widen before subtracting to avoid signed int overflow. */
        const int64_t complement = (int64_t)target - (int64_t)nums[i];

        if (complement >= INT_MIN && complement <= INT_MAX) {
            size_t prior_index;

            if (find_key(table, capacity, (int)complement, &prior_index)) {
                result->first_index = prior_index;
                result->second_index = i;
                free(table);
                return true;
            }
        }
        insert_key(table, capacity, nums[i], i);
    }

    free(table);
    return false;
}

static void test_pair(const int *nums, size_t length, int target,
                      size_t expected_first, size_t expected_second)
{
    TwoSumResult result;

    assert(two_sum(nums, length, target, &result));
    assert(result.first_index == expected_first);
    assert(result.second_index == expected_second);
    assert(result.first_index != result.second_index);
    assert((int64_t)nums[result.first_index] +
               (int64_t)nums[result.second_index] ==
           (int64_t)target);
}

static void test_no_solution(const int *nums, size_t length, int target)
{
    TwoSumResult result = {SIZE_MAX, SIZE_MAX};

    assert(!two_sum(nums, length, target, &result));
    assert(result.first_index == 0);
    assert(result.second_index == 0);
}

int main(void)
{
    const int ordinary[] = {2, 7, 11, 15};
    const int negatives[] = {-8, -3, 4, 10};
    const int duplicates[] = {3, 3};
    const int no_pair[] = {1, 2, 4};
    const int extremes[] = {INT_MIN, INT_MAX, 1};

    test_pair(ordinary, sizeof(ordinary) / sizeof(ordinary[0]), 9, 0, 1);
    test_pair(negatives, sizeof(negatives) / sizeof(negatives[0]), -11, 0, 1);
    test_pair(duplicates, sizeof(duplicates) / sizeof(duplicates[0]), 6, 0, 1);
    test_pair(extremes, sizeof(extremes) / sizeof(extremes[0]), -1, 0, 1);
    test_no_solution(no_pair, sizeof(no_pair) / sizeof(no_pair[0]), 100);
    test_no_solution(NULL, 0, 0);
    test_no_solution(NULL, 3, 0);

    {
        const int one[] = {1};
        assert(!two_sum(one, sizeof(one) / sizeof(one[0]), 2, NULL));
    }

    puts("two_sum: all self-tests passed");
    return 0;
}
