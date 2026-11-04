#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <limits>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace max_flow {

struct InputEdge {
    int from;
    int to;
    std::int64_t capacity;
};

// Computes a directed graph's maximum s-t flow with Edmonds-Karp.
// Parallel edges are kept as separate residual edge pairs. Source equal to
// sink has a maximum-flow value of zero, after all arguments are validated.
// Throws invalid_argument for an invalid vertex count or negative capacity,
// out_of_range for invalid vertices, and overflow_error if the result cannot
// be represented by int64_t.
std::int64_t edmondsKarp(int vertex_count,
                         const std::vector<InputEdge>& edges,
                         int source,
                         int sink) {
    if (vertex_count <= 0) {
        throw std::invalid_argument("vertex_count must be positive");
    }
    if (source < 0 || source >= vertex_count) {
        throw std::out_of_range("source vertex is out of range");
    }
    if (sink < 0 || sink >= vertex_count) {
        throw std::out_of_range("sink vertex is out of range");
    }
    for (const InputEdge& edge : edges) {
        if (edge.from < 0 || edge.from >= vertex_count || edge.to < 0 ||
            edge.to >= vertex_count) {
            throw std::out_of_range("edge endpoint is out of range");
        }
        if (edge.capacity < 0) {
            throw std::invalid_argument("edge capacity must be nonnegative");
        }
    }

    if (source == sink) {
        return 0;
    }

    struct ResidualEdge {
        int to;
        std::size_t reverse_index;
        std::int64_t residual_capacity;
    };
    std::vector<std::vector<ResidualEdge>> graph(
        static_cast<std::size_t>(vertex_count));

    const auto add_edge = [&graph](int from, int to, std::int64_t capacity) {
        const std::size_t forward_index =
            graph[static_cast<std::size_t>(from)].size();
        const std::size_t reverse_index =
            graph[static_cast<std::size_t>(to)].size();
        if (from == to) {
            graph[static_cast<std::size_t>(from)].push_back(
                {to, forward_index + 1U, capacity});
            graph[static_cast<std::size_t>(from)].push_back(
                {from, forward_index, 0});
        } else {
            graph[static_cast<std::size_t>(from)].push_back(
                {to, reverse_index, capacity});
            graph[static_cast<std::size_t>(to)].push_back(
                {from, forward_index, 0});
        }
    };

    for (const InputEdge& edge : edges) {
        add_edge(edge.from, edge.to, edge.capacity);
    }

    std::int64_t total_flow = 0;
    const std::size_t no_edge = std::numeric_limits<std::size_t>::max();
    std::vector<int> parent_vertex(static_cast<std::size_t>(vertex_count));
    std::vector<std::size_t> parent_edge(static_cast<std::size_t>(vertex_count));

    while (true) {
        std::fill(parent_vertex.begin(), parent_vertex.end(), -1);
        std::fill(parent_edge.begin(), parent_edge.end(), no_edge);
        std::queue<int> pending;
        parent_vertex[static_cast<std::size_t>(source)] = source;
        pending.push(source);

        while (!pending.empty() &&
               parent_vertex[static_cast<std::size_t>(sink)] == -1) {
            const int vertex = pending.front();
            pending.pop();
            const auto& adjacent = graph[static_cast<std::size_t>(vertex)];
            for (std::size_t edge_index = 0; edge_index < adjacent.size();
                 ++edge_index) {
                const ResidualEdge& edge = adjacent[edge_index];
                if (edge.residual_capacity > 0 &&
                    parent_vertex[static_cast<std::size_t>(edge.to)] == -1) {
                    parent_vertex[static_cast<std::size_t>(edge.to)] = vertex;
                    parent_edge[static_cast<std::size_t>(edge.to)] = edge_index;
                    pending.push(edge.to);
                    if (edge.to == sink) {
                        break;
                    }
                }
            }
        }

        if (parent_vertex[static_cast<std::size_t>(sink)] == -1) {
            break;
        }

        std::int64_t augment = std::numeric_limits<std::int64_t>::max();
        for (int vertex = sink; vertex != source;
             vertex = parent_vertex[static_cast<std::size_t>(vertex)]) {
            const int previous = parent_vertex[static_cast<std::size_t>(vertex)];
            const ResidualEdge& edge =
                graph[static_cast<std::size_t>(previous)]
                     [parent_edge[static_cast<std::size_t>(vertex)]];
            if (edge.residual_capacity < augment) {
                augment = edge.residual_capacity;
            }
        }

        if (augment > std::numeric_limits<std::int64_t>::max() - total_flow) {
            throw std::overflow_error("maximum flow exceeds int64_t range");
        }
        total_flow += augment;

        for (int vertex = sink; vertex != source;
             vertex = parent_vertex[static_cast<std::size_t>(vertex)]) {
            const int previous = parent_vertex[static_cast<std::size_t>(vertex)];
            const std::size_t edge_index =
                parent_edge[static_cast<std::size_t>(vertex)];
            ResidualEdge& edge =
                graph[static_cast<std::size_t>(previous)][edge_index];
            ResidualEdge& reverse =
                graph[static_cast<std::size_t>(vertex)][edge.reverse_index];
            edge.residual_capacity -= augment;
            // Each reverse edge is paired with one original edge, so its
            // residual capacity cannot exceed that edge's int64_t capacity.
            reverse.residual_capacity += augment;
        }
    }

    return total_flow;
}

}  // namespace max_flow

