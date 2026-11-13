#include <algorithm>
#include <iostream>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

struct BipartiteMatching {
    int cardinality;
    // left_to_right[u] is the matched right vertex, or -1 if u is unmatched.
    std::vector<int> left_to_right;
    // right_to_left[v] is the matched left vertex, or -1 if v is unmatched.
    std::vector<int> right_to_left;
};

// Returns a maximum-cardinality matching in a bipartite graph whose vertices
// are numbered [0, left_count) and [0, right_count). Duplicate edges are
// permitted. Negative partition sizes throw std::invalid_argument; any edge
// endpoint outside its partition also throws std::invalid_argument.
//
// Time: O(E * sqrt(V)), where V = left_count + right_count and E is the
// number of input edges (including duplicates). Space: O(V + E).
BipartiteMatching hopcroft_karp(
    int left_count,
    int right_count,
    const std::vector<std::pair<int, int>>& edges) {
    if (left_count < 0 || right_count < 0) {
        throw std::invalid_argument("bipartite partition sizes must be nonnegative");
    }
    if (left_count > std::numeric_limits<int>::max() - right_count - 1) {
        throw std::length_error("combined bipartite vertex count is too large");
    }

    std::vector<std::vector<int>> adjacency(static_cast<std::size_t>(left_count));
    for (const auto& edge : edges) {
        const int left = edge.first;
        const int right = edge.second;
        if (left < 0 || left >= left_count || right < 0 || right >= right_count) {
            throw std::invalid_argument("bipartite edge endpoint is out of range");
        }
        adjacency[static_cast<std::size_t>(left)].push_back(right);
    }

    const int unmatched = -1;
    const int infinity = left_count + right_count + 1;
    std::vector<int> left_mate(static_cast<std::size_t>(left_count), unmatched);
    std::vector<int> right_mate(static_cast<std::size_t>(right_count), unmatched);
    std::vector<int> distance(static_cast<std::size_t>(left_count), infinity);
    int shortest_augmenting_path = infinity;

    const auto build_layers = [&]() {
        std::vector<int> queue;
        queue.reserve(static_cast<std::size_t>(left_count));
        for (int left = 0; left < left_count; ++left) {
            if (left_mate[static_cast<std::size_t>(left)] == unmatched) {
                distance[static_cast<std::size_t>(left)] = 0;
                queue.push_back(left);
            } else {
                distance[static_cast<std::size_t>(left)] = infinity;
            }
        }

        shortest_augmenting_path = infinity;
        for (std::size_t head = 0; head < queue.size(); ++head) {
            const int left = queue[head];
            const int next_distance = distance[static_cast<std::size_t>(left)] + 1;
            if (next_distance > shortest_augmenting_path) {
                continue;
            }
            for (const int right : adjacency[static_cast<std::size_t>(left)]) {
                const int paired_left = right_mate[static_cast<std::size_t>(right)];
                if (paired_left == unmatched) {
                    shortest_augmenting_path = next_distance;
                } else if (next_distance < shortest_augmenting_path &&
                           distance[static_cast<std::size_t>(paired_left)] == infinity) {
                    distance[static_cast<std::size_t>(paired_left)] = next_distance;
                    queue.push_back(paired_left);
                }
            }
        }
        return shortest_augmenting_path != infinity;
    };

    const auto find_augmenting_path = [&](const auto& self, int left) -> bool {
        for (const int right : adjacency[static_cast<std::size_t>(left)]) {
            const int paired_left = right_mate[static_cast<std::size_t>(right)];
            if (paired_left == unmatched) {
                if (distance[static_cast<std::size_t>(left)] + 1 ==
                    shortest_augmenting_path) {
                    left_mate[static_cast<std::size_t>(left)] = right;
                    right_mate[static_cast<std::size_t>(right)] = left;
                    return true;
                }
            } else if (distance[static_cast<std::size_t>(paired_left)] ==
                           distance[static_cast<std::size_t>(left)] + 1 &&
                       self(self, paired_left)) {
                left_mate[static_cast<std::size_t>(left)] = right;
                right_mate[static_cast<std::size_t>(right)] = left;
                return true;
            }
        }
        distance[static_cast<std::size_t>(left)] = infinity;
        return false;
    };

    int cardinality = 0;
    while (build_layers()) {
        for (int left = 0; left < left_count; ++left) {
            if (left_mate[static_cast<std::size_t>(left)] == unmatched &&
                find_augmenting_path(find_augmenting_path, left)) {
                ++cardinality;
            }
        }
    }

    return {cardinality, std::move(left_mate), std::move(right_mate)};
}

