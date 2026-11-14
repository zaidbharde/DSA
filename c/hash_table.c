/*
 * Open-addressed int-to-int hash map (linear probing).
 *
 * Ownership/API:
 *   - HashMap owns its allocated slot array. Initialize it with
 *     hash_map_init(), and release it with hash_map_destroy().
 *   - Do not copy an initialized HashMap by value; that would duplicate the
 *     owning pointer. Reinitialize only after destroying it.
 *   - hash_map_put() inserts or updates; when `inserted` is non-NULL it is
 *     set true only for a newly inserted key.
 *   - hash_map_get() writes the value to `out_value` only on success.
 *   - hash_map_remove() reports HASH_MAP_NOT_FOUND if the key is absent.
 *   - A zero initial capacity selects the default capacity. Capacities below
 *     the minimum are raised to the minimum.
 *
 * Status values make invalid arguments, allocation failure, capacity-size
 * overflow, and missing keys distinguishable. All operations expect a map
 * that has been successfully initialized and not yet destroyed.
 *
 * Expected operation time is O(1) with a well-distributed hash and a bounded
 * load factor; worst-case operation time is O(n). Storage is O(capacity).
 * Compile with -DHASH_TABLE_SELF_TEST to enable the self-test main function.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define HASH_MAP_MIN_CAPACITY ((size_t)8)
#define HASH_MAP_DEFAULT_CAPACITY ((size_t)8)

typedef enum HashMapStatus {
    HASH_MAP_OK = 0,
    HASH_MAP_NOT_FOUND,
    HASH_MAP_INVALID_ARGUMENT,
    HASH_MAP_OUT_OF_MEMORY,
    HASH_MAP_CAPACITY_OVERFLOW
} HashMapStatus;

typedef struct HashMapSlot HashMapSlot;

typedef struct HashMap {
    HashMapSlot *slots;
    size_t capacity;
    size_t size;
    size_t tombstones;
} HashMap;

HashMapStatus hash_map_init(HashMap *map, size_t initial_capacity);
void hash_map_destroy(HashMap *map);
HashMapStatus hash_map_put(HashMap *map, int key, int value, bool *inserted);
HashMapStatus hash_map_get(const HashMap *map, int key, int *out_value);
HashMapStatus hash_map_remove(HashMap *map, int key);
size_t hash_map_size(const HashMap *map);

struct HashMapSlot {
    int key;
    int value;
    unsigned char state;
};

enum {
    SLOT_EMPTY = 0,
    SLOT_OCCUPIED = 1,
    SLOT_TOMBSTONE = 2
};

static uint32_t hash_map_hash_int(int key)
{
    /* Conversion to uint32_t is defined modulo 2^32, including negatives. */
    uint32_t value = (uint32_t)key;

    value ^= value >> 16;
    value *= UINT32_C(0x7feb352d);
    value ^= value >> 15;
    value *= UINT32_C(0x846ca68b);
    value ^= value >> 16;
    return value;
}

static bool hash_map_allocation_size_valid(size_t capacity)
{
    return capacity <= SIZE_MAX / sizeof(HashMapSlot);
}

static size_t hash_map_load_limit(size_t capacity)
{
    /* floor(70% * capacity), written without overflowing multiplication. */
    const size_t limit = (capacity / 10U) * 7U + ((capacity % 10U) * 7U) / 10U;
    return limit == 0U ? 1U : limit;
}

static size_t hash_map_index_for(int key, size_t capacity)
{
    return (size_t)hash_map_hash_int(key) % capacity;
}

/* Locate a key, also returning the best slot in which to insert if absent. */
static bool hash_map_probe(const HashMap *map, int key, size_t *found_index,
                           size_t *insert_index)
{
    const size_t start = hash_map_index_for(key, map->capacity);
    size_t index = start;
    size_t first_tombstone = SIZE_MAX;
    size_t offset;

    for (offset = 0U; offset < map->capacity; ++offset) {
        const HashMapSlot *slot = &map->slots[index];

        if (slot->state == SLOT_OCCUPIED) {
            if (slot->key == key) {
                if (found_index != NULL) {
                    *found_index = index;
                }
                return true;
            }
        } else if (slot->state == SLOT_TOMBSTONE) {
            if (first_tombstone == SIZE_MAX) {
                first_tombstone = index;
            }
        } else {
            if (insert_index != NULL) {
                *insert_index = first_tombstone == SIZE_MAX ? index : first_tombstone;
            }
            return false;
        }
        ++index;
        if (index == map->capacity) {
            index = 0U;
        }
    }

    if (insert_index != NULL) {
        *insert_index = first_tombstone;
    }
    return false;
}

