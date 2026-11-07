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
 * Time: O(V log V), where V is the number of grid cells.
 * Space: O(V) for search state and the returned path.
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

    const std::size_t infinity = std::numeric_limits<std::size_t>::max();
    std::vector<std::vector<std::size_t>> distance(
        rows, std::vector<std::size_t>(columns, infinity));
    std::vector<std::vector<Cell>> parent(
        rows, std::vector<Cell>(columns, Cell{infinity, infinity}));

    // Tuple order makes equal-priority choices deterministic by (f, g, row, col).
    using Candidate = std::tuple<std::size_t, std::size_t, std::size_t, std::size_t>;
    std::priority_queue<Candidate, std::vector<Candidate>, std::greater<Candidate>> open;
    distance[start.first][start.second] = 0;
    open.emplace(heuristic(start), 0, start.first, start.second);

    // Fixed neighbor order: up, left, right, down.
    constexpr std::array<std::pair<int, int>, 4> directions{{
        {-1, 0}, {0, -1}, {0, 1}, {1, 0}}};

    while (!open.empty()) {
        const auto [estimated_total, current_distance, row, column] = open.top();
        static_cast<void>(estimated_total);
        open.pop();
        if (current_distance != distance[row][column]) {
            continue;
        }

        const Cell current{row, column};
        if (current == goal) {
            std::vector<Cell> path;
            Cell cell = goal;
            path.push_back(cell);
            while (cell != start) {
                cell = parent[cell.first][cell.second];
                path.push_back(cell);
            }
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
            if (tentative_distance < distance[next.first][next.second]) {
                distance[next.first][next.second] = tentative_distance;
                parent[next.first][next.second] = current;
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
        std::cout << "All A* grid pathfinding tests passed.\n";
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "A* grid pathfinding test failure: " << error.what() << '\n';
        return 1;
    }
}