#ifdef HOPCROFT_KARP_SELF_TEST
namespace {
void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

int brute_force_matching(
    int left_count,
    int right_count,
    const std::vector<std::pair<int, int>>& edges) {
    std::vector<std::vector<int>> adjacency(static_cast<std::size_t>(left_count));
    for (const auto& edge : edges) {
        adjacency[static_cast<std::size_t>(edge.first)].push_back(edge.second);
    }
    std::vector<bool> used_right(static_cast<std::size_t>(right_count), false);
    const auto search = [&](const auto& self, int left) -> int {
        if (left == left_count) {
            return 0;
        }
        int best = self(self, left + 1);
        for (const int right : adjacency[static_cast<std::size_t>(left)]) {
            if (!used_right[static_cast<std::size_t>(right)]) {
                used_right[static_cast<std::size_t>(right)] = true;
                best = std::max(best, 1 + self(self, left + 1));
                used_right[static_cast<std::size_t>(right)] = false;
            }
        }
        return best;
    };
    return search(search, 0);
}

void check_matches_edges(
    int left_count,
    int right_count,
    const std::vector<std::pair<int, int>>& edges,
    const BipartiteMatching& matching) {
    require(matching.cardinality >= 0, "negative matching cardinality");
    require(matching.left_to_right.size() == static_cast<std::size_t>(left_count),
            "wrong left mate-map size");
    require(matching.right_to_left.size() == static_cast<std::size_t>(right_count),
            "wrong right mate-map size");
    int counted = 0;
    for (int left = 0; left < left_count; ++left) {
        const int right = matching.left_to_right[static_cast<std::size_t>(left)];
        if (right != -1) {
            require(right >= 0 && right < right_count, "invalid right mate");
            require(matching.right_to_left[static_cast<std::size_t>(right)] == left,
                    "mate maps are inconsistent");
            require(std::find(edges.begin(), edges.end(), std::make_pair(left, right)) !=
                        edges.end(),
                    "reported pair is not an input edge");
            ++counted;
        }
    }
    for (int right = 0; right < right_count; ++right) {
        const int left = matching.right_to_left[static_cast<std::size_t>(right)];
        if (left != -1) {
            require(left >= 0 && left < left_count, "invalid left mate");
            require(matching.left_to_right[static_cast<std::size_t>(left)] == right,
                    "reverse mate maps are inconsistent");
        }
    }
    require(counted == matching.cardinality, "cardinality disagrees with mate map");
}

void expect_cardinality(
    int left_count,
    int right_count,
    const std::vector<std::pair<int, int>>& edges,
    int expected) {
    const BipartiteMatching matching =
        hopcroft_karp(left_count, right_count, edges);
    require(matching.cardinality == expected, "unexpected matching cardinality");
    check_matches_edges(left_count, right_count, edges, matching);
}

void expect_invalid_input(
    int left_count,
    int right_count,
    const std::vector<std::pair<int, int>>& edges) {
    bool threw = false;
    try {
        static_cast<void>(hopcroft_karp(left_count, right_count, edges));
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    require(threw, "invalid input did not throw std::invalid_argument");
}

void run_tests() {
    expect_cardinality(0, 0, {}, 0);
    expect_cardinality(0, 3, {}, 0);
    expect_cardinality(4, 0, {}, 0);
    expect_cardinality(4, 3, {}, 0);
    expect_cardinality(4, 4, {{0, 0}, {1, 1}, {2, 2}, {3, 3}}, 4);
    expect_cardinality(4, 3, {{0, 0}, {0, 1}, {1, 0}, {2, 0}}, 2);
    expect_cardinality(3, 2, {{0, 0}, {0, 0}, {1, 0}, {1, 1}, {2, 1}}, 2);

    expect_invalid_input(-1, 0, {});
    expect_invalid_input(0, -1, {});
    expect_invalid_input(2, 2, {{-1, 0}});
    expect_invalid_input(2, 2, {{0, -1}});
    expect_invalid_input(2, 2, {{2, 0}});
    expect_invalid_input(2, 2, {{0, 2}});
    bool oversized_partitions_rejected = false;
    try {
        static_cast<void>(hopcroft_karp(std::numeric_limits<int>::max(), 1, {}));
    } catch (const std::length_error&) {
        oversized_partitions_rejected = true;
    }
    require(oversized_partitions_rejected,
            "partition sizes that overflow the distance sentinel must be rejected");

    // Exhaust every simple graph for partition sizes up to 3-by-3, then
    // compare additional deterministic 4-by-4 samples with exhaustive search.
    for (int left_count = 0; left_count <= 3; ++left_count) {
        for (int right_count = 0; right_count <= 3; ++right_count) {
            const int edge_slots = left_count * right_count;
            const unsigned int graph_count = 1U << edge_slots;
            for (unsigned int mask = 0; mask < graph_count; ++mask) {
                std::vector<std::pair<int, int>> edges;
                for (int left = 0; left < left_count; ++left) {
                    for (int right = 0; right < right_count; ++right) {
                        const int bit = left * right_count + right;
                        if ((mask & (1U << bit)) != 0U) {
                            edges.emplace_back(left, right);
                        }
                    }
                }
                const BipartiteMatching matching =
                    hopcroft_karp(left_count, right_count, edges);
                check_matches_edges(left_count, right_count, edges, matching);
                require(matching.cardinality ==
                            brute_force_matching(left_count, right_count, edges),
                        "exhaustive small-graph comparison failed");
            }
        }
    }

    unsigned int state = 0xC0FFEEU;
    for (int sample = 0; sample < 256; ++sample) {
        std::vector<std::pair<int, int>> edges;
        for (int left = 0; left < 4; ++left) {
            for (int right = 0; right < 4; ++right) {
                state = state * 1664525U + 1013904223U;
                if ((state >> 29U) < 3U) {
                    edges.emplace_back(left, right);
                }
            }
        }
        const BipartiteMatching matching = hopcroft_karp(4, 4, edges);
        check_matches_edges(4, 4, edges, matching);
        require(matching.cardinality == brute_force_matching(4, 4, edges),
                "deterministic sample comparison failed");
    }
}
}  // namespace

int main() {
    run_tests();
    std::cout << "Hopcroft-Karp tests passed\n";
    return 0;
}
#else
int main() {
    const BipartiteMatching matching = hopcroft_karp(
        4, 4, {{0, 1}, {0, 2}, {1, 0}, {2, 1}, {3, 3}});
    std::cout << "Maximum matching size: " << matching.cardinality << '\n';
    return 0;
}
#endif
