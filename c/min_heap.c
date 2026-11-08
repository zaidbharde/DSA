/*
 * Dynamic binary min-heap for int values.
 *
 * Call min_heap_init() before using a MinHeap and min_heap_destroy() when
 * finished. Do not copy an initialized MinHeap by value or initialize it again
 * before destroying it. An initial capacity of zero is valid and defers
 * allocation until the first push. Status values distinguish invalid
 * arguments, empty-heap operations, allocation failure, and capacity overflow.
 * On a failed growth, the existing heap and its contents are unchanged.
 *
 * Complexity: push and pop are O(log n), peek is O(1), and storage is O(n).
 * Compile with -DMIN_HEAP_SELF_TEST to enable the standalone tests.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum MinHeapStatus {
    MIN_HEAP_OK = 0,
    MIN_HEAP_INVALID_ARGUMENT,
    MIN_HEAP_EMPTY,
    MIN_HEAP_OUT_OF_MEMORY,
    MIN_HEAP_CAPACITY_OVERFLOW
} MinHeapStatus;

typedef struct MinHeap {
    int *items;
    size_t size;
    size_t capacity;
    bool initialized;
} MinHeap;

MinHeapStatus min_heap_init(MinHeap *heap, size_t initial_capacity);
void min_heap_destroy(MinHeap *heap);
MinHeapStatus min_heap_push(MinHeap *heap, int value);
MinHeapStatus min_heap_peek(const MinHeap *heap, int *out_value);
MinHeapStatus min_heap_pop(MinHeap *heap, int *out_value);
MinHeapStatus min_heap_size(const MinHeap *heap, size_t *out_size);

#define MIN_HEAP_DEFAULT_CAPACITY ((size_t)8)

static bool min_heap_is_valid(const MinHeap *heap)
{
    return heap != NULL && heap->initialized &&
           ((heap->capacity == 0U && heap->items == NULL && heap->size == 0U) ||
            (heap->capacity > 0U && heap->items != NULL &&
             heap->size <= heap->capacity));
}

MinHeapStatus min_heap_init(MinHeap *heap, size_t initial_capacity)
{
    int *items = NULL;

    if (heap == NULL) {
        return MIN_HEAP_INVALID_ARGUMENT;
    }

    heap->items = NULL;
    heap->size = 0U;
    heap->capacity = 0U;
    heap->initialized = false;

    if (initial_capacity > SIZE_MAX / sizeof(*items)) {
        return MIN_HEAP_CAPACITY_OVERFLOW;
    }
    if (initial_capacity > 0U) {
        items = (int *)malloc(initial_capacity * sizeof(*items));
        if (items == NULL) {
            return MIN_HEAP_OUT_OF_MEMORY;
        }
    }

    heap->items = items;
    heap->capacity = initial_capacity;
    heap->initialized = true;
    return MIN_HEAP_OK;
}

void min_heap_destroy(MinHeap *heap)
{
    if (heap == NULL) {
        return;
    }

    free(heap->items);
    heap->items = NULL;
    heap->size = 0U;
    heap->capacity = 0U;
    heap->initialized = false;
}

static MinHeapStatus min_heap_grow(MinHeap *heap)
{
    size_t new_capacity;
    int *new_items;
    size_t i;

    if (heap->capacity == 0U) {
        new_capacity = MIN_HEAP_DEFAULT_CAPACITY;
    } else {
        if (heap->capacity > SIZE_MAX / 2U) {
            return MIN_HEAP_CAPACITY_OVERFLOW;
        }
        new_capacity = heap->capacity * 2U;
    }
    if (new_capacity > SIZE_MAX / sizeof(*new_items)) {
        return MIN_HEAP_CAPACITY_OVERFLOW;
    }

    new_items = (int *)malloc(new_capacity * sizeof(*new_items));
    if (new_items == NULL) {
        return MIN_HEAP_OUT_OF_MEMORY;
    }
    for (i = 0U; i < heap->size; ++i) {
        new_items[i] = heap->items[i];
    }

    free(heap->items);
    heap->items = new_items;
    heap->capacity = new_capacity;
    return MIN_HEAP_OK;
}

MinHeapStatus min_heap_push(MinHeap *heap, int value)
{
    size_t index;

    if (!min_heap_is_valid(heap)) {
        return MIN_HEAP_INVALID_ARGUMENT;
    }
    if (heap->size == heap->capacity) {
        const MinHeapStatus status = min_heap_grow(heap);
        if (status != MIN_HEAP_OK) {
            return status;
        }
    }

    index = heap->size;
    heap->items[index] = value;
    ++heap->size;
    while (index > 0U) {
        const size_t parent = (index - 1U) / 2U;
        int temporary;

        if (heap->items[parent] <= heap->items[index]) {
            break;
        }
        temporary = heap->items[parent];
        heap->items[parent] = heap->items[index];
        heap->items[index] = temporary;
        index = parent;
    }
    return MIN_HEAP_OK;
}

MinHeapStatus min_heap_peek(const MinHeap *heap, int *out_value)
{
    if (!min_heap_is_valid(heap) || out_value == NULL) {
        return MIN_HEAP_INVALID_ARGUMENT;
    }
    if (heap->size == 0U) {
        return MIN_HEAP_EMPTY;
    }

    *out_value = heap->items[0];
    return MIN_HEAP_OK;
}

MinHeapStatus min_heap_pop(MinHeap *heap, int *out_value)
{
    size_t index;

    if (!min_heap_is_valid(heap) || out_value == NULL) {
        return MIN_HEAP_INVALID_ARGUMENT;
    }
    if (heap->size == 0U) {
        return MIN_HEAP_EMPTY;
    }

    *out_value = heap->items[0];
    --heap->size;
    if (heap->size == 0U) {
        return MIN_HEAP_OK;
    }

    heap->items[0] = heap->items[heap->size];
    index = 0U;
    while (index < heap->size / 2U) {
        const size_t left = index * 2U + 1U;
        const size_t right = left + 1U;
        size_t smaller_child = left;
        int temporary;

        if (right < heap->size && heap->items[right] < heap->items[left]) {
            smaller_child = right;
        }
        if (heap->items[index] <= heap->items[smaller_child]) {
            break;
        }
        temporary = heap->items[index];
        heap->items[index] = heap->items[smaller_child];
        heap->items[smaller_child] = temporary;
        index = smaller_child;
    }
    return MIN_HEAP_OK;
}

MinHeapStatus min_heap_size(const MinHeap *heap, size_t *out_size)
{
    if (!min_heap_is_valid(heap) || out_size == NULL) {
        return MIN_HEAP_INVALID_ARGUMENT;
    }

    *out_size = heap->size;
    return MIN_HEAP_OK;
}

#ifdef MIN_HEAP_SELF_TEST
#include <assert.h>
#include <limits.h>
#include <stdio.h>

static void test_empty_and_singleton(void)
{
    MinHeap heap;
    size_t count = 99U;
    int value = 123;

    assert(min_heap_init(&heap, 0U) == MIN_HEAP_OK);
    assert(min_heap_size(&heap, &count) == MIN_HEAP_OK && count == 0U);
    assert(min_heap_peek(&heap, &value) == MIN_HEAP_EMPTY && value == 123);
    assert(min_heap_pop(&heap, &value) == MIN_HEAP_EMPTY && value == 123);
    assert(min_heap_push(&heap, 7) == MIN_HEAP_OK);
    assert(min_heap_peek(&heap, &value) == MIN_HEAP_OK && value == 7);
    assert(min_heap_pop(&heap, &value) == MIN_HEAP_OK && value == 7);
    assert(min_heap_size(&heap, &count) == MIN_HEAP_OK && count == 0U);
    min_heap_destroy(&heap);
}

static void test_duplicates_extremes_and_ordering(void)
{
    MinHeap heap;
    const int values[] = {5, -2, 5, INT_MAX, INT_MIN, 0, -2, 17};
    const int expected[] = {INT_MIN, -2, -2, 0, 5, 5, 17, INT_MAX};
    size_t i;
    size_t count;
    int value;

    assert(min_heap_init(&heap, 2U) == MIN_HEAP_OK);
    for (i = 0U; i < sizeof(values) / sizeof(values[0]); ++i) {
        assert(min_heap_push(&heap, values[i]) == MIN_HEAP_OK);
    }
    assert(heap.capacity >= sizeof(values) / sizeof(values[0]));
    assert(min_heap_size(&heap, &count) == MIN_HEAP_OK && count == 8U);
    for (i = 0U; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        assert(min_heap_peek(&heap, &value) == MIN_HEAP_OK);
        assert(value == expected[i]);
        assert(min_heap_pop(&heap, &value) == MIN_HEAP_OK);
        assert(value == expected[i]);
    }
    assert(min_heap_pop(&heap, &value) == MIN_HEAP_EMPTY);
    min_heap_destroy(&heap);
}

static void test_invalid_arguments_and_capacity_overflow(void)
{
    MinHeap heap;
    MinHeap boundary_heap;
    int value = 42;
    size_t count = 42U;

    assert(min_heap_init(NULL, 0U) == MIN_HEAP_INVALID_ARGUMENT);
    assert(min_heap_init(&heap, SIZE_MAX) == MIN_HEAP_CAPACITY_OVERFLOW);
    assert(min_heap_push(NULL, 1) == MIN_HEAP_INVALID_ARGUMENT);
    assert(min_heap_peek(NULL, &value) == MIN_HEAP_INVALID_ARGUMENT);
    assert(min_heap_pop(NULL, &value) == MIN_HEAP_INVALID_ARGUMENT);
    assert(min_heap_size(NULL, &count) == MIN_HEAP_INVALID_ARGUMENT);
    assert(min_heap_init(&heap, 1U) == MIN_HEAP_OK);
    assert(min_heap_peek(&heap, NULL) == MIN_HEAP_INVALID_ARGUMENT);
    assert(min_heap_pop(&heap, NULL) == MIN_HEAP_INVALID_ARGUMENT);
    assert(min_heap_size(&heap, NULL) == MIN_HEAP_INVALID_ARGUMENT);
    min_heap_destroy(&heap);
    assert(min_heap_push(&heap, 1) == MIN_HEAP_INVALID_ARGUMENT);
    min_heap_destroy(NULL);

    /* Synthetic boundary state: push detects overflow before accessing items. */
    value = 11;
    boundary_heap.items = &value;
    boundary_heap.size = SIZE_MAX / sizeof(int) + 1U;
    boundary_heap.capacity = boundary_heap.size;
    boundary_heap.initialized = true;
    assert(min_heap_push(&boundary_heap, 99) == MIN_HEAP_CAPACITY_OVERFLOW);
    assert(boundary_heap.items == &value);
    assert(boundary_heap.size == SIZE_MAX / sizeof(int) + 1U);
    assert(boundary_heap.capacity == boundary_heap.size);
    assert(value == 11);
}

int main(void)
{
    test_empty_and_singleton();
    test_duplicates_extremes_and_ordering();
    test_invalid_arguments_and_capacity_overflow();
    puts("min_heap: all self-tests passed");
    return 0;
}
#endif
