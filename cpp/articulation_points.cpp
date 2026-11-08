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
 * Finds articulation vertices in an undirected graph supplied as an edge list.
 * Vertex IDs are in [0, vertex_count); invalid counts or endpoints throw
 * std::invalid_argument. Parallel edges and self-loops are supported.
 * The returned vertex IDs are sorted and unique.
 *
 * Time: O(V + E), where E counts edge-list entries (including parallel edges).
 * Space: O(V + E), including the built adjacency list and DFS state.
 */
namespace articulation_points {

using Edge = std::pair<int, int>;

std::vector<int> findArticulationPoints(int vertex_count,
                                        const std::vector<Edge>& edges) {
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
    std::vector<bool> is_articulation(static_cast<std::size_t>(vertex_count), false);
    std::size_t time = 0;
    const std::size_t no_parent_edge = std::numeric_limits<std::size_t>::max();

    std::function<void(int, std::size_t)> dfs = [&](int vertex,
                                                    std::size_t parent_edge_id) {
        const std::size_t index = static_cast<std::size_t>(vertex);
        discovery[index] = low[index] = ++time;
        std::size_t tree_children = 0;

        for (const auto& [neighbor, edge_id] : adjacency[index]) {
            if (edge_id == parent_edge_id) {
                continue;
            }

            const std::size_t neighbor_index = static_cast<std::size_t>(neighbor);
            if (discovery[neighbor_index] == 0) {
                ++tree_children;
                dfs(neighbor, edge_id);
                low[index] = std::min(low[index], low[neighbor_index]);

                if (parent_edge_id != no_parent_edge &&
                    low[neighbor_index] >= discovery[index]) {
                    is_articulation[index] = true;
                }
            } else {
                low[index] = std::min(low[index], discovery[neighbor_index]);
            }
        }

        if (parent_edge_id == no_parent_edge && tree_children > 1) {
            is_articulation[index] = true;
        }
    };

    for (int vertex = 0; vertex < vertex_count; ++vertex) {
        if (discovery[static_cast<std::size_t>(vertex)] == 0) {
            dfs(vertex, no_parent_edge);
        }
    }

    std::vector<int> result;
    for (int vertex = 0; vertex < vertex_count; ++vertex) {
        if (is_articulation[static_cast<std::size_t>(vertex)]) {
            result.push_back(vertex);
        }
    }
    return result;
}

}  // namespace articulation_points

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void testChain() {
    const std::vector<articulation_points::Edge> edges{{0, 1}, {1, 2}, {2, 3}};
    require(articulation_points::findArticulationPoints(4, edges) ==
                std::vector<int>({1, 2}),
            "chain should have articulation vertices 1 and 2");
}

void testCycle() {
    const std::vector<articulation_points::Edge> edges{
        {0, 1}, {1, 2}, {2, 3}, {3, 0}};
    require(articulation_points::findArticulationPoints(4, edges).empty(),
            "cycle should have no articulation vertices");
}

void testStar() {
    const std::vector<articulation_points::Edge> edges{
        {0, 1}, {0, 2}, {0, 3}, {0, 4}};
    require(articulation_points::findArticulationPoints(5, edges) ==
                std::vector<int>({0}),
            "star center should be the only articulation vertex");
}

void testDisconnectedGraph() {
    const std::vector<articulation_points::Edge> edges{
        {0, 1}, {1, 2}, {2, 3}, {4, 5}, {5, 6}, {6, 4}};
    require(articulation_points::findArticulationPoints(8, edges) ==
                std::vector<int>({1, 2}),
            "disconnected graph should find cut vertices and ignore cycle and isolate");
}

void testParallelEdges() {
    const std::vector<articulation_points::Edge> edges{{0, 1}, {0, 1}};
    require(articulation_points::findArticulationPoints(2, edges).empty(),
            "parallel edges between two vertices should create no articulation vertex");
}

void testSelfLoop() {
    const std::vector<articulation_points::Edge> edges{{0, 1}, {1, 2}, {2, 2}};
    require(articulation_points::findArticulationPoints(3, edges) ==
                std::vector<int>({1}),
            "self-loop should not change the articulation point in a chain");
}

void testIsolatedVerticesAndEmptyGraph() {
    require(articulation_points::findArticulationPoints(4, {}).empty(),
            "isolated vertices should not be articulation vertices");
    require(articulation_points::findArticulationPoints(0, {}).empty(),
            "empty graph should have no articulation vertices");
}

void testInvalidEndpoint() {
    bool threw = false;
    try {
        static_cast<void>(articulation_points::findArticulationPoints(2, {{0, 2}}));
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    require(threw, "invalid endpoint should throw std::invalid_argument");
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
        std::cout << "All articulation-point tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Articulation-point test failure: " << error.what() << '\n';
        return 1;
    }
}
