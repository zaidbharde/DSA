#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
 * A bounded FIFO integer queue backed by allocated storage.
 *
 * Initialize a queue with queue_init() before any other operation. A
 * zero-capacity queue is valid: it is both empty and full, and enqueue,
 * dequeue, and peek fail. Operations return true on success (or when the
 * queried condition holds); enqueue fails when full, while dequeue and peek
 * fail when empty. A false return also indicates a null queue/output pointer.
 * Always call queue_destroy() when finished. Do not initialize a live queue
 * again without first destroying it.
 *
 * Complexity: enqueue, dequeue, peek, is_empty, and is_full are O(1);
 * storage is O(capacity).
 */
typedef struct {
    int *items;
    size_t capacity;
    size_t head;
    size_t count;
} CircularQueue;

/* Returns true on success. Zero capacity needs no allocation and succeeds. */
static bool queue_init(CircularQueue *queue, size_t capacity)
{
    if (queue == NULL) {
        return false;
    }

    queue->items = NULL;
    queue->capacity = 0U;
    queue->head = 0U;
    queue->count = 0U;

    if (capacity == 0U) {
        return true;
    }
    if (capacity > SIZE_MAX / sizeof(*queue->items)) {
        return false;
    }

    queue->items = malloc(capacity * sizeof(*queue->items));
    if (queue->items == NULL) {
        return false;
    }
    queue->capacity = capacity;
    return true;
}

/* Returns true if an item was added; false if full or given a null queue. */
static bool queue_enqueue(CircularQueue *queue, int value)
{
    size_t tail;

    if (queue == NULL || queue->count == queue->capacity) {
        return false;
    }

    tail = (queue->head + queue->count) % queue->capacity;
    queue->items[tail] = value;
    queue->count++;
    return true;
}

/* Returns true if an item was removed and written to out; false if empty. */
static bool queue_dequeue(CircularQueue *queue, int *out)
{
    if (queue == NULL || out == NULL || queue->count == 0U) {
        return false;
    }

    *out = queue->items[queue->head];
    queue->head = (queue->head + 1U) % queue->capacity;
    queue->count--;
    return true;
}

/* Returns true if the front item was written to out; false if empty. */
static bool queue_peek(const CircularQueue *queue, int *out)
{
    if (queue == NULL || out == NULL || queue->count == 0U) {
        return false;
    }

    *out = queue->items[queue->head];
    return true;
}

/* A null queue is treated as empty. */
static bool queue_is_empty(const CircularQueue *queue)
{
    return queue == NULL || queue->count == 0U;
}

/* A null queue is treated as full (it cannot accept an item). */
static bool queue_is_full(const CircularQueue *queue)
{
    return queue == NULL || queue->count == queue->capacity;
}

/* Safe for null pointers and repeated calls; resets the object. */
static void queue_destroy(CircularQueue *queue)
{
    if (queue == NULL) {
        return;
    }

    free(queue->items);
    queue->items = NULL;
    queue->capacity = 0U;
    queue->head = 0U;
    queue->count = 0U;
}

static void test_fifo_and_wraparound(void)
{
    CircularQueue queue = {NULL, 0U, 0U, 0U};
    int value = 0;

    assert(queue_init(&queue, 3U));
    assert(queue_is_empty(&queue));
    assert(!queue_is_full(&queue));
    assert(queue_enqueue(&queue, 10));
    assert(queue_enqueue(&queue, 20));
    assert(queue_enqueue(&queue, 30));
    assert(queue_is_full(&queue));
    assert(!queue_enqueue(&queue, 40));

    assert(queue_dequeue(&queue, &value) && value == 10);
    assert(queue_dequeue(&queue, &value) && value == 20);
    assert(queue_enqueue(&queue, 40));
    assert(queue_enqueue(&queue, 50));
    assert(queue_is_full(&queue));

    assert(queue_peek(&queue, &value) && value == 30);
    assert(queue_dequeue(&queue, &value) && value == 30);
    assert(queue_dequeue(&queue, &value) && value == 40);
    assert(queue_dequeue(&queue, &value) && value == 50);
    assert(queue_is_empty(&queue));
    assert(!queue_dequeue(&queue, &value));
    assert(!queue_peek(&queue, &value));
    queue_destroy(&queue);
}

static void test_empty_and_zero_capacity(void)
{
    CircularQueue queue = {NULL, 0U, 0U, 0U};
    int value = 123;

    assert(queue_init(&queue, 0U));
    assert(queue_is_empty(&queue));
    assert(queue_is_full(&queue));
    assert(!queue_enqueue(&queue, 1));
    assert(!queue_dequeue(&queue, &value));
    assert(!queue_peek(&queue, &value));
    assert(value == 123);
    queue_destroy(&queue);
    queue_destroy(&queue);
}

static void test_invalid_arguments(void)
{
    CircularQueue queue = {NULL, 0U, 0U, 0U};
    int value = 0;

    assert(!queue_init(NULL, 1U));
    assert(!queue_enqueue(NULL, 1));
    assert(!queue_dequeue(NULL, &value));
    assert(!queue_dequeue(&queue, NULL));
    assert(!queue_peek(NULL, &value));
    assert(!queue_peek(&queue, NULL));
    assert(queue_is_empty(NULL));
    assert(queue_is_full(NULL));
    queue_destroy(NULL);
}

int main(void)
{
    test_fifo_and_wraparound();
    test_empty_and_zero_capacity();
    test_invalid_arguments();
    puts("All circular queue tests passed.");
    return 0;
}