static HashMapStatus hash_map_rehash(HashMap *map, size_t new_capacity)
{
    HashMapSlot *new_slots;
    size_t i;

    if (new_capacity < HASH_MAP_MIN_CAPACITY ||
        !hash_map_allocation_size_valid(new_capacity)) {
        return HASH_MAP_CAPACITY_OVERFLOW;
    }

    new_slots = (HashMapSlot *)calloc(new_capacity, sizeof(*new_slots));
    if (new_slots == NULL) {
        return HASH_MAP_OUT_OF_MEMORY;
    }

    for (i = 0U; i < map->capacity; ++i) {
        const HashMapSlot *old_slot = &map->slots[i];
        if (old_slot->state == SLOT_OCCUPIED) {
            size_t index = hash_map_index_for(old_slot->key, new_capacity);
            while (new_slots[index].state == SLOT_OCCUPIED) {
                ++index;
                if (index == new_capacity) {
                    index = 0U;
                }
            }
            new_slots[index] = *old_slot;
        }
    }

    free(map->slots);
    map->slots = new_slots;
    map->capacity = new_capacity;
    map->tombstones = 0U;
    return HASH_MAP_OK;
}

HashMapStatus hash_map_init(HashMap *map, size_t initial_capacity)
{
    size_t capacity = initial_capacity;

    if (map == NULL) {
        return HASH_MAP_INVALID_ARGUMENT;
    }

    map->slots = NULL;
    map->capacity = 0U;
    map->size = 0U;
    map->tombstones = 0U;

    if (capacity == 0U) {
        capacity = HASH_MAP_DEFAULT_CAPACITY;
    } else if (capacity < HASH_MAP_MIN_CAPACITY) {
        capacity = HASH_MAP_MIN_CAPACITY;
    }

    if (!hash_map_allocation_size_valid(capacity)) {
        return HASH_MAP_CAPACITY_OVERFLOW;
    }

    map->slots = (HashMapSlot *)calloc(capacity, sizeof(*map->slots));
    if (map->slots == NULL) {
        return HASH_MAP_OUT_OF_MEMORY;
    }
    map->capacity = capacity;
    return HASH_MAP_OK;
}

void hash_map_destroy(HashMap *map)
{
    if (map != NULL) {
        free(map->slots);
        map->slots = NULL;
        map->capacity = 0U;
        map->size = 0U;
        map->tombstones = 0U;
    }
}

HashMapStatus hash_map_put(HashMap *map, int key, int value, bool *inserted)
{
    size_t index = SIZE_MAX;
    size_t insertion_index = SIZE_MAX;
    size_t used;
    size_t limit;
    bool found;
    HashMapStatus status;

    if (inserted != NULL) {
        *inserted = false;
    }
    if (map == NULL || map->slots == NULL || map->capacity == 0U) {
        return HASH_MAP_INVALID_ARGUMENT;
    }

    found = hash_map_probe(map, key, &index, &insertion_index);
    if (found) {
        map->slots[index].value = value;
        return HASH_MAP_OK;
    }

    limit = hash_map_load_limit(map->capacity);
    if (map->size >= limit) {
        size_t new_capacity;
        if (map->capacity > SIZE_MAX / 2U) {
            return HASH_MAP_CAPACITY_OVERFLOW;
        }
        new_capacity = map->capacity * 2U;
        status = hash_map_rehash(map, new_capacity);
        if (status != HASH_MAP_OK) {
            return status;
        }
        (void)hash_map_probe(map, key, NULL, &index);
    } else {
        index = insertion_index;
        used = map->size + map->tombstones;
        if (index == SIZE_MAX) {
            /* This can occur only for a completely full table. */
            if (map->capacity > SIZE_MAX / 2U) {
                return HASH_MAP_CAPACITY_OVERFLOW;
            }
            status = hash_map_rehash(map, map->capacity * 2U);
            if (status != HASH_MAP_OK) {
                return status;
            }
            (void)hash_map_probe(map, key, NULL, &index);
        } else if (map->slots[index].state == SLOT_EMPTY && used >= limit) {
            /* Compact tombstones without growing when live entries permit it. */
            status = hash_map_rehash(map, map->capacity);
            if (status != HASH_MAP_OK) {
                return status;
            }
            (void)hash_map_probe(map, key, NULL, &index);
        }
    }

    if (index == SIZE_MAX) {
        return HASH_MAP_CAPACITY_OVERFLOW;
    }
    if (map->slots[index].state == SLOT_TOMBSTONE) {
        --map->tombstones;
    }
    map->slots[index].key = key;
    map->slots[index].value = value;
    map->slots[index].state = SLOT_OCCUPIED;
    ++map->size;
    if (inserted != NULL) {
        *inserted = true;
    }
    return HASH_MAP_OK;
}

