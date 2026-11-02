#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

// Returns the 1-indexed kth-smallest value, reordering values in place.
// Average time: O(n). Worst-case time: O(n^2). Auxiliary space: O(1).
// The deterministic median-of-three pivot helps typical inputs but does not
// change the quadratic worst case. Three-way partitioning handles duplicates.
int kthSmallest(std::vector<int>& values, int k) {
    if (k < 1) {
        throw std::invalid_argument("k must be at least 1");
    }
    if (static_cast<std::size_t>(k) > values.size()) {
        throw std::out_of_range("k must not exceed the number of elements");
    }

    std::size_t left = 0;
    std::size_t right = values.size();  // Exclusive.
    const std::size_t target = static_cast<std::size_t>(k - 1);

    while (true) {
        const std::size_t middle = left + (right - left) / 2;
        const int first = values[left];
        const int median_candidate = values[middle];
        const int last = values[right - 1];

        int pivot;
        if ((first <= median_candidate && median_candidate <= last) ||
            (last <= median_candidate && median_candidate <= first)) {
            pivot = median_candidate;
        } else if ((median_candidate <= first && first <= last) ||
                   (last <= first && first <= median_candidate)) {
            pivot = first;
        } else {
            pivot = last;
        }

        // After partitioning: [left, less_end) < pivot,
        // [less_end, scan) == pivot, and [greater_begin, right) > pivot.
        std::size_t less_end = left;
        std::size_t scan = left;
        std::size_t greater_begin = right;
        while (scan < greater_begin) {
            if (values[scan] < pivot) {
                const int temporary = values[less_end];
                values[less_end] = values[scan];
                values[scan] = temporary;
                ++less_end;
                ++scan;
            } else if (values[scan] > pivot) {
                --greater_begin;
                const int temporary = values[scan];
                values[scan] = values[greater_begin];
                values[greater_begin] = temporary;
            } else {
                ++scan;
            }
        }

        if (target < less_end) {
            right = less_end;
        } else if (target >= greater_begin) {
            left = greater_begin;
        } else {
            return pivot;
        }
    }
}

namespace {

void expectEqual(int actual, int expected, const std::string& test_name) {
    if (actual != expected) {
        throw std::runtime_error(test_name + ": expected " +
                                 std::to_string(expected) + ", got " +
                                 std::to_string(actual));
    }
}

void expectThrowsInvalidArgument(const std::vector<int>& input, int k,
                                 const std::string& test_name) {
    std::vector<int> values = input;
    try {
        (void)kthSmallest(values, k);
    } catch (const std::invalid_argument&) {
        return;
    }
    throw std::runtime_error(test_name + ": expected std::invalid_argument");
}

void expectThrowsOutOfRange(const std::vector<int>& input, int k,
                            const std::string& test_name) {
    std::vector<int> values = input;
    try {
        (void)kthSmallest(values, k);
    } catch (const std::out_of_range&) {
        return;
    }
    throw std::runtime_error(test_name + ": expected std::out_of_range");
}

void runTests() {
    // First/last ranks, including singleton input.
    std::vector<int> first_case{7, 2, 9, 1, 5};
    expectEqual(kthSmallest(first_case, 1), 1, "first k");
    std::vector<int> last_case{7, 2, 9, 1, 5};
    expectEqual(kthSmallest(last_case, 5), 9, "last k");
    std::vector<int> singleton{42};
    expectEqual(kthSmallest(singleton, 1), 42, "singleton");

    // Repeated values, already sorted input, reverse-sorted input, negatives.
    std::vector<int> duplicates{4, 1, 4, 2, 4, 2, 1};
    expectEqual(kthSmallest(duplicates, 4), 2, "duplicates");
    std::vector<int> sorted{1, 2, 3, 4, 5, 6};
    expectEqual(kthSmallest(sorted, 3), 3, "sorted input");
    std::vector<int> reverse_sorted{6, 5, 4, 3, 2, 1};
    expectEqual(kthSmallest(reverse_sorted, 4), 4, "reverse-sorted input");
    std::vector<int> negatives{-8, -3, -10, 0, -1};
    expectEqual(kthSmallest(negatives, 2), -8, "negative values");

    // Rank validation, including empty input.
    expectThrowsInvalidArgument({3, 1}, 0, "k equal to zero");
    expectThrowsInvalidArgument({}, -1, "negative k");
    expectThrowsOutOfRange({3, 1}, 3, "k greater than size");
    expectThrowsOutOfRange({}, 1, "empty input");
}

}  // namespace

int main() {
    try {
        runTests();
        std::cout << "All quickselect tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Quickselect test failure: " << error.what() << '\n';
        return 1;
    }
}
