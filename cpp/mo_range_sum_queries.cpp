#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace mo_range_sum_queries {

/** An inclusive range [left, right], with zero-based indices. */
struct Query {
    std::size_t left;
    std::size_t right;
};

/**
 * Answers inclusive range-sum queries offline with Mo's algorithm. Queries are
 * sorted by left-index blocks and by alternating right-end order within each
 * block, reducing pointer travel; computed answers are returned in input order.
 *
 * Throws std::invalid_argument if any query is reversed or either endpoint is
 * outside values. Throws std::overflow_error if a sum encountered in the
 * moving window cannot be represented by int64_t.
 *
 * With block size floor(sqrt(N)) (at least one), sorting costs O(Q log Q) and
 * pointer updates cost O(N sqrt(N) + Q sqrt(N)); total time is
 * O(Q log Q + (N + Q) sqrt(N)). Auxiliary storage is O(N + Q), including the
 * sorted query records and answer vector; input values and queries are not
 * copied, aside from one O(Q) decorated-query array.
 */
std::vector<std::int64_t> range_sums(const std::vector<std::int64_t>& values,
                                     const std::vector<Query>& queries) {
    const std::size_t value_count = values.size();
    for (const Query& query : queries) {
        if (query.left > query.right || query.right >= value_count) {
            throw std::invalid_argument("query bounds must form a valid inclusive range");
        }
    }
    if (queries.empty()) {
        return {};
    }

    struct OrderedQuery {
        Query range;
        std::size_t answer_index;
    };
    std::vector<OrderedQuery> ordered;
    ordered.reserve(queries.size());
    for (std::size_t index = 0U; index < queries.size(); ++index) {
        ordered.push_back({queries[index], index});
    }

    const std::size_t block_size = std::max<std::size_t>(
        1U, static_cast<std::size_t>(std::sqrt(static_cast<double>(value_count))));
    std::sort(ordered.begin(), ordered.end(), [block_size](const OrderedQuery& a,
                                                            const OrderedQuery& b) {
        const std::size_t a_block = a.range.left / block_size;
        const std::size_t b_block = b.range.left / block_size;
        if (a_block != b_block) {
            return a_block < b_block;
        }
        if (a.range.right != b.range.right) {
            return (a_block % 2U == 0U) ? a.range.right < b.range.right
                                       : a.range.right > b.range.right;
        }
        return a.answer_index < b.answer_index;
    });

    const auto add_checked = [](std::int64_t sum, std::int64_t value) {
        constexpr std::int64_t minimum = std::numeric_limits<std::int64_t>::min();
        constexpr std::int64_t maximum = std::numeric_limits<std::int64_t>::max();
        if ((value > 0 && sum > maximum - value) ||
            (value < 0 && sum < minimum - value)) {
            throw std::overflow_error("range sum is outside int64_t range");
        }
        return static_cast<std::int64_t>(sum + value);
    };
    const auto subtract_checked = [](std::int64_t sum, std::int64_t value) {
        constexpr std::int64_t minimum = std::numeric_limits<std::int64_t>::min();
        constexpr std::int64_t maximum = std::numeric_limits<std::int64_t>::max();
        if ((value > 0 && sum < minimum + value) ||
            (value < 0 && sum > maximum + value)) {
            throw std::overflow_error("range sum is outside int64_t range");
        }
        return static_cast<std::int64_t>(sum - value);
    };

    std::vector<std::int64_t> answers(queries.size(), 0);
    std::size_t current_left = 0U;
    std::size_t current_right = 0U;  // Current window is [current_left, current_right).
    std::int64_t current_sum = 0;
    for (const OrderedQuery& query : ordered) {
        const std::size_t target_right = query.range.right + 1U;
        while (current_left > query.range.left) {
            --current_left;
            current_sum = add_checked(current_sum, values[current_left]);
        }
        while (current_right < target_right) {
            current_sum = add_checked(current_sum, values[current_right]);
            ++current_right;
        }
        while (current_left < query.range.left) {
            current_sum = subtract_checked(current_sum, values[current_left]);
            ++current_left;
        }
        while (current_right > target_right) {
            --current_right;
            current_sum = subtract_checked(current_sum, values[current_right]);
        }
        answers[query.answer_index] = current_sum;
    }
    return answers;
}

}  // namespace mo_range_sum_queries

namespace {

using mo_range_sum_queries::Query;
using mo_range_sum_queries::range_sums;

void test_empty_query_set() {
    assert(range_sums({}, {}).empty());
    assert(range_sums({4, -2, 9}, {}).empty());
}

void test_singleton_whole_range_duplicates_and_negatives() {
    const std::vector<std::int64_t> values{5, -8, 12, -3, -6};
    const std::vector<Query> queries{{2U, 2U}, {0U, 4U}, {1U, 3U},
                                     {2U, 2U}, {0U, 0U}, {0U, 4U}};
    const std::vector<std::int64_t> expected{12, 0, 1, 12, 5, 0};
    assert(range_sums(values, queries) == expected);
}

void test_sum_exceeding_32_bit_range() {
    const std::vector<std::int64_t> values{2000000000LL, 2000000000LL};
    assert(range_sums(values, {{0U, 1U}}) ==
           std::vector<std::int64_t>{4000000000LL});
}

void test_deterministic_differential_cases() {
    std::uint32_t state = 0x5a17U;
    const auto next_value = [&state]() {
        state = state * 1664525U + 1013904223U;
        return state;
    };
    for (std::size_t size = 1U; size <= 24U; ++size) {
        std::vector<std::int64_t> values(size);
        for (std::int64_t& value : values) {
            value = static_cast<std::int64_t>(next_value() % 41U) - 20;
        }
        std::vector<Query> queries;
        for (std::size_t i = 0U; i < 90U; ++i) {
            const std::size_t first = next_value() % size;
            const std::size_t second = next_value() % size;
            queries.push_back({std::min(first, second), std::max(first, second)});
        }
        const std::vector<std::int64_t> actual = range_sums(values, queries);
        assert(actual.size() == queries.size());
        for (std::size_t i = 0U; i < queries.size(); ++i) {
            std::int64_t expected = 0;
            for (std::size_t index = queries[i].left; index <= queries[i].right; ++index) {
                expected += values[index];
            }
            assert(actual[i] == expected);
        }
    }
}

void test_invalid_query_bounds() {
    bool reversed_threw = false;
    try {
        static_cast<void>(range_sums({1, 2}, {{1U, 0U}}));
    } catch (const std::invalid_argument&) {
        reversed_threw = true;
    }
    assert(reversed_threw);

    bool out_of_bounds_threw = false;
    try {
        static_cast<void>(range_sums({1, 2}, {{0U, 2U}}));
    } catch (const std::invalid_argument&) {
        out_of_bounds_threw = true;
    }
    assert(out_of_bounds_threw);

    bool empty_input_threw = false;
    try {
        static_cast<void>(range_sums({}, {{0U, 0U}}));
    } catch (const std::invalid_argument&) {
        empty_input_threw = true;
    }
    assert(empty_input_threw);
}

}  // namespace

int main() {
    test_empty_query_set();
    test_singleton_whole_range_duplicates_and_negatives();
    test_sum_exceeding_32_bit_range();
    test_deterministic_differential_cases();
    test_invalid_query_bounds();
}
