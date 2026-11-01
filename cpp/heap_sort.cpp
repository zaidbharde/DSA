#include <iostream>
#include <limits>
#include <string>
#include <utility>
#include <vector>

// Ascending in-place heap sort using an iterative sift-down.
// Time: O(n log n). Auxiliary space: O(1).
void siftDown(std::vector<int>& values, std::size_t root, std::size_t heapSize) {
    while (root < heapSize / 2) {
        const std::size_t left = root * 2 + 1;
        const std::size_t right = left + 1;
        std::size_t largest = root;

        if (values[left] > values[largest]) {
            largest = left;
        }
        if (right < heapSize && values[right] > values[largest]) {
            largest = right;
        }
        if (largest == root) {
            return;
        }

        std::swap(values[root], values[largest]);
        root = largest;
    }
}

void heapSort(std::vector<int>& values) {
    const std::size_t size = values.size();

    for (std::size_t index = size / 2; index > 0; --index) {
        siftDown(values, index - 1, size);
    }

    for (std::size_t heapSize = size; heapSize > 1; --heapSize) {
        std::swap(values[0], values[heapSize - 1]);
        siftDown(values, 0, heapSize - 1);
    }
}

bool runTest(const std::string& name, std::vector<int> input,
             const std::vector<int>& expected) {
    heapSort(input);
    if (input != expected) {
        std::cerr << "FAIL: " << name << '\n';
        return false;
    }
    std::cout << "PASS: " << name << '\n';
    return true;
}

int main() {
    const int minValue = std::numeric_limits<int>::min();
    const int maxValue = std::numeric_limits<int>::max();
    bool allPassed = true;

    allPassed = runTest("empty", {}, {}) && allPassed;
    allPassed = runTest("one item", {42}, {42}) && allPassed;
    allPassed = runTest("duplicates", {4, 2, 4, 1, 2, 4}, {1, 2, 2, 4, 4, 4}) && allPassed;
    allPassed = runTest("already sorted", {-3, -1, 0, 2, 8}, {-3, -1, 0, 2, 8}) && allPassed;
    allPassed = runTest("reverse sorted", {9, 5, 2, 0, -4}, {-4, 0, 2, 5, 9}) && allPassed;
    allPassed = runTest("negative values", {-8, -1, -12, -3}, {-12, -8, -3, -1}) && allPassed;
    allPassed = runTest("integer limits", {maxValue, 0, minValue, -1, 1},
                        {minValue, -1, 0, 1, maxValue}) && allPassed;

    if (!allPassed) {
        return 1;
    }
    std::cout << "All heap sort self-tests passed.\n";
    return 0;
}
