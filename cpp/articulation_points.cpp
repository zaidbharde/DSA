#include <algorithm>
#include <cstddef>
#include <exception>
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

    struct DfsFrame {
        int vertex;
        std::size_t parent_edge_id;
        std::size_t next_edge;
        std::size_t tree_children;
    };
    std::vector<DfsFrame> stack;
    stack.reserve(static_cast<std::size_t>(vertex_count));

    for (int vertex = 0; vertex < vertex_count; ++vertex) {
        const std::size_t root_index = static_cast<std::size_t>(vertex);
        if (discovery[root_index] != 0) {
            continue;
        }

        discovery[root_index] = low[root_index] = ++time;
        stack.push_back({vertex, no_parent_edge, 0, 0});
        while (!stack.empty()) {
            DfsFrame& frame = stack.back();
            const std::size_t index = static_cast<std::size_t>(frame.vertex);
            if (frame.next_edge < adjacency[index].size()) {
                const auto [neighbor, edge_id] = adjacency[index][frame.next_edge++];
                if (edge_id == frame.parent_edge_id) {
                    continue;
                }

                const std::size_t neighbor_index = static_cast<std::size_t>(neighbor);
                if (discovery[neighbor_index] == 0) {
                    ++frame.tree_children;
                    discovery[neighbor_index] = low[neighbor_index] = ++time;
                    stack.push_back({neighbor, edge_id, 0, 0});
                } else {
                    // Skip only the exact tree edge: another parallel edge is a
                    // back edge and must lower the child's low-link value.
                    low[index] = std::min(low[index], discovery[neighbor_index]);
                }
                continue;
            }

            const DfsFrame finished = frame;
            stack.pop_back();
            const std::size_t finished_index =
                static_cast<std::size_t>(finished.vertex);
            if (finished.parent_edge_id == no_parent_edge) {
                if (finished.tree_children > 1) {
                    is_articulation[finished_index] = true;
                }
                continue;
            }

            // The parent frame remains on the stack until this child's DFS
            // finishes, so post-order low-link propagation is explicit.
            DfsFrame& parent = stack.back();
            const std::size_t parent_index = static_cast<std::size_t>(parent.vertex);
            low[parent_index] = std::min(low[parent_index], low[finished_index]);
            if (parent.parent_edge_id != no_parent_edge &&
                low[finished_index] >= discovery[parent_index]) {
                is_articulation[parent_index] = true;
            }
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

void testDeepChain() {
    constexpr int vertex_count = 50000;
    std::vector<articulation_points::Edge> edges;
    edges.reserve(static_cast<std::size_t>(vertex_count - 1));
    for (int vertex = 0; vertex + 1 < vertex_count; ++vertex) {
        edges.emplace_back(vertex, vertex + 1);
    }

    std::vector<int> expected;
    expected.reserve(static_cast<std::size_t>(vertex_count - 2));
    for (int vertex = 1; vertex + 1 < vertex_count; ++vertex) {
        expected.push_back(vertex);
    }
    require(articulation_points::findArticulationPoints(vertex_count, edges) == expected,
            "deep chain should find every internal vertex without recursion");
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

void testRootChildCountRule() {
    require(articulation_points::findArticulationPoints(2, {{0, 1}}).empty(),
            "DFS root with one tree child must not be an articulation vertex");
    require(articulation_points::findArticulationPoints(
                5, {{0, 1}, {0, 2}, {0, 3}, {0, 4}}) == std::vector<int>({0}),
            "DFS root with multiple tree children must be an articulation vertex");
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

    threw = false;
    try {
        static_cast<void>(articulation_points::findArticulationPoints(2, {{-1, 0}}));
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    require(threw, "negative endpoint should throw std::invalid_argument");
}

}  // namespace

int main() {
    try {
        testChain();
        testDeepChain();
        testCycle();
        testStar();
        testRootChildCountRule();
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
