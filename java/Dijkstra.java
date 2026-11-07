import java.util.ArrayList;
import java.util.Arrays;
import java.util.Comparator;
import java.util.List;
import java.util.PriorityQueue;

/**
 * Dijkstra's single-source shortest-path algorithm for directed graphs with
 * nonnegative {@code long} edge weights.
 *
 * <p>The graph is represented as an adjacency list. {@link #UNREACHABLE} is
 * {@code -1L}; since valid distances are nonnegative, this sentinel leaves the
 * entire nonnegative {@code long} range available for distances.
 *
 * <p>Time complexity: O((V + E) log V) using a binary priority queue; space
 * complexity: O(V + E). Here V is the number of vertices and E is the number
 * of adjacency-list entries.
 */
public final class Dijkstra {
    /** Sentinel returned for vertices unreachable from the source. */
    public static final long UNREACHABLE = -1L;

    private Dijkstra() {
        // Utility class.
    }

    /** A directed edge from the containing adjacency list's vertex to {@code to}. */
    public static final class Edge {
        public final int to;
        public final long weight;

        public Edge(int to, long weight) {
            this.to = to;
            this.weight = weight;
        }
    }

    /** A vertex and its current tentative distance in the priority queue. */
    private static final class QueueEntry {
        private final int vertex;
        private final long distance;

        private QueueEntry(int vertex, long distance) {
            this.vertex = vertex;
            this.distance = distance;
        }
    }

    /**
     * Returns shortest-path distances from {@code source}.
     *
     * @param graph adjacency list; each edge's {@code to} vertex must be in range
     * @param source source vertex, in the range {@code [0, graph.size())}
     * @return an array indexed by vertex, with {@link #UNREACHABLE} for vertices
     *         that cannot be reached from the source
     * @throws IllegalArgumentException if the graph, an adjacency list, or an
     *         edge is null; the source or an edge endpoint is invalid; or an
     *         edge has a negative weight
     * @throws ArithmeticException if a candidate path distance exceeds
     *         {@link Long#MAX_VALUE}; this is reported rather than wrapping or
     *         silently treating an overflowing route as unreachable
     */
    public static long[] shortestPaths(List<List<Edge>> graph, int source) {
        if (graph == null) {
            throw new IllegalArgumentException("graph must not be null");
        }
        int vertexCount = graph.size();
        validateSource(source, vertexCount);

        // Validate the whole input before traversal, including edges in
        // components not reachable from the source.
        for (int from = 0; from < vertexCount; from++) {
            List<Edge> edges = graph.get(from);
            if (edges == null) {
                throw new IllegalArgumentException("adjacency list for vertex " + from
                        + " must not be null");
            }
            for (int index = 0; index < edges.size(); index++) {
                Edge edge = edges.get(index);
                if (edge == null) {
                    throw new IllegalArgumentException("edge at index " + index
                            + " from vertex " + from + " must not be null");
                }
                if (edge.to < 0 || edge.to >= vertexCount) {
                    throw new IllegalArgumentException("invalid endpoint " + edge.to
                            + " on edge from vertex " + from + "; valid vertices are 0 through "
                            + (vertexCount - 1));
                }
                if (edge.weight < 0) {
                    throw new IllegalArgumentException("negative edge weight " + edge.weight
                            + " on edge from vertex " + from + " to " + edge.to);
                }
            }
        }

        long[] distances = new long[vertexCount];
        Arrays.fill(distances, UNREACHABLE);
        distances[source] = 0L;

        PriorityQueue<QueueEntry> queue = new PriorityQueue<>(
                Comparator.comparingLong(entry -> entry.distance));
        queue.add(new QueueEntry(source, 0L));

        while (!queue.isEmpty()) {
            QueueEntry current = queue.remove();
            if (current.distance != distances[current.vertex]) {
                continue; // Ignore a stale entry superseded by a shorter route.
            }

            for (Edge edge : graph.get(current.vertex)) {
                final long candidate;
                try {
                    candidate = Math.addExact(current.distance, edge.weight);
                } catch (ArithmeticException overflow) {
                    throw new ArithmeticException("path distance overflow while traversing edge from "
                            + current.vertex + " to " + edge.to + " (distance "
                            + current.distance + ", weight " + edge.weight + ")");
                }

                if (distances[edge.to] == UNREACHABLE || candidate < distances[edge.to]) {
                    distances[edge.to] = candidate;
                    queue.add(new QueueEntry(edge.to, candidate));
                }
            }
        }

        return distances;
    }

