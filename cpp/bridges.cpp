#include <algorithm>
#include <cstddef>
#include <exception>
#include <functional>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

/*
 * Finds bridges in an undirected graph supplied as an edge list. Vertex IDs
 * are in [0, vertex_count); invalid counts or endpoints throw
 * std::invalid_argument. Parallel edges and self-loops are supported by
 * tracking the identity of each input edge. Returned endpoint pairs are
 * normalized (smaller endpoint first) and sorted lexicographically.
 *
 * Time: O(V + E + B log B), where E counts input edge entries and B is the
 * number of bridges (the final term sorts the returned bridge list).
 * Space: O(V + E + B), including adjacency, DFS state, and returned bridges.
 */
namespace bridges {

using Edge = std::pair<int, int>;

std::vector<Edge> findBridges(int vertex_count, const std::vector<Edge>& edges) {
    if (vertex_count < 0) {
        throw std::invalid_argument("vertex count must be nonnegative");
    }

    using AdjacentEdge = std::pair<int, std::size_t>;
    std::vector<std::vector<AdjacentEdge>> adjacency(
        static_cast<std::size_t>(vertex_count));
    for (std::size_t edge_id = 0; edge_id < edges.size(); ++edge_id) {
        const auto [from, to] = edges[edge_id];
        if (from < 0 || from >= vertex_count || to < 0 || to >= vertex_count) {
            throw std::invalid_argument("edge endpoint is outside the graph");
        }
        adjacency[static_cast<std::size_t>(from)].emplace_back(to, edge_id);
        adjacency[static_cast<std::size_t>(to)].emplace_back(from, edge_id);
    }

    std::vector<std::size_t> discovery(static_cast<std::size_t>(vertex_count), 0);
    std::vector<std::size_t> low(static_cast<std::size_t>(vertex_count), 0);
    std::vector<Edge> result;
    std::size_t time = 0;
    const std::size_t no_parent_edge = std::numeric_limits<std::size_t>::max();

    std::function<void(int, std::size_t)> dfs = [&](int vertex,
                                                    std::size_t parent_edge_id) {
        const std::size_t index = static_cast<std::size_t>(vertex);
        discovery[index] = low[index] = ++time;

        for (const auto& [neighbor, edge_id] : adjacency[index]) {
            if (edge_id == parent_edge_id) {
                continue;
            }

            const std::size_t neighbor_index = static_cast<std::size_t>(neighbor);
            if (discovery[neighbor_index] == 0) {
                dfs(neighbor, edge_id);
                low[index] = std::min(low[index], low[neighbor_index]);
                if (low[neighbor_index] > discovery[index]) {
                    result.emplace_back(std::min(vertex, neighbor),
                                        std::max(vertex, neighbor));
                }
            } else {
                low[index] = std::min(low[index], discovery[neighbor_index]);
            }
        }
    };

    for (int vertex = 0; vertex < vertex_count; ++vertex) {
        if (discovery[static_cast<std::size_t>(vertex)] == 0) {
            dfs(vertex, no_parent_edge);
        }
    }

    std::sort(result.begin(), result.end());
    return result;
}

}  // namespace bridges

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void testChain() {
    const std::vector<bridges::Edge> edges{{0, 1}, {1, 2}, {2, 3}};
    require(bridges::findBridges(4, edges) ==
                std::vector<bridges::Edge>({{0, 1}, {1, 2}, {2, 3}}),
            "every edge in a chain should be a bridge");
}

void testCycle() {
    const std::vector<bridges::Edge> edges{
        {0, 1}, {1, 2}, {2, 3}, {3, 0}};
    require(bridges::findBridges(4, edges).empty(),
            "cycle should have no bridges");
}

void testStar() {
    const std::vector<bridges::Edge> edges{
        {0, 1}, {0, 2}, {0, 3}, {0, 4}};
    require(bridges::findBridges(5, edges) ==
                std::vector<bridges::Edge>({{0, 1}, {0, 2}, {0, 3}, {0, 4}}),
            "every edge in a star should be a bridge");
}

void testDisconnectedGraph() {
    const std::vector<bridges::Edge> edges{
        {0, 1}, {1, 2}, {2, 0}, {3, 4}, {5, 6}};
    require(bridges::findBridges(8, edges) ==
                std::vector<bridges::Edge>({{3, 4}, {5, 6}}),
            "disconnected graph should find bridges in each component");
}

void testParallelEdges() {
    const std::vector<bridges::Edge> edges{{0, 1}, {0, 1}};
    require(bridges::findBridges(2, edges).empty(),
            "parallel edges should prevent either edge from being a bridge");
}

void testSelfLoop() {
    const std::vector<bridges::Edge> edges{{0, 1}, {1, 2}, {2, 2}};
    require(bridges::findBridges(3, edges) ==
                std::vector<bridges::Edge>({{0, 1}, {1, 2}}),
            "self-loop should not affect bridges in the chain");
}

void testIsolatedVerticesAndEmptyGraph() {
    require(bridges::findBridges(4, {}).empty(),
            "isolated vertices should not produce bridges");
    require(bridges::findBridges(0, {}).empty(),
            "empty graph should have no bridges");
}

void testInvalidEndpoint() {
    bool threw = false;
    try {
        static_cast<void>(bridges::findBridges(2, {{0, 2}}));
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    require(threw, "invalid endpoint should throw std::invalid_argument");
}

void testInvalidVertexCount() {
    bool threw = false;
    try {
        static_cast<void>(bridges::findBridges(-1, {}));
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    require(threw, "negative vertex count should throw std::invalid_argument");
}

void testNormalizedAndSortedOutput() {
    const std::vector<bridges::Edge> edges{{3, 2}, {2, 0}, {4, 1}};
    require(bridges::findBridges(5, edges) ==
                std::vector<bridges::Edge>({{0, 2}, {1, 4}, {2, 3}}),
            "bridges should be normalized and sorted lexicographically");
}

}  // namespace

int main() {
    try {
        testChain();
        testCycle();
        testStar();
        testDisconnectedGraph();
        testParallelEdges();
        testSelfLoop();
        testIsolatedVerticesAndEmptyGraph();
        testInvalidEndpoint();
        testInvalidVertexCount();
        testNormalizedAndSortedOutput();
        std::cout << "All bridge tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Bridge test failure: " << error.what() << '\n';
        return 1;
    }
}
