#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <utility>
#include <vector>

namespace floyd_warshall {

struct Edge {
    std::size_t from;
    std::size_t to;
    std::int64_t weight;
};

struct Result {
    std::vector<std::vector<std::optional<std::int64_t>>> distances;
    bool has_negative_cycle;
};

// Computes all-pairs shortest-path distances in O(V^3) time and O(V^2) storage.
// Unreachable pairs are represented by std::nullopt. Invalid edge endpoints
// throw std::out_of_range, and an unrepresentable reachable candidate sum
// throws std::overflow_error.
std::int64_t checked_add(std::int64_t lhs, std::int64_t rhs) {
    constexpr std::int64_t minimum = std::numeric_limits<std::int64_t>::min();
    constexpr std::int64_t maximum = std::numeric_limits<std::int64_t>::max();

    if ((rhs > 0 && lhs > maximum - rhs) ||
        (rhs < 0 && lhs < minimum - rhs)) {
        throw std::overflow_error("Floyd-Warshall distance addition overflow");
    }
    return lhs + rhs;
}

Result compute(std::size_t vertex_count, const std::vector<Edge>& edges) {
    std::vector<std::vector<std::optional<std::int64_t>>> distances(
        vertex_count,
        std::vector<std::optional<std::int64_t>>(vertex_count, std::nullopt));

    for (std::size_t vertex = 0; vertex < vertex_count; ++vertex) {
        distances[vertex][vertex] = 0;
    }

    for (const Edge& edge : edges) {
        if (edge.from >= vertex_count || edge.to >= vertex_count) {
            throw std::out_of_range("edge endpoint is outside the graph");
        }
        std::optional<std::int64_t>& current = distances[edge.from][edge.to];
        if (!current.has_value() || edge.weight < *current) {
            current = edge.weight;
        }
    }

    for (std::size_t intermediate = 0; intermediate < vertex_count; ++intermediate) {
        for (std::size_t from = 0; from < vertex_count; ++from) {
            if (!distances[from][intermediate].has_value()) {
                continue;
            }
            for (std::size_t to = 0; to < vertex_count; ++to) {
                if (!distances[intermediate][to].has_value()) {
                    continue;
                }
                const std::int64_t candidate = checked_add(
                    *distances[from][intermediate], *distances[intermediate][to]);
                std::optional<std::int64_t>& current = distances[from][to];
                if (!current.has_value() || candidate < *current) {
                    current = candidate;
                }
            }
        }
    }

    bool has_negative_cycle = false;
    for (std::size_t vertex = 0; vertex < vertex_count; ++vertex) {
        if (distances[vertex][vertex].has_value() &&
            *distances[vertex][vertex] < 0) {
            has_negative_cycle = true;
            break;
        }
    }
    return {std::move(distances), has_negative_cycle};
}

}  // namespace floyd_warshall

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void test_negative_edges_without_cycles() {
    const floyd_warshall::Result result = floyd_warshall::compute(
        3, {{0, 1, 4}, {0, 2, 5}, {1, 2, -2}});
    require(!result.has_negative_cycle, "unexpected negative cycle");
    require(result.distances[0][2] == 2, "negative edge path distance mismatch");
    require(result.distances[2][0] == std::nullopt, "reverse path should be unreachable");
}

void test_disconnected_vertices() {
    const floyd_warshall::Result result =
        floyd_warshall::compute(3, {{0, 1, 7}});
    require(result.distances[0][1] == 7, "direct distance mismatch");
    require(result.distances[0][2] == std::nullopt, "disconnected pair should be unreachable");
    require(result.distances[2][2] == 0, "diagonal should be zero");
    require(!result.has_negative_cycle, "unexpected negative cycle");
}

void test_parallel_edges() {
    const floyd_warshall::Result result =
        floyd_warshall::compute(2, {{0, 1, 9}, {0, 1, 3}, {0, 1, 6}});
    require(result.distances[0][1] == 3, "parallel edges should use minimum weight");
}

void test_negative_cycle() {
    const floyd_warshall::Result result =
        floyd_warshall::compute(3, {{0, 1, 1}, {1, 2, -3}, {2, 0, 1}});
    require(result.has_negative_cycle, "negative cycle was not detected");
}

void test_invalid_endpoints() {
    bool threw = false;
    try {
        static_cast<void>(floyd_warshall::compute(2, {{0, 2, 1}}));
    } catch (const std::out_of_range&) {
        threw = true;
    }
    require(threw, "invalid endpoint should throw std::out_of_range");
}

void test_overflow() {
    const std::int64_t maximum = std::numeric_limits<std::int64_t>::max();
    bool threw = false;
    try {
        static_cast<void>(floyd_warshall::compute(
            3, {{0, 1, maximum}, {1, 2, 1}}));
    } catch (const std::overflow_error&) {
        threw = true;
    }
    require(threw, "unrepresentable reachable sum should throw std::overflow_error");
}

}  // namespace

int main() {
    test_negative_edges_without_cycles();
    test_disconnected_vertices();
    test_parallel_edges();
    test_negative_cycle();
    test_invalid_endpoints();
    test_overflow();
    std::cout << "All Floyd-Warshall tests passed.\n";
    return 0;
}
