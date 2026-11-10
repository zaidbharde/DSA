#include <algorithm>
#include <array>
#include <cstddef>
#include <exception>
#include <iostream>
#include <limits>
#include <queue>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

/*
 * A* shortest path on a rectangular four-neighbor grid. A true cell is blocked;
 * every move has unit cost and the Manhattan distance is the heuristic.
 *
 * Empty grids, zero-width grids, jagged grids, and out-of-range endpoints throw
 * std::invalid_argument. A blocked start or goal returns an empty path. For an
 * unblocked start equal to goal, the path contains that single cell.
 * Neighbors and priority-queue ties are ordered deterministically.
 *
 * Time: O(V log V), where V is the number of grid cells (the grid has at most
 * four edges per cell, and each successful relaxation adds one queue entry).
 * Space: O(V) for the flat distance/parent arrays, queue, and returned path.
 */
namespace a_star_grid {

using Cell = std::pair<std::size_t, std::size_t>;
using Grid = std::vector<std::vector<bool>>;

std::vector<Cell> shortestPath(const Grid& blocked, Cell start, Cell goal) {
    if (blocked.empty() || blocked.front().empty()) {
        throw std::invalid_argument("grid must have at least one row and one column");
    }

    const std::size_t rows = blocked.size();
    const std::size_t columns = blocked.front().size();
    for (const auto& row : blocked) {
        if (row.size() != columns) {
            throw std::invalid_argument("grid must be rectangular");
        }
    }

    const auto inBounds = [rows, columns](const Cell& cell) {
        return cell.first < rows && cell.second < columns;
    };
    if (!inBounds(start) || !inBounds(goal)) {
        throw std::invalid_argument("start and goal must be inside the grid");
    }
    if (blocked[start.first][start.second] || blocked[goal.first][goal.second]) {
        return {};
    }

    const auto heuristic = [goal](const Cell& cell) {
        const std::size_t row_distance = cell.first > goal.first
                                             ? cell.first - goal.first
                                             : goal.first - cell.first;
        const std::size_t column_distance = cell.second > goal.second
                                                ? cell.second - goal.second
                                                : goal.second - cell.second;
        return row_distance + column_distance;
    };

    if (rows > std::numeric_limits<std::size_t>::max() / columns) {
        throw std::length_error("grid is too large to index");
    }
    const std::size_t cell_count = rows * columns;
    const auto indexOf = [columns](const Cell& cell) {
        return cell.first * columns + cell.second;
    };
    const std::size_t infinity = std::numeric_limits<std::size_t>::max();
    const std::size_t start_index = indexOf(start);
    std::vector<std::size_t> distance(cell_count, infinity);
    std::vector<std::size_t> parent(cell_count, infinity);

    // Tuple order makes equal-priority choices deterministic by (f, g, row, col).
    using Candidate = std::tuple<std::size_t, std::size_t, std::size_t, std::size_t>;
    std::priority_queue<Candidate, std::vector<Candidate>, std::greater<Candidate>> open;
    distance[start_index] = 0;
    open.emplace(heuristic(start), 0, start.first, start.second);

    // Fixed neighbor order: up, left, right, down.
    constexpr std::array<std::pair<int, int>, 4> directions{{
        {-1, 0}, {0, -1}, {0, 1}, {1, 0}}};

    while (!open.empty()) {
        const auto [estimated_total, current_distance, row, column] = open.top();
        static_cast<void>(estimated_total);
        open.pop();
        const std::size_t current_index = row * columns + column;
        if (current_distance != distance[current_index]) {
            continue;
        }

        const Cell current{row, column};
        if (current == goal) {
            std::vector<Cell> path;
            std::size_t path_index = indexOf(goal);
            while (path_index != start_index) {
                path.push_back({path_index / columns, path_index % columns});
                path_index = parent[path_index];
            }
            path.push_back(start);
            std::reverse(path.begin(), path.end());
            return path;
        }

        for (const auto& [row_delta, column_delta] : directions) {
            const long long next_row = static_cast<long long>(row) + row_delta;
            const long long next_column = static_cast<long long>(column) + column_delta;
            if (next_row < 0 || next_column < 0 ||
                next_row >= static_cast<long long>(rows) ||
                next_column >= static_cast<long long>(columns)) {
                continue;
            }

            const Cell next{static_cast<std::size_t>(next_row),
                            static_cast<std::size_t>(next_column)};
            if (blocked[next.first][next.second]) {
                continue;
            }

            const std::size_t tentative_distance = current_distance + 1;
            const std::size_t next_index = indexOf(next);
            if (tentative_distance < distance[next_index]) {
                distance[next_index] = tentative_distance;
                parent[next_index] = current_index;
                open.emplace(tentative_distance + heuristic(next), tentative_distance,
                             next.first, next.second);
            }
        }
    }

    return {};
}

}  // namespace a_star_grid

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

template <typename Function>
void requireInvalidArgument(Function&& function, const char* message) {
    bool threw = false;
    try {
        function();
    } catch (const std::invalid_argument&) {
        threw = true;
    }
    require(threw, message);
}

