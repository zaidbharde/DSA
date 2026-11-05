/*
 * Rotate a row-major n-by-n int matrix 90 degrees clockwise, in place.
 *
 * Return 0 on success and -1 for an invalid request. An empty matrix (n == 0)
 * is a successful no-op, including when matrix is NULL. For n > 0, matrix
 * must be non-NULL. The element-count multiplication is checked before any
 * matrix indexing, so an overflowing n*n is rejected without modifying data.
 *
 * Complexity: O(n^2) time and O(1) auxiliary space.
 */
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

int rotate_matrix_clockwise(int *matrix, size_t n)
{
    size_t row;
    size_t col;

    if (n != 0 && n > SIZE_MAX / n) {
        return -1;
    }
    if (n == 0) {
        return 0;
    }
    if (matrix == NULL) {
        return -1;
    }

    /* Transpose the matrix across its main diagonal. */
    for (row = 0; row < n; ++row) {
        for (col = row + 1; col < n; ++col) {
            const size_t a = row * n + col;
            const size_t b = col * n + row;
            const int temporary = matrix[a];

            matrix[a] = matrix[b];
            matrix[b] = temporary;
        }
    }

    /* Reverse every row to complete the clockwise rotation. */
    for (row = 0; row < n; ++row) {
        for (col = 0; col < n / 2; ++col) {
            const size_t left = row * n + col;
            const size_t right = row * n + (n - 1 - col);
            const int temporary = matrix[left];

            matrix[left] = matrix[right];
            matrix[right] = temporary;
        }
    }

    return 0;
}

int main(void)
{
    int singleton[] = {42};
    int matrix_2x2[] = {1, 2, 3, 4};
    const int expected_2x2[] = {3, 1, 4, 2};
    int matrix_3x3[] = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    const int expected_3x3[] = {7, 4, 1, 8, 5, 2, 9, 6, 3};
    int overflow_guard = 0;
    size_t i;

    /* Empty matrices are successful no-ops, even with a NULL pointer. */
    assert(rotate_matrix_clockwise(NULL, 0) == 0);

    /* Nonempty matrices require storage; overflowing element counts fail. */
    assert(rotate_matrix_clockwise(NULL, 1) == -1);
    assert(rotate_matrix_clockwise(&overflow_guard, SIZE_MAX) == -1);
    assert(overflow_guard == 0);

    assert(rotate_matrix_clockwise(singleton, 1) == 0);
    assert(singleton[0] == 42);

    assert(rotate_matrix_clockwise(matrix_2x2, 2) == 0);
    for (i = 0; i < sizeof matrix_2x2 / sizeof matrix_2x2[0]; ++i) {
        assert(matrix_2x2[i] == expected_2x2[i]);
    }

    assert(rotate_matrix_clockwise(matrix_3x3, 3) == 0);
    for (i = 0; i < sizeof matrix_3x3 / sizeof matrix_3x3[0]; ++i) {
        assert(matrix_3x3[i] == expected_3x3[i]);
    }

    return 0;
}
