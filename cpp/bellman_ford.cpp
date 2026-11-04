#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <vector>

namespace bellman_ford {

/** A directed, weighted edge. Vertices are numbered [0, vertex_count). */
struct Edge {
    std::size_t from;
    std::size_t to;
    std::int64_t weight;
};

/**
 * Result of shortest_paths(). A disengaged distance means the vertex is
 * unreachable from the requested source. If has_reachable_negative_cycle is
 * true, finite distances are only intermediate values and are not promised to
 * be shortest paths; callers should use the cycle flag to reject the result.
 */
struct Result {
    std::vector<std::optional<std::int64_t>> distances;
    bool has_reachable_negative_cycle;
};

/**
 * Computes single-source shortest-path distances with Bellman-Ford.
 *
 * Throws std::invalid_argument if vertex_count is zero, source is out of
 * range, or any edge endpoint is out of range. Throws std::overflow_error if
 * adding the weight of any edge from a currently reachable vertex would
 * produce a value outside the int64_t range. Thus unrepresentable candidate
 * distances are reported rather than wrapped or saturated, even if that
 * candidate would not ultimately be part of a shortest path.
 *
 * Time: O(VE). Algorithm/output storage: O(V), including the returned
 * distances. The caller-owned input edge list is O(E) and is not copied.
 */
Result shortest_paths(std::size_t vertex_count,
                      const std::vector<Edge>& edges,
                      std::size_t source) {
    if (vertex_count == 0U || source >= vertex_count) {
        throw std::invalid_argument("source must name a vertex in a nonempty graph");
    }
    for (const Edge& edge : edges) {
        if (edge.from >= vertex_count || edge.to >= vertex_count) {
            throw std::invalid_argument("edge endpoint is outside the graph");
        }
    }

    Result result{std::vector<std::optional<std::int64_t>>(vertex_count), false};
    result.distances[source] = 0;

    const auto add_weight_checked = [](std::int64_t distance,
                                       std::int64_t weight) -> std::int64_t {
        constexpr std::int64_t min_value = std::numeric_limits<std::int64_t>::min();
        constexpr std::int64_t max_value = std::numeric_limits<std::int64_t>::max();
        if ((weight > 0 && distance > max_value - weight) ||
            (weight < 0 && distance < min_value - weight)) {
            throw std::overflow_error("reachable path distance is outside int64_t range");
        }
        return distance + weight;
    };

    for (std::size_t pass = 1U; pass < vertex_count; ++pass) {
        bool changed = false;
        for (const Edge& edge : edges) {
            const std::optional<std::int64_t>& from_distance = result.distances[edge.from];
            if (!from_distance.has_value()) {
                continue;
            }
            const std::int64_t candidate = add_weight_checked(*from_distance, edge.weight);
            std::optional<std::int64_t>& to_distance = result.distances[edge.to];
            if (!to_distance.has_value() || candidate < *to_distance) {
                to_distance = candidate;
                changed = true;
            }
        }
        if (!changed) {
            break;
        }
    }

    for (const Edge& edge : edges) {
        const std::optional<std::int64_t>& from_distance = result.distances[edge.from];
        if (!from_distance.has_value()) {
            continue;
        }
        const std::int64_t candidate = add_weight_checked(*from_distance, edge.weight);
        const std::optional<std::int64_t>& to_distance = result.distances[edge.to];
        if (!to_distance.has_value() || candidate < *to_distance) {
            result.has_reachable_negative_cycle = true;
            break;
        }
    }

    return result;
}

}  // namespace bellman_ford

#ifdef BELLMAN_FORD_SELF_TEST
int main() {
    using bellman_ford::Edge;
    using bellman_ford::Result;
    using bellman_ford::shortest_paths;

    {
        const std::vector<Edge> edges{{0U, 1U, 4}, {1U, 2U, -3}};
        const Result result = shortest_paths(3U, edges, 0U);
        assert(!result.has_reachable_negative_cycle);
        assert(result.distances[2U] == 1);
        assert(edges.size() == 2U);
        assert(edges[0U].from == 0U && edges[0U].to == 1U && edges[0U].weight == 4);
        assert(edges[1U].from == 1U && edges[1U].to == 2U && edges[1U].weight == -3);
    }

    {
        const Result result = shortest_paths(4U, {{0U, 1U, 4}, {0U, 2U, 5}, {1U, 2U, -3}}, 0U);
        assert(!result.has_reachable_negative_cycle);
        assert(result.distances[0U] == 0);
        assert(result.distances[1U] == 4);
        assert(result.distances[2U] == 1);
        assert(!result.distances[3U].has_value());
    }
    {
        const Result result = shortest_paths(4U, {{0U, 1U, 2}, {2U, 3U, -2}, {3U, 2U, -2}}, 0U);
        assert(!result.has_reachable_negative_cycle);
        assert(!result.distances[2U].has_value());
        assert(!result.distances[3U].has_value());
    }
    {
        const Result result = shortest_paths(3U, {{0U, 1U, 1}, {1U, 2U, -3}, {2U, 1U, 1}}, 0U);
        assert(result.has_reachable_negative_cycle);
    }
    {
        const Result result = shortest_paths(4U, {{0U, 1U, 7}, {2U, 3U, -2}, {3U, 2U, -2}}, 0U);
        assert(!result.has_reachable_negative_cycle);
    }
    {
        bool threw = false;
        try {
            (void)shortest_paths(0U, {}, 0U);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        assert(threw);
    }
    {
        bool threw = false;
        try {
            (void)shortest_paths(2U, {{0U, 1U, 1}}, 2U);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        assert(threw);
    }
    {
        bool threw = false;
        try {
            (void)shortest_paths(2U, {{0U, 2U, 1}}, 0U);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        assert(threw);
    }
    {
        const std::int64_t max_value = std::numeric_limits<std::int64_t>::max();
        const Result result = shortest_paths(2U, {{0U, 1U, max_value}}, 0U);
        assert(result.distances[1U] == max_value);
    }
    {
        bool threw = false;
        try {
            const std::int64_t max_value = std::numeric_limits<std::int64_t>::max();
            (void)shortest_paths(3U, {{0U, 1U, max_value}, {1U, 2U, 1}}, 0U);
        } catch (const std::overflow_error&) {
            threw = true;
        }
        assert(threw);
    }
    {
        bool threw = false;
        try {
            const std::int64_t min_value = std::numeric_limits<std::int64_t>::min();
            (void)shortest_paths(3U, {{0U, 1U, min_value}, {1U, 2U, -1}}, 0U);
        } catch (const std::overflow_error&) {
            threw = true;
        }
        assert(threw);
    }

    return 0;
}
#endif