void testDirectPath() {
    const a_star_grid::Grid grid(1, std::vector<bool>(4, false));
    const std::vector<a_star_grid::Cell> path =
        a_star_grid::shortestPath(grid, {0, 0}, {0, 3});
    require(path == std::vector<a_star_grid::Cell>{{0, 0}, {0, 1}, {0, 2}, {0, 3}},
            "direct path should follow the row to the goal");
}

void testDetour() {
    const a_star_grid::Grid grid{
        {false, true, false},
        {false, true, false},
        {false, false, false},
    };
    const std::vector<a_star_grid::Cell> path =
        a_star_grid::shortestPath(grid, {0, 0}, {0, 2});
    const std::vector<a_star_grid::Cell> expected{
        {0, 0}, {1, 0}, {2, 0}, {2, 1}, {2, 2}, {1, 2}, {0, 2}};
    require(path == expected, "detour path should avoid the blocked corridor");
}

void testUnreachable() {
    const a_star_grid::Grid grid{
        {false, true, false},
        {true, true, true},
        {false, true, false},
    };
    require(a_star_grid::shortestPath(grid, {0, 0}, {2, 2}).empty(),
            "unreachable goal should return an empty path");
}

void testBlockedEndpoints() {
    const a_star_grid::Grid grid{{true, false}, {false, false}};
    require(a_star_grid::shortestPath(grid, {0, 0}, {1, 1}).empty(),
            "blocked start should return an empty path");
    require(a_star_grid::shortestPath(grid, {1, 0}, {0, 0}).empty(),
            "blocked goal should return an empty path");
}

void testSameCell() {
    const a_star_grid::Grid grid{{false, true}, {false, false}};
    require(a_star_grid::shortestPath(grid, {1, 0}, {1, 0}) ==
                std::vector<a_star_grid::Cell>{{1, 0}},
            "unblocked start equal to goal should return a single-cell path");
    require(a_star_grid::shortestPath(grid, {0, 1}, {0, 1}).empty(),
            "blocked start equal to goal should return an empty path");
}

void testInvalidGridShapesAndEndpoints() {
    requireInvalidArgument(
        [] { static_cast<void>(a_star_grid::shortestPath({}, {0, 0}, {0, 0})); },
        "empty grid should throw std::invalid_argument");
    requireInvalidArgument(
        [] { static_cast<void>(a_star_grid::shortestPath({{}}, {0, 0}, {0, 0})); },
        "zero-width grid should throw std::invalid_argument");
    requireInvalidArgument(
        [] {
            static_cast<void>(a_star_grid::shortestPath(
                {{false, false}, {false}}, {0, 0}, {0, 1}));
        },
        "jagged grid should throw std::invalid_argument");
    requireInvalidArgument(
        [] {
            static_cast<void>(a_star_grid::shortestPath(
                {{false}}, {0, 0}, {1, 0}));
        },
        "out-of-range endpoint should throw std::invalid_argument");
}

void testDeterministicOutput() {
    const a_star_grid::Grid grid(3, std::vector<bool>(3, false));
    const std::vector<a_star_grid::Cell> expected{
        {0, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}};
    const auto first = a_star_grid::shortestPath(grid, {0, 0}, {2, 2});
    const auto second = a_star_grid::shortestPath(grid, {0, 0}, {2, 2});
    require(first == expected && second == expected,
            "equal-cost paths should have stable deterministic tie-breaking");
}

void testLargerGridFlatState() {
    constexpr std::size_t rows = 32;
    constexpr std::size_t columns = 40;
    const a_star_grid::Grid grid(rows, std::vector<bool>(columns, false));
    const auto path = a_star_grid::shortestPath(grid, {0, 0}, {rows - 1, columns - 1});
    require(path.size() == rows + columns - 1,
            "larger open-grid path should have Manhattan-shortest length");
    require(path.front() == a_star_grid::Cell{0, 0} &&
                path.back() == a_star_grid::Cell{rows - 1, columns - 1},
            "larger open-grid path should preserve endpoints");
    for (std::size_t i = 1; i < path.size(); ++i) {
        const std::size_t row_delta = path[i].first > path[i - 1].first
                                          ? path[i].first - path[i - 1].first
                                          : path[i - 1].first - path[i].first;
        const std::size_t column_delta = path[i].second > path[i - 1].second
                                             ? path[i].second - path[i - 1].second
                                             : path[i - 1].second - path[i].second;
        require(row_delta + column_delta == 1,
                "larger-grid path should contain only neighboring cells");
    }
}

}  // namespace

int main() {
    try {
        testDirectPath();
        testDetour();
        testUnreachable();
        testBlockedEndpoints();
        testSameCell();
        testInvalidGridShapesAndEndpoints();
        testDeterministicOutput();
        testLargerGridFlatState();
        std::cout << "All A* grid pathfinding tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "A* grid pathfinding test failure: " << error.what() << '\n';
        return 1;
    }
}
