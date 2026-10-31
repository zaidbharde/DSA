import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Queue;

public class BFSGraph {
    /**
     * Returns the breadth-first traversal reachable from {@code start}.
     * The graph is represented by an adjacency list with vertices numbered
     * from 0 through adj.size() - 1.
     *
     * Time: O(V + E); auxiliary space: O(V).
     */
    public static List<Integer> bfs(List<List<Integer>> adj, int start) {
        if (adj == null) {
            throw new IllegalArgumentException("Adjacency list must not be null");
        }
        if (adj.isEmpty()) {
            throw new IllegalArgumentException("Graph must contain at least one vertex");
        }
        if (start < 0 || start >= adj.size()) {
            throw new IllegalArgumentException(
                    "Start vertex " + start + " is outside the valid range 0.." + (adj.size() - 1));
        }

        // Validate the full graph first, including components not reached from start.
        for (int vertex = 0; vertex < adj.size(); vertex++) {
            List<Integer> neighbors = adj.get(vertex);
            if (neighbors == null) {
                throw new IllegalArgumentException("Neighbor list for vertex " + vertex + " must not be null");
            }
            for (Integer neighbor : neighbors) {
                if (neighbor == null || neighbor < 0 || neighbor >= adj.size()) {
                    throw new IllegalArgumentException(
                            "Invalid neighbor index " + neighbor + " in adjacency list for vertex " + vertex
                                    + "; valid indices are 0.." + (adj.size() - 1));
                }
            }
        }

        boolean[] visited = new boolean[adj.size()];
        Queue<Integer> queue = new LinkedList<>();
        List<Integer> traversal = new ArrayList<>();

        visited[start] = true;
        queue.add(start);
        while (!queue.isEmpty()) {
            int vertex = queue.remove();
            traversal.add(vertex);
            for (int neighbor : adj.get(vertex)) {
                if (!visited[neighbor]) {
                    visited[neighbor] = true;
                    queue.add(neighbor);
                }
            }
        }
        return traversal;
    }

    private static List<List<Integer>> graph(int vertexCount) {
        List<List<Integer>> adj = new ArrayList<>();
        for (int i = 0; i < vertexCount; i++) {
            adj.add(new ArrayList<>());
        }
        return adj;
    }

    private static void expectInvalid(String description, Runnable operation) {
        try {
            operation.run();
            throw new AssertionError("Expected IllegalArgumentException for " + description);
        } catch (IllegalArgumentException expected) {
            // The input was rejected as expected.
        }
    }

    public static void main(String[] args) {
        // Ordinary traversal follows breadth-first order.
        List<List<Integer>> ordinary = graph(5);
        ordinary.get(0).add(1);
        ordinary.get(0).add(2);
        ordinary.get(1).add(3);
        ordinary.get(2).add(4);
        ordinary.get(3).add(4);
        assert bfs(ordinary, 0).equals(List.of(0, 1, 2, 3, 4));

        // Disconnected vertices are not included; an isolated start is returned alone.
        List<List<Integer>> disconnected = graph(5);
        disconnected.get(0).add(1);
        disconnected.get(1).add(0);
        disconnected.get(3).add(4);
        assert bfs(disconnected, 0).equals(List.of(0, 1));
        assert bfs(disconnected, 2).equals(List.of(2));

        // Invalid graph, start, and neighbor inputs have explicit failures.
        expectInvalid("null graph", () -> bfs(null, 0));
        expectInvalid("empty graph", () -> bfs(graph(0), 0));
        expectInvalid("negative start", () -> bfs(graph(1), -1));
        expectInvalid("start past the final vertex", () -> bfs(graph(1), 1));
        List<List<Integer>> invalidNeighbor = graph(2);
        invalidNeighbor.get(0).add(2);
        expectInvalid("out-of-range neighbor", () -> bfs(invalidNeighbor, 0));
        List<List<Integer>> invalidDisconnectedNeighbor = graph(2);
        invalidDisconnectedNeighbor.get(1).add(-1);
        expectInvalid("invalid neighbor in a disconnected component", () -> bfs(invalidDisconnectedNeighbor, 0));

        System.out.println("All BFSGraph self-tests passed.");
    }
}
