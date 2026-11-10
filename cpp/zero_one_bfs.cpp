#include <cassert>
#include <cstddef>
#include <deque>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

// Directed edge for 0-1 BFS. The weight must be exactly 0 or 1.
struct Edge {
    int from;
    int to;
    int weight;
};

// Unreachable vertices are represented by INF.
constexpr long long INF = std::numeric_limits<long long>::max();

// Computes shortest-path distances from source in a directed graph with
// edge weights restricted to 0 and 1. Throws std::invalid_argument if the
// vertex count, source, an endpoint, or an edge weight is invalid.
//
// Complexity: O(V + E) time and O(V + E) space.
std::vector<long long> zero_one_bfs(int vertex_count,
                                    const std::vector<Edge>& edges,
                                    int source) {
    if (vertex_count < 0) {
        throw std::invalid_argument("vertex count must be non-negative");
    }
    if (source < 0 || source >= vertex_count) {
        throw std::invalid_argument("source is outside the graph");
    }

    std::vector<std::vector<std::pair<int, int>>> adjacency(
        static_cast<std::size_t>(vertex_count));
    for (const Edge& edge : edges) {
        if (edge.from < 0 || edge.from >= vertex_count || edge.to < 0 ||
            edge.to >= vertex_count) {
            throw std::invalid_argument("edge endpoint is outside the graph");
        }
        if (edge.weight != 0 && edge.weight != 1) {
            throw std::invalid_argument("edge weight must be 0 or 1");
        }
        adjacency[static_cast<std::size_t>(edge.from)].emplace_back(
            edge.to, edge.weight);
    }

    std::vector<long long> distance(static_cast<std::size_t>(vertex_count), INF);
    std::deque<std::pair<long long, int>> queue;
    distance[static_cast<std::size_t>(source)] = 0;
    queue.emplace_front(0, source);

    while (!queue.empty()) {
        const auto [queued_distance, vertex] = queue.front();
        queue.pop_front();
        if (queued_distance != distance[static_cast<std::size_t>(vertex)]) {
            continue;
        }

        for (const auto& [neighbor, weight] :
             adjacency[static_cast<std::size_t>(vertex)]) {
            const long long candidate = queued_distance + weight;
            long long& neighbor_distance =
                distance[static_cast<std::size_t>(neighbor)];
            if (candidate < neighbor_distance) {
                neighbor_distance = candidate;
                if (weight == 0) {
                    queue.emplace_front(candidate, neighbor);
                } else {
                    queue.emplace_back(candidate, neighbor);
                }
            }
        }
    }

    return distance;
}

namespace {

void expect_invalid_argument(void (*operation)()) {
    bool threw = false;
    try {
        operation();
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    assert(threw);
}

void test_zero_cost_chain() {
    const std::vector<Edge> edges{{0, 1, 0}, {1, 2, 0}, {2, 3, 0}};
    const std::vector<long long> expected{0, 0, 0, 0};
    assert(zero_one_bfs(4, edges, 0) == expected);
}

void test_mixed_paths() {
    const std::vector<Edge> edges{{0, 1, 1}, {0, 2, 0}, {2, 1, 0},
                                  {1, 3, 1}, {2, 3, 1}, {3, 4, 0}};
    const std::vector<long long> expected{0, 0, 0, 1, 1};
    assert(zero_one_bfs(5, edges, 0) == expected);
}

void test_cycles() {
    const std::vector<Edge> edges{{0, 1, 1}, {1, 2, 0}, {2, 0, 0},
                                  {2, 3, 1}, {3, 1, 0}};
    const std::vector<long long> expected{0, 1, 1, 2};
    assert(zero_one_bfs(4, edges, 0) == expected);
}

void test_disconnected_vertices() {
    const std::vector<Edge> edges{{0, 1, 1}, {2, 3, 0}};
    const std::vector<long long> expected{0, 1, INF, INF, INF};
    assert(zero_one_bfs(5, edges, 0) == expected);
}

void test_source_to_source() {
    const std::vector<Edge> edges{{0, 1, 1}, {1, 2, 0}};
    const std::vector<long long> expected{INF, INF, 0, INF};
    assert(zero_one_bfs(4, edges, 2) == expected);
}

void invalid_endpoint_test() {
    const std::vector<Edge> edges{{0, 2, 1}};
    (void)zero_one_bfs(2, edges, 0);
}

void invalid_weight_test() {
    const std::vector<Edge> edges{{0, 1, 2}};
    (void)zero_one_bfs(2, edges, 0);
}

void invalid_vertex_count_test() {
    (void)zero_one_bfs(-1, {}, 0);
}

void invalid_source_test() {
    (void)zero_one_bfs(2, {}, 2);
}

}  // namespace

int main() {
    test_zero_cost_chain();
    test_mixed_paths();
    test_cycles();
    test_disconnected_vertices();
    test_source_to_source();
    expect_invalid_argument(invalid_endpoint_test);
    expect_invalid_argument(invalid_weight_test);
    expect_invalid_argument(invalid_vertex_count_test);
    expect_invalid_argument(invalid_source_test);
    std::cout << "All 0-1 BFS tests passed.\n";
    return 0;
}
