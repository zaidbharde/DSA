#include <cassert>
#include <cstddef>
#include <iostream>
#include <vector>

using Index = std::ptrdiff_t;

Index search(const std::vector<int>& arr, int target) {
    std::size_t lo = 0;
    std::size_t hi = arr.size();

    // Find the first position whose value is not less than target.
    while (lo < hi) {
        const std::size_t mid = lo + (hi - lo) / 2;
        if (arr[mid] < target) {
            lo = mid + 1;
        } else {
            hi = mid;
        }
    }

    if (lo < arr.size() && arr[lo] == target) {
        return static_cast<Index>(lo);
    }
    return -1;
}

int main() {
    const std::vector<int> example = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    std::cout << "Example: 23 found at index " << search(example, 23) << '\n';

    const std::vector<int> empty;
    const std::vector<int> singleton = {7};
    const std::vector<int> values = {2, 5, 8, 12, 16};
    const std::vector<int> duplicates = {1, 3, 3, 3, 6};

    assert(search(empty, 1) == -1);                 // Empty vector
    assert(search(singleton, 7) == 0);              // Singleton hit
    assert(search(singleton, 4) == -1);             // Singleton miss
    assert(search(values, 2) == 0);                 // First position
    assert(search(values, 16) == 4);                // Last position
    assert(search(duplicates, 3) == 1);             // First duplicate
    assert(search(values, 9) == -1);                // Absent target

    std::cout << "All binary search self-tests passed.\n";
    return 0;
}
