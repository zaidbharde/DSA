#include <cstddef>
#include <cstdint>
#include <exception>
#include <iostream>
#include <limits>
#include <queue>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

/*
 * Prim minimum spanning forest for an undirected weighted adjacency list.
 *
 * The adjacency list must describe an undirected graph (each edge should be
 * present in both endpoint lists). Negative edge weights are supported.
 * Invalid endpoints throw std::invalid_argument; a total that cannot be
 * represented by int64_t throws std::overflow_error.
 *
 * Time: O((V + E) log V), where E counts undirected edges.
 * Space: O(V + E) including the supplied adjacency list and result.
 */
namespace prim_mst {

struct Edge {
    std::size_t to;
    std::int64_t weight;
};

struct SelectedEdge {
    std::size_t from;
    std::size_t to;
    std::int64_t weight;
};

struct Result {
    std::int64_t total_weight;
    std::size_t component_count;
    std::vector<SelectedEdge> selected_edges;
};

namespace detail {

std::int64_t checkedAdd(std::int64_t lhs, std::int64_t rhs) {
    const std::int64_t maximum = std::numeric_limits<std::int64_t>::max();
    const std::int64_t minimum = std::numeric_limits<std::int64_t>::min();
    if ((rhs > 0 && lhs > maximum - rhs) ||
        (rhs < 0 && lhs < minimum - rhs)) {
        throw std::overflow_error("Prim forest total weight overflows int64_t");
    }
    return lhs + rhs;
}

}  // namespace detail

Result minimumSpanningForest(const std::vector<std::vector<Edge>>& adjacency) {
    const std::size_t vertex_count = adjacency.size();
    for (const auto& neighbors : adjacency) {
        for (const Edge& edge : neighbors) {
            if (edge.to >= vertex_count) {
                throw std::invalid_argument("adjacency list contains an invalid endpoint");
            }
        }
    }

    using Candidate = std::tuple<std::int64_t, std::size_t, std::size_t>;
    std::priority_queue<Candidate, std::vector<Candidate>, std::greater<Candidate>> candidates;
    std::vector<bool> visited(vertex_count, false);
    Result result{0, 0, {}};

    for (std::size_t root = 0; root < vertex_count; ++root) {
        if (visited[root]) {
            continue;
        }

        ++result.component_count;
        visited[root] = true;
        for (const Edge& edge : adjacency[root]) {
            if (!visited[edge.to]) {
                candidates.emplace(edge.weight, root, edge.to);
            }
        }

        while (!candidates.empty()) {
            const auto [weight, from, to] = candidates.top();
            candidates.pop();
            if (visited[to]) {
                continue;
            }

            visited[to] = true;
            result.total_weight = detail::checkedAdd(result.total_weight, weight);
            result.selected_edges.push_back(SelectedEdge{from, to, weight});
            for (const Edge& edge : adjacency[to]) {
                if (!visited[edge.to]) {
                    candidates.emplace(edge.weight, to, edge.to);
                }
            }
        }
    }

    return result;
}

}  // namespace prim_mst

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void addUndirectedEdge(std::vector<std::vector<prim_mst::Edge>>& adjacency,
                       std::size_t from,
                       std::size_t to,
                       std::int64_t weight) {
    adjacency[from].push_back(prim_mst::Edge{to, weight});
    adjacency[to].push_back(prim_mst::Edge{from, weight});
}

void testConnectedGraph() {
    std::vector<std::vector<prim_mst::Edge>> graph(4);
    addUndirectedEdge(graph, 0, 1, 1);
    addUndirectedEdge(graph, 0, 2, 4);
    addUndirectedEdge(graph, 1, 2, 2);
    addUndirectedEdge(graph, 1, 3, 5);
    addUndirectedEdge(graph, 2, 3, 3);

    const prim_mst::Result result = prim_mst::minimumSpanningForest(graph);
    require(result.total_weight == 6, "connected graph total should be 6");
    require(result.component_count == 1, "connected graph should have one component");
    require(result.selected_edges.size() == 3, "connected graph should select V-1 edges");
}

void testDisconnectedGraph() {
    std::vector<std::vector<prim_mst::Edge>> graph(5);
    addUndirectedEdge(graph, 0, 1, 4);
    addUndirectedEdge(graph, 1, 2, 1);
    addUndirectedEdge(graph, 3, 4, 2);

    const prim_mst::Result result = prim_mst::minimumSpanningForest(graph);
    require(result.total_weight == 7, "disconnected forest total should be 7");
    require(result.component_count == 2, "disconnected graph should have two components");
    require(result.selected_edges.size() == 3, "forest should select V-components edges");
}

void testNegativeWeights() {
    std::vector<std::vector<prim_mst::Edge>> graph(3);
    addUndirectedEdge(graph, 0, 1, -5);
    addUndirectedEdge(graph, 1, 2, 2);
    addUndirectedEdge(graph, 0, 2, 1);

    const prim_mst::Result result = prim_mst::minimumSpanningForest(graph);
    require(result.total_weight == -4, "negative-weight graph total should be -4");
    require(result.component_count == 1, "negative-weight graph should be connected");
    require(result.selected_edges.size() == 2, "negative-weight graph should select two edges");
}

void testSingletonAndEmptyGraphs() {
    const std::vector<std::vector<prim_mst::Edge>> singleton(1);
    const prim_mst::Result singleton_result = prim_mst::minimumSpanningForest(singleton);
    require(singleton_result.total_weight == 0, "singleton total should be zero");
    require(singleton_result.component_count == 1, "singleton should count as one component");
    require(singleton_result.selected_edges.empty(), "singleton should select no edges");

    const std::vector<std::vector<prim_mst::Edge>> empty;
    const prim_mst::Result empty_result = prim_mst::minimumSpanningForest(empty);
    require(empty_result.total_weight == 0, "empty graph total should be zero");
    require(empty_result.component_count == 0, "empty graph should have zero components");
    require(empty_result.selected_edges.empty(), "empty graph should select no edges");
}

void testInvalidEndpoint() {
    std::vector<std::vector<prim_mst::Edge>> graph(2);
    graph[0].push_back(prim_mst::Edge{2, 1});

    bool threw = false;
    try {
        static_cast<void>(prim_mst::minimumSpanningForest(graph));
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    require(threw, "invalid endpoint should throw std::invalid_argument");
}

void testTotalWeightOverflow() {
    std::vector<std::vector<prim_mst::Edge>> graph(3);
    addUndirectedEdge(graph, 0, 1, std::numeric_limits<std::int64_t>::max());
    addUndirectedEdge(graph, 1, 2, 1);

    bool threw = false;
    try {
        static_cast<void>(prim_mst::minimumSpanningForest(graph));
    } catch (const std::overflow_error&) {
        threw = true;
    }
    require(threw, "overflowing total should throw std::overflow_error");
}

}  // namespace

int main() {
    try {
        testConnectedGraph();
        testDisconnectedGraph();
        testNegativeWeights();
        testSingletonAndEmptyGraphs();
        testInvalidEndpoint();
        testTotalWeightOverflow();
        std::cout << "All Prim minimum spanning forest tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Prim minimum spanning forest test failure: " << error.what() << '\n';
        return 1;
    }
}
