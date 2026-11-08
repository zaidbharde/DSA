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

/**
 * Computes all-pairs shortest-path distances for a directed weighted graph.
 * Parallel edges are reduced to their minimum weight; unreachable pairs are
 * represented by std::nullopt. A negative diagonal entry sets
 * has_negative_cycle; when that flag is true, the returned distances are
 * intermediate values and are not promised to be shortest paths. Invalid edge
 * endpoints throw std::out_of_range. Any unrepresentable sum of two currently
 * reachable distances throws std::overflow_error rather than wrapping.
 *
 * Time: O(V^3 + E), including endpoint validation and edge initialization.
 * Storage: O(V^2) for the returned matrix and working state.
 */
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
    // Validate before allocating O(V^2) storage, so malformed input is reported
    // even when the requested matrix would be prohibitively large.
    for (const Edge& edge : edges) {
        if (edge.from >= vertex_count || edge.to >= vertex_count) {
            throw std::out_of_range("edge endpoint is outside the graph");
        }
    }

    std::vector<std::vector<std::optional<std::int64_t>>> distances(
        vertex_count,
        std::vector<std::optional<std::int64_t>>(vertex_count, std::nullopt));

    for (std::size_t vertex = 0; vertex < vertex_count; ++vertex) {
        distances[vertex][vertex] = 0;
    }

    for (const Edge& edge : edges) {
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

// Independent all-sources Bellman-Ford reference used only on small graphs.
// Its bounded test weights make arithmetic overflow impossible here.
floyd_warshall::Result bellman_ford_reference(
    std::size_t vertex_count, const std::vector<floyd_warshall::Edge>& edges) {
    std::vector<std::vector<std::optional<std::int64_t>>> all_distances(
        vertex_count,
        std::vector<std::optional<std::int64_t>>(vertex_count, std::nullopt));
    bool has_negative_cycle = false;

    for (std::size_t source = 0; source < vertex_count; ++source) {
        std::vector<std::optional<std::int64_t>> distances(vertex_count);
        distances[source] = 0;
        for (std::size_t pass = 1; pass < vertex_count; ++pass) {
            bool changed = false;
            for (const floyd_warshall::Edge& edge : edges) {
                if (!distances[edge.from].has_value()) {
                    continue;
                }
                const std::int64_t candidate =
                    *distances[edge.from] + edge.weight;
                if (!distances[edge.to].has_value() ||
                    candidate < *distances[edge.to]) {
                    distances[edge.to] = candidate;
                    changed = true;
                }
            }
            if (!changed) {
                break;
            }
        }

        for (const floyd_warshall::Edge& edge : edges) {
            if (distances[edge.from].has_value() &&
                (!distances[edge.to].has_value() ||
                 *distances[edge.from] + edge.weight < *distances[edge.to])) {
                has_negative_cycle = true;
                break;
            }
        }
        all_distances[source] = std::move(distances);
    }
    return {std::move(all_distances), has_negative_cycle};
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
        floyd_warshall::compute(4, {{0, 1, 7}, {2, 3, -2}});
    require(result.distances[0][1] == 7, "direct distance mismatch");
    require(result.distances[0][2] == std::nullopt, "disconnected pair should be unreachable");
    require(result.distances[2][3] == -2, "second component distance mismatch");
    require(result.distances[3][2] == std::nullopt, "reverse edge should be unreachable");
    require(result.distances[2][2] == 0, "diagonal should be zero");
    require(!result.has_negative_cycle, "unexpected negative cycle");
}

void test_parallel_and_multiple_optimal_paths() {
    const floyd_warshall::Result result = floyd_warshall::compute(
        5, {{0, 1, 9}, {0, 1, 3}, {0, 1, 6},
            {0, 2, 2}, {2, 3, 2}, {0, 3, 4},
            {3, 4, 1}, {2, 4, 3}});
    require(result.distances[0][1] == 3, "parallel edges should use minimum weight");
    require(result.distances[0][3] == 4, "multiple optimal paths distance mismatch");
    require(result.distances[0][4] == 5, "equal-cost routes to destination mismatch");
}

void test_negative_cycles() {
    const floyd_warshall::Result reachable =
        floyd_warshall::compute(3, {{0, 1, 1}, {1, 2, -3}, {2, 0, 1}});
    require(reachable.has_negative_cycle, "negative cycle was not detected");

    const floyd_warshall::Result disconnected =
        floyd_warshall::compute(4, {{0, 1, 5}, {2, 3, -2}, {3, 2, 1}});
    require(disconnected.has_negative_cycle,
            "negative cycle in a disconnected component was not detected");
}

void test_empty_graph_and_invalid_endpoints() {
    const floyd_warshall::Result empty = floyd_warshall::compute(0, {});
    require(empty.distances.empty() && !empty.has_negative_cycle,
            "empty graph result mismatch");

    bool threw = false;
    try {
        static_cast<void>(floyd_warshall::compute(2, {{0, 2, 1}}));
    } catch (const std::out_of_range&) {
        threw = true;
    }
    require(threw, "invalid endpoint should throw std::out_of_range");
}

void test_overflow_boundaries() {
    const std::int64_t maximum = std::numeric_limits<std::int64_t>::max();
    const std::int64_t minimum = std::numeric_limits<std::int64_t>::min();
    const floyd_warshall::Result exact = floyd_warshall::compute(
        4, {{0, 1, maximum}, {2, 3, minimum}});
    require(exact.distances[0][1] == maximum,
            "exact maximum distance should remain representable");
    require(exact.distances[2][3] == minimum,
            "exact minimum distance should remain representable");

    bool positive_threw = false;
    try {
        static_cast<void>(floyd_warshall::compute(
            3, {{0, 1, maximum}, {1, 2, 1}}));
    } catch (const std::overflow_error&) {
        positive_threw = true;
    }
    require(positive_threw, "positive overflow should throw std::overflow_error");

    bool negative_threw = false;
    try {
        static_cast<void>(floyd_warshall::compute(
            3, {{0, 1, minimum}, {1, 2, -1}}));
    } catch (const std::overflow_error&) {
        negative_threw = true;
    }
    require(negative_threw, "negative overflow should throw std::overflow_error");

    const floyd_warshall::Result disconnected = floyd_warshall::compute(
        4, {{0, 1, maximum}, {2, 3, 1}});
    require(disconnected.distances[0][1] == maximum,
            "unrelated components must not create overflow candidates");
}

std::uint32_t next_random(std::uint32_t& state) {
    state = state * 1664525U + 1013904223U;
    return state;
}

void test_deterministic_random_graphs() {
    std::uint32_t state = 0xC0FFEEU;
    for (std::size_t trial = 0; trial < 500; ++trial) {
        const std::size_t vertex_count =
            static_cast<std::size_t>(next_random(state) % 8U);
        std::vector<floyd_warshall::Edge> edges;
        if (vertex_count != 0U) {
            const std::size_t edge_count =
                static_cast<std::size_t>(next_random(state) % 33U);
            edges.reserve(edge_count);
            for (std::size_t index = 0; index < edge_count; ++index) {
                const std::size_t from =
                    static_cast<std::size_t>(next_random(state) % vertex_count);
                const std::size_t to =
                    static_cast<std::size_t>(next_random(state) % vertex_count);
                const std::int64_t weight =
                    static_cast<std::int64_t>(next_random(state) % 19U) - 9;
                edges.push_back({from, to, weight});
            }
        }

        const floyd_warshall::Result actual =
            floyd_warshall::compute(vertex_count, edges);
        const floyd_warshall::Result expected =
            bellman_ford_reference(vertex_count, edges);
        require(actual.has_negative_cycle == expected.has_negative_cycle,
                "random graph negative-cycle result disagrees with reference");
        if (!expected.has_negative_cycle) {
            require(actual.distances == expected.distances,
                    "random graph distances disagree with Bellman-Ford reference");
        }
    }
}

}  // namespace

int main() {
    test_negative_edges_without_cycles();
    test_disconnected_vertices();
    test_parallel_and_multiple_optimal_paths();
    test_negative_cycles();
    test_empty_graph_and_invalid_endpoints();
    test_overflow_boundaries();
    test_deterministic_random_graphs();
    std::cout << "All Floyd-Warshall tests passed.\n";
    return 0;
}