namespace {

void require(bool condition, const std::string& message) {
    if (!condition) {
        throw std::runtime_error("test failed: " + message);
    }
}

template <typename Exception, typename Function>
void require_throws(Function&& function, const std::string& message) {
    try {
        function();
    } catch (const Exception&) {
        return;
    }
    throw std::runtime_error("test failed (expected exception): " + message);
}

void test_standard_network() {
    const std::vector<max_flow::InputEdge> edges = {
        {0, 1, 16}, {0, 2, 13}, {1, 2, 10}, {2, 1, 4}, {1, 3, 12},
        {3, 2, 9},  {2, 4, 14}, {4, 3, 7},  {3, 5, 20}, {4, 5, 4}};
    require(max_flow::edmondsKarp(6, edges, 0, 5) == 23,
            "standard network has max flow 23");
}

void test_disconnected_sink() {
    const std::vector<max_flow::InputEdge> edges = {{0, 1, 9}, {1, 2, 3}};
    require(max_flow::edmondsKarp(4, edges, 0, 3) == 0,
            "disconnected sink has max flow 0");
}

void test_parallel_and_zero_capacity_edges() {
    const std::vector<max_flow::InputEdge> edges = {
        {0, 1, 3}, {0, 1, 5}, {0, 1, 0}, {1, 2, 10}, {0, 2, 0}};
    require(max_flow::edmondsKarp(3, edges, 0, 2) == 8,
            "parallel capacities sum and zero-capacity edges are ignored");
}

void test_reverse_residual_rerouting() {
    // BFS first uses 0->1->3->5. The only way to obtain the second unit is
    // through the reverse residual edge 3->1, then 1->4->5.
    const std::vector<max_flow::InputEdge> edges = {
        {0, 1, 1}, {0, 2, 1}, {1, 3, 1}, {1, 4, 1},
        {2, 3, 1}, {3, 5, 1}, {4, 5, 1}};
    require(max_flow::edmondsKarp(6, edges, 0, 5) == 2,
            "reverse residual edge enables rerouting");
}

void test_invalid_inputs() {
    require_throws<std::invalid_argument>(
        [] { (void)max_flow::edmondsKarp(0, {}, 0, 0); },
        "nonpositive vertex count");
    require_throws<std::out_of_range>(
        [] { (void)max_flow::edmondsKarp(2, {}, -1, 1); }, "invalid source");
    require_throws<std::out_of_range>(
        [] { (void)max_flow::edmondsKarp(2, {}, 0, 2); }, "invalid sink");
    require_throws<std::out_of_range>(
        [] { (void)max_flow::edmondsKarp(2, {{0, 2, 1}}, 0, 1); },
        "invalid edge endpoint");
    require_throws<std::invalid_argument>(
        [] { (void)max_flow::edmondsKarp(2, {{0, 1, -1}}, 0, 1); },
        "negative capacity");
}

void test_source_equals_sink() {
    require(max_flow::edmondsKarp(2, {{0, 1, 7}}, 1, 1) == 0,
            "source equal to sink has flow zero");
}

void test_flow_overflow_guard() {
    const std::int64_t maximum = std::numeric_limits<std::int64_t>::max();
    const std::vector<max_flow::InputEdge> edges = {
        {0, 1, maximum}, {0, 1, 1}};
    require_throws<std::overflow_error>(
        [&edges] { (void)max_flow::edmondsKarp(2, edges, 0, 1); },
        "flow sum beyond int64_t");
}

}  // namespace

int main() {
    try {
        test_standard_network();
        test_disconnected_sink();
        test_parallel_and_zero_capacity_edges();
        test_reverse_residual_rerouting();
        test_invalid_inputs();
        test_source_equals_sink();
        test_flow_overflow_guard();
        std::cout << "All Edmonds-Karp tests passed.\n";
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
    return 0;
}

/* Complexity: O(V E^2) time and O(V + E) storage, counting residual edges. */