HashMapStatus hash_map_get(const HashMap *map, int key, int *out_value)
{
    size_t index;

    if (map == NULL || map->slots == NULL || map->capacity == 0U || out_value == NULL) {
        return HASH_MAP_INVALID_ARGUMENT;
    }
    if (!hash_map_probe(map, key, &index, NULL)) {
        return HASH_MAP_NOT_FOUND;
    }
    *out_value = map->slots[index].value;
    return HASH_MAP_OK;
}

HashMapStatus hash_map_remove(HashMap *map, int key)
{
    size_t index;

    if (map == NULL || map->slots == NULL || map->capacity == 0U) {
        return HASH_MAP_INVALID_ARGUMENT;
    }
    if (!hash_map_probe(map, key, &index, NULL)) {
        return HASH_MAP_NOT_FOUND;
    }
    map->slots[index].state = SLOT_TOMBSTONE;
    --map->size;
    ++map->tombstones;
    return HASH_MAP_OK;
}

size_t hash_map_size(const HashMap *map)
{
    return map == NULL || map->slots == NULL ? 0U : map->size;
}

#ifdef HASH_TABLE_SELF_TEST
#include <assert.h>
#include <limits.h>
#include <stdio.h>

static void test_collisions_updates_and_deletion(void)
{
    HashMap map;
    size_t keys[3];
    size_t key;
    size_t count = 0U;
    size_t target_bucket;
    int value;
    bool inserted;

    assert(hash_map_init(&map, 8U) == HASH_MAP_OK);
    target_bucket = hash_map_index_for(0, map.capacity);
    for (key = 0U; key < 10000U && count < 3U; ++key) {
        if (hash_map_index_for((int)key, map.capacity) == target_bucket) {
            keys[count++] = key;
        }
    }
    assert(count == 3U);
    assert(hash_map_put(&map, (int)keys[0], 10, &inserted) == HASH_MAP_OK && inserted);
    assert(hash_map_put(&map, (int)keys[1], 20, &inserted) == HASH_MAP_OK && inserted);
    assert(hash_map_put(&map, (int)keys[2], 30, &inserted) == HASH_MAP_OK && inserted);
    assert(hash_map_get(&map, (int)keys[2], &value) == HASH_MAP_OK && value == 30);
    assert(hash_map_remove(&map, (int)keys[1]) == HASH_MAP_OK);
    assert(hash_map_get(&map, (int)keys[2], &value) == HASH_MAP_OK && value == 30);
    assert(hash_map_get(&map, (int)keys[1], &value) == HASH_MAP_NOT_FOUND);
    assert(hash_map_put(&map, (int)keys[0], 99, &inserted) == HASH_MAP_OK && !inserted);
    assert(hash_map_get(&map, (int)keys[0], &value) == HASH_MAP_OK && value == 99);
    assert(hash_map_put(&map, (int)keys[1], 21, &inserted) == HASH_MAP_OK && inserted);
    assert(hash_map_get(&map, (int)keys[1], &value) == HASH_MAP_OK && value == 21);
    assert(hash_map_remove(&map, (int)keys[1]) == HASH_MAP_OK);
    assert(hash_map_remove(&map, (int)keys[1]) == HASH_MAP_NOT_FOUND);
    hash_map_destroy(&map);
}

static void test_missing_growth_and_extreme_keys(void)
{
    HashMap map;
    size_t i;
    int value;
    bool inserted;

    assert(hash_map_init(&map, 0U) == HASH_MAP_OK);
    assert(hash_map_get(&map, 123, &value) == HASH_MAP_NOT_FOUND);
    assert(hash_map_put(&map, INT_MIN, -7, &inserted) == HASH_MAP_OK && inserted);
    assert(hash_map_put(&map, INT_MAX, 7, &inserted) == HASH_MAP_OK && inserted);
    assert(hash_map_put(&map, -1, 11, &inserted) == HASH_MAP_OK && inserted);
    for (i = 0U; i < 500U; ++i) {
        assert(hash_map_put(&map, (int)i, (int)(i * 3U), &inserted) == HASH_MAP_OK);
        assert(inserted);
    }
    assert(map.capacity > HASH_MAP_DEFAULT_CAPACITY);
    assert(hash_map_get(&map, INT_MIN, &value) == HASH_MAP_OK && value == -7);
    assert(hash_map_get(&map, INT_MAX, &value) == HASH_MAP_OK && value == 7);
    assert(hash_map_get(&map, -1, &value) == HASH_MAP_OK && value == 11);
    for (i = 0U; i < 500U; ++i) {
        assert(hash_map_get(&map, (int)i, &value) == HASH_MAP_OK);
        assert(value == (int)(i * 3U));
    }
    assert(hash_map_size(&map) == 503U);
    hash_map_destroy(&map);
}

