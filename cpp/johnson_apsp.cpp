#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iostream>
#include <limits>
#include <optional>
#include <queue>
#include <stdexcept>
#include <utility>
#include <vector>

namespace johnson_apsp {

/** A directed, weighted edge. Vertices are numbered [0, vertex_count). */
struct Edge {
    std::size_t from;
    std::size_t to;
    std::int64_t weight;
};

/** A missing entry in distances means that the ordered vertex pair is unreachable. */
struct Result {
    std::vector<std::vector<std::optional<std::int64_t>>> distances;
};

std::int64_t checked_add(std::int64_t lhs, std::int64_t rhs) {
    constexpr std::int64_t minimum = std::numeric_limits<std::int64_t>::min();
    constexpr std::int64_t maximum = std::numeric_limits<std::int64_t>::max();
    if ((rhs > 0 && lhs > maximum - rhs) ||
        (rhs < 0 && lhs < minimum - rhs)) {
        throw std::overflow_error("Johnson distance or reweight addition overflow");
    }
    return lhs + rhs;
}

std::int64_t checked_subtract(std::int64_t lhs, std::int64_t rhs) {
    constexpr std::int64_t minimum = std::numeric_limits<std::int64_t>::min();
    constexpr std::int64_t maximum = std::numeric_limits<std::int64_t>::max();
    if ((rhs > 0 && lhs < minimum + rhs) ||
        (rhs < 0 && lhs > maximum + rhs)) {
        throw std::overflow_error("Johnson distance or reweight subtraction overflow");
    }
    return lhs - rhs;
}

/**
 * Computes all-pairs shortest-path distances using Johnson's algorithm.
 * Parallel edges are retained and naturally considered by relaxation;
 * unreachable pairs are represented by std::nullopt. A graph with any
 * negative cycle throws std::domain_error, including a cycle in a component
 * disconnected from the first vertex. Invalid edge endpoints throw
 * std::out_of_range.
 *
 * All arithmetic is checked. std::overflow_error is thrown if a potential,
 * reduced edge weight, Dijkstra candidate, or restored distance requires an
 * intermediate value outside int64_t. The two operations of a reweight or
 * distance restoration are checked individually, so an unrepresentable
 * intermediate is an error even if a later operation could bring the final
 * mathematical expression back into range.
 *
 * With E counting parallel input edges, Bellman-Ford takes O(VE). Each
 * priority-queue Dijkstra run takes O(V + E log(E+1)) in the multigraph case,
 * including distance initialization, giving O(VE + V^2 + VE log(E+1)) total
 * time. For simple graphs, E=O(V^2) and this is commonly expressed as
 * O(VE + V E log V), with the O(V^2) initialization term implicit. Storage is
 * O(V^2 + E + V), including the output matrix and adjacency list. The
 * caller-owned input edge list is not copied.
 */
Result compute(std::size_t vertex_count, const std::vector<Edge>& edges) {
    for (const Edge& edge : edges) {
        if (edge.from >= vertex_count || edge.to >= vertex_count) {
            throw std::out_of_range("edge endpoint is outside the graph");
        }
    }

    // Equivalent to adding a zero-weight super-source edge to every vertex.
    // Initializing all potentials to zero detects negative cycles in every
    // component, not only in the component of an arbitrary source.
    std::vector<std::int64_t> potential(vertex_count, 0);
    for (std::size_t pass = 0; pass < vertex_count; ++pass) {
        bool changed = false;
        for (const Edge& edge : edges) {
            const std::int64_t candidate =
                checked_add(potential[edge.from], edge.weight);
            if (candidate < potential[edge.to]) {
                potential[edge.to] = candidate;
                changed = true;
            }
        }
        if (!changed) {
            break;
        }
        if (pass + 1U == vertex_count) {
            throw std::domain_error("graph contains a negative-weight cycle");
        }
    }

    struct ReweightedEdge {
        std::size_t to;
        std::int64_t weight;
    };
    std::vector<std::vector<ReweightedEdge>> adjacency(vertex_count);
    for (const Edge& edge : edges) {
        const std::int64_t with_source_potential =
            checked_add(edge.weight, potential[edge.from]);
        const std::int64_t reweighted =
            checked_subtract(with_source_potential, potential[edge.to]);
        if (reweighted < 0) {
            // This should be impossible if Bellman-Ford's potential invariant holds.
            throw std::logic_error("Johnson reweighting produced a negative edge");
        }
        adjacency[edge.from].push_back({edge.to, reweighted});
    }

    Result result{std::vector<std::vector<std::optional<std::int64_t>>>(
        vertex_count,
        std::vector<std::optional<std::int64_t>>(vertex_count, std::nullopt))};
    using QueueEntry = std::pair<std::int64_t, std::size_t>;

    for (std::size_t source = 0; source < vertex_count; ++source) {
        std::vector<std::optional<std::int64_t>> reweighted_distances(vertex_count);
        std::priority_queue<QueueEntry, std::vector<QueueEntry>,
                            std::greater<QueueEntry>> queue;
        reweighted_distances[source] = 0;
        queue.push({0, source});

        while (!queue.empty()) {
            const QueueEntry current = queue.top();
            queue.pop();
            const std::int64_t distance = current.first;
            const std::size_t vertex = current.second;
            if (!reweighted_distances[vertex].has_value() ||
                distance != *reweighted_distances[vertex]) {
                continue;
            }
            for (const ReweightedEdge& edge : adjacency[vertex]) {
                const std::int64_t candidate = checked_add(distance, edge.weight);
                std::optional<std::int64_t>& best = reweighted_distances[edge.to];
                if (!best.has_value() || candidate < *best) {
                    best = candidate;
                    queue.push({candidate, edge.to});
                }
            }
        }

        for (std::size_t target = 0; target < vertex_count; ++target) {
            if (!reweighted_distances[target].has_value()) {
                continue;
            }
            const std::int64_t remove_source_potential = checked_subtract(
                *reweighted_distances[target], potential[source]);
            result.distances[source][target] = checked_add(
                remove_source_potential, potential[target]);
        }
    }
    return result;
}

}  // namespace johnson_apsp

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