    private static void validateSource(int source, int vertexCount) {
        if (source < 0 || source >= vertexCount) {
            throw new IllegalArgumentException("invalid source " + source
                    + "; graph has " + vertexCount + " vertices");
        }
    }

    private static List<List<Edge>> graph(int vertexCount) {
        List<List<Edge>> result = new ArrayList<>(vertexCount);
        for (int vertex = 0; vertex < vertexCount; vertex++) {
            result.add(new ArrayList<>());
        }
        return result;
    }

    private static void addEdge(List<List<Edge>> graph, int from, int to, long weight) {
        graph.get(from).add(new Edge(to, weight));
    }

    private static void expectIllegalArgument(Runnable action, String messagePart) {
        try {
            action.run();
            throw new AssertionError("Expected IllegalArgumentException containing: "
                    + messagePart);
        } catch (IllegalArgumentException exception) {
            assert exception.getMessage().contains(messagePart)
                    : "Unexpected exception message: " + exception.getMessage();
        }
    }

    private static void expectArithmeticError(Runnable action, String messagePart) {
        try {
            action.run();
            throw new AssertionError("Expected ArithmeticException containing: " + messagePart);
        } catch (ArithmeticException exception) {
            assert exception.getMessage().contains(messagePart)
                    : "Unexpected exception message: " + exception.getMessage();
        }
    }

    /** Assertion-enabled self-tests; run with {@code java -ea java/Dijkstra.java}. */
    public static void main(String[] args) {
        boolean assertionsEnabled = false;
        assert assertionsEnabled = true;
        if (!assertionsEnabled) {
            throw new IllegalStateException("Run tests with assertions enabled (-ea)");
        }

        List<List<Edge>> connected = graph(5);
        addEdge(connected, 0, 1, 4);
        addEdge(connected, 0, 2, 1);
        addEdge(connected, 2, 1, 2);
        addEdge(connected, 1, 3, 1);
        addEdge(connected, 2, 3, 5);
        long[] connectedDistances = shortestPaths(connected, 0);
        assert Arrays.equals(connectedDistances, new long[] {0L, 3L, 1L, 4L, UNREACHABLE})
                : Arrays.toString(connectedDistances);

        List<List<Edge>> zeroWeight = graph(3);
        addEdge(zeroWeight, 0, 1, 0L);
        addEdge(zeroWeight, 1, 2, 0L);
        assert Arrays.equals(shortestPaths(zeroWeight, 0), new long[] {0L, 0L, 0L});

        List<List<Edge>> invalidEndpoint = graph(2);
        addEdge(invalidEndpoint, 0, 2, 1L);
        expectIllegalArgument(() -> shortestPaths(invalidEndpoint, 0), "invalid endpoint");

        List<List<Edge>> negativeWeight = graph(2);
        addEdge(negativeWeight, 0, 1, -1L);
        expectIllegalArgument(() -> shortestPaths(negativeWeight, 0), "negative edge weight");

        List<List<Edge>> overflow = graph(3);
        addEdge(overflow, 0, 1, Long.MAX_VALUE);
        addEdge(overflow, 1, 2, 1L);
        expectArithmeticError(() -> shortestPaths(overflow, 0), "path distance overflow");

        List<List<Edge>> empty = graph(0);
        expectIllegalArgument(() -> shortestPaths(empty, 0), "invalid source");

        System.out.println("Dijkstra tests passed");
    }
}