static void test_large_collision_chain_growth_and_tombstones(void)
{
    enum { KEY_COUNT = 900, FINAL_CAPACITY = 2048 };
    HashMap map;
    int keys[KEY_COUNT];
    size_t count = 0U;
    int candidate;
    size_t i;
    int value;
    bool inserted;

    assert(hash_map_init(&map, 8U) == HASH_MAP_OK);
    /* All keys share the final table's last bucket, forcing wraparound probes. */
    for (candidate = 0; candidate < INT_MAX && count < KEY_COUNT; ++candidate) {
        if (hash_map_index_for(candidate, FINAL_CAPACITY) == FINAL_CAPACITY - 1U) {
            keys[count++] = candidate;
        }
    }
    assert(count == KEY_COUNT);

    for (i = 0U; i < KEY_COUNT; ++i) {
        assert(hash_map_put(&map, keys[i], (int)i, &inserted) == HASH_MAP_OK);
        assert(inserted);
    }
    assert(map.capacity == FINAL_CAPACITY);
    assert(hash_map_size(&map) == KEY_COUNT);

    for (i = 0U; i < KEY_COUNT; ++i) {
        assert(hash_map_get(&map, keys[i], &value) == HASH_MAP_OK);
        assert(value == (int)i);
        assert(hash_map_put(&map, keys[i], (int)(i + KEY_COUNT), &inserted) == HASH_MAP_OK);
        assert(!inserted);
    }
    assert(hash_map_size(&map) == KEY_COUNT);

    for (i = 0U; i < KEY_COUNT; i += 2U) {
        assert(hash_map_remove(&map, keys[i]) == HASH_MAP_OK);
    }
    for (i = 0U; i < KEY_COUNT; ++i) {
        if (i % 2U == 0U) {
            assert(hash_map_get(&map, keys[i], &value) == HASH_MAP_NOT_FOUND);
        } else {
            assert(hash_map_get(&map, keys[i], &value) == HASH_MAP_OK);
            assert(value == (int)(i + KEY_COUNT));
        }
    }
    for (i = 0U; i < KEY_COUNT; i += 2U) {
        assert(hash_map_put(&map, keys[i], -(int)i, &inserted) == HASH_MAP_OK);
        assert(inserted);
    }
    assert(hash_map_size(&map) == KEY_COUNT);
    for (i = 0U; i < KEY_COUNT; ++i) {
        assert(hash_map_get(&map, keys[i], &value) == HASH_MAP_OK);
        assert(value == (i % 2U == 0U ? -(int)i : (int)(i + KEY_COUNT)));
    }
    hash_map_destroy(&map);
}

static void test_invalid_arguments_and_capacity_overflow(void)
{
    HashMap map;
    int value;

    assert(hash_map_init(NULL, 8U) == HASH_MAP_INVALID_ARGUMENT);
    assert(hash_map_init(&map, SIZE_MAX) == HASH_MAP_CAPACITY_OVERFLOW);
    assert(hash_map_put(NULL, 1, 2, NULL) == HASH_MAP_INVALID_ARGUMENT);
    assert(hash_map_get(NULL, 1, &value) == HASH_MAP_INVALID_ARGUMENT);
    assert(hash_map_get(NULL, 1, NULL) == HASH_MAP_INVALID_ARGUMENT);
    assert(hash_map_remove(NULL, 1) == HASH_MAP_INVALID_ARGUMENT);
    assert(hash_map_size(NULL) == 0U);
    hash_map_destroy(NULL);
}

int main(void)
{
    test_collisions_updates_and_deletion();
    test_missing_growth_and_extreme_keys();
    test_large_collision_chain_growth_and_tombstones();
    test_invalid_arguments_and_capacity_overflow();
    puts("hash_table self-tests passed");
    return 0;
}
#endif