struct ReferenceResult {
    std::vector<std::vector<std::optional<std::int64_t>>> distances;
    bool has_negative_cycle;
};

// Independent Floyd-Warshall reference for small bounded-weight test graphs.
ReferenceResult floyd_warshall_reference(
    std::size_t vertex_count, const std::vector<johnson_apsp::Edge>& edges) {
    std::vector<std::vector<std::optional<std::int64_t>>> distances(
        vertex_count,
        std::vector<std::optional<std::int64_t>>(vertex_count, std::nullopt));
    for (std::size_t vertex = 0; vertex < vertex_count; ++vertex) {
        distances[vertex][vertex] = 0;
    }
    for (const johnson_apsp::Edge& edge : edges) {
        std::optional<std::int64_t>& current = distances[edge.from][edge.to];
        if (!current.has_value() || edge.weight < *current) {
            current = edge.weight;
        }
    }
    for (std::size_t middle = 0; middle < vertex_count; ++middle) {
        for (std::size_t from = 0; from < vertex_count; ++from) {
            if (!distances[from][middle].has_value()) {
                continue;
            }
            for (std::size_t to = 0; to < vertex_count; ++to) {
                if (!distances[middle][to].has_value()) {
                    continue;
                }
                const std::int64_t candidate =
                    *distances[from][middle] + *distances[middle][to];
                std::optional<std::int64_t>& best = distances[from][to];
                if (!best.has_value() || candidate < *best) {
                    best = candidate;
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

std::uint32_t next_random(std::uint32_t& state) {
    state = state * 1664525U + 1013904223U;
    return state;
}

void test_reference_cases() {
    const std::vector<std::vector<johnson_apsp::Edge>> cases{
        {{0, 1, 4}, {0, 2, 5}, {1, 2, -3}, {2, 3, 2}},
        {{0, 1, 7}, {2, 3, -2}},
        {{0, 1, 8}, {0, 1, 2}, {0, 2, 0}, {2, 1, 2}, {1, 2, 0}},
        {},
        {{1, 2, -4}, {2, 1, 1}},
    };
    const std::vector<std::size_t> sizes{4U, 4U, 3U, 0U, 4U};
    for (std::size_t index = 0; index < cases.size(); ++index) {
        const ReferenceResult expected =
            floyd_warshall_reference(sizes[index], cases[index]);
        bool threw_cycle = false;
        johnson_apsp::Result actual;
        try {
            actual = johnson_apsp::compute(sizes[index], cases[index]);
        } catch (const std::domain_error&) {
            threw_cycle = true;
        }
        require(threw_cycle == expected.has_negative_cycle,
                "Johnson negative-cycle result disagrees with Floyd-Warshall");
        if (!expected.has_negative_cycle) {
            require(actual.distances == expected.distances,
                    "Johnson distances disagree with Floyd-Warshall");
        }
    }
}

void test_deterministic_graphs_against_reference() {
    std::uint32_t state = 0x51A7E123U;
    for (std::size_t trial = 0; trial < 400U; ++trial) {
        const std::size_t vertex_count =
            static_cast<std::size_t>(next_random(state) % 8U);
        std::vector<johnson_apsp::Edge> edges;
        if (vertex_count != 0U) {
            const std::size_t edge_count =
                static_cast<std::size_t>(next_random(state) % 25U);
            edges.reserve(edge_count);
            for (std::size_t index = 0; index < edge_count; ++index) {
                const std::size_t from =
                    static_cast<std::size_t>(next_random(state) % vertex_count);
                const std::size_t to =
                    static_cast<std::size_t>(next_random(state) % vertex_count);
                const std::int64_t weight =
                    static_cast<std::int64_t>(next_random(state) % 15U) - 7;
                edges.push_back({from, to, weight});
            }
        }

        const ReferenceResult expected =
            floyd_warshall_reference(vertex_count, edges);
        bool threw_cycle = false;
        johnson_apsp::Result actual;
        try {
            actual = johnson_apsp::compute(vertex_count, edges);
        } catch (const std::domain_error&) {
            threw_cycle = true;
        }
        require(threw_cycle == expected.has_negative_cycle,
                "deterministic graph cycle result disagrees with reference");
        if (!expected.has_negative_cycle) {
            require(actual.distances == expected.distances,
                    "deterministic graph distances disagree with reference");
        }
    }
}

void test_invalid_endpoint() {
    bool threw = false;
    try {
        static_cast<void>(johnson_apsp::compute(2U, {{0U, 2U, 1}}));
    } catch (const std::out_of_range&) {
        threw = true;
    }
    require(threw, "invalid edge endpoint should throw std::out_of_range");
}

void test_overflow() {
    const std::int64_t maximum = std::numeric_limits<std::int64_t>::max();
    const std::int64_t minimum = std::numeric_limits<std::int64_t>::min();

    bool potential_overflow = false;
    try {
        static_cast<void>(johnson_apsp::compute(
            3U, {{0U, 1U, minimum}, {1U, 2U, -1}}));
    } catch (const std::overflow_error&) {
        potential_overflow = true;
    }
    require(potential_overflow, "potential overflow should throw std::overflow_error");

    bool reweight_overflow = false;
    try {
        static_cast<void>(johnson_apsp::compute(
            3U, {{0U, 1U, minimum}, {2U, 1U, maximum}}));
    } catch (const std::overflow_error&) {
        reweight_overflow = true;
    }
    require(reweight_overflow, "reweight overflow should throw std::overflow_error");

    bool dijkstra_overflow = false;
    try {
        static_cast<void>(johnson_apsp::compute(
            3U, {{0U, 1U, maximum}, {1U, 2U, 1}}));
    } catch (const std::overflow_error&) {
        dijkstra_overflow = true;
    }
    require(dijkstra_overflow, "Dijkstra distance overflow should throw std::overflow_error");

    const johnson_apsp::Result exact =
        johnson_apsp::compute(2U, {{0U, 1U, maximum}});
    require(exact.distances[0U][1U] == maximum,
            "exact maximum representable distance should be retained");
}

}  // namespace

int main() {
    test_reference_cases();
    test_deterministic_graphs_against_reference();
    test_invalid_endpoint();
    test_overflow();
    std::cout << "All Johnson APSP tests passed.\n";
    return 0;
}
