import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

/**
 * Kruskal's minimum spanning tree algorithm.
 *
 * <p>For an empty graph (zero vertices and no edges), the minimum spanning
 * forest has total weight zero. A single-vertex graph also has total weight
 * zero. For two or more vertices, this method requires the graph to be
 * connected and throws {@link IllegalArgumentException} otherwise.
 *
 * <p>Time complexity: O(E log E + E alpha(V)); space complexity: O(V + E).
 */
public final class KruskalMST {
    private KruskalMST() {
        // Utility class.
    }

    /** An undirected, weighted edge between two vertices. */
    public static final class Edge {
        public final int from;
        public final int to;
        public final int weight;

        public Edge(int from, int to, int weight) {
            this.from = from;
            this.to = to;
            this.weight = weight;
        }

        @Override
        public String toString() {
            return from + " -- " + to + " (" + weight + ")";
        }
    }

    /**
     * Returns the total weight of a minimum spanning tree.
     *
     * @param vertexCount number of vertices, labeled {@code 0} through
     *                    {@code vertexCount - 1}
     * @param edges       all undirected edges in the graph
     * @return the MST weight as a {@code long}; zero for an empty or
     *         single-vertex graph
     * @throws IllegalArgumentException if the vertex count is negative, the
     *         edge list or an edge is null, an endpoint is outside the vertex
     *         range, or a graph with at least two vertices is disconnected
     */
    public static long minimumSpanningTree(int vertexCount, List<Edge> edges) {
        if (vertexCount < 0) {
            throw new IllegalArgumentException("vertexCount must be non-negative");
        }
        if (edges == null) {
            throw new IllegalArgumentException("edges must not be null");
        }

        for (int i = 0; i < edges.size(); i++) {
            Edge edge = edges.get(i);
            if (edge == null) {
                throw new IllegalArgumentException("edge at index " + i + " is null");
            }
            validateEndpoint(edge.from, vertexCount, "from", edge);
            validateEndpoint(edge.to, vertexCount, "to", edge);
        }

        if (vertexCount <= 1) {
            return 0L;
        }

        List<Edge> sortedEdges = new ArrayList<>(edges);
        sortedEdges.sort(Comparator.comparingInt(edge -> edge.weight));

        DisjointSet sets = new DisjointSet(vertexCount);
        long totalWeight = 0L;
        int selectedEdges = 0;

        for (Edge edge : sortedEdges) {
            if (sets.union(edge.from, edge.to)) {
                totalWeight += edge.weight;
                selectedEdges++;
                if (selectedEdges == vertexCount - 1) {
                    return totalWeight;
                }
            }
        }

        throw new IllegalArgumentException(
                "graph is disconnected: expected " + (vertexCount - 1)
                        + " edges in a spanning tree but could select " + selectedEdges);
    }

    private static void validateEndpoint(int vertex, int vertexCount,
                                         String endpointName, Edge edge) {
        if (vertex < 0 || vertex >= vertexCount) {
            throw new IllegalArgumentException("invalid " + endpointName + " endpoint "
                    + vertex + " in edge " + edge + "; valid vertices are 0 through "
                    + (vertexCount - 1));
        }
    }

    /** Disjoint-set union with union-by-size and path compression. */
    private static final class DisjointSet {
        private final int[] parent;
        private final int[] size;

        private DisjointSet(int count) {
            parent = new int[count];
            size = new int[count];
            for (int i = 0; i < count; i++) {
                parent[i] = i;
                size[i] = 1;
            }
        }

        private int find(int vertex) {
            while (vertex != parent[vertex]) {
                parent[vertex] = parent[parent[vertex]];
                vertex = parent[vertex];
            }
            return vertex;
        }

        /** Joins two components; returns false if they were already joined. */
        private boolean union(int first, int second) {
            int firstRoot = find(first);
            int secondRoot = find(second);
            if (firstRoot == secondRoot) {
                return false;
            }

            if (size[firstRoot] < size[secondRoot]) {
                int temporary = firstRoot;
                firstRoot = secondRoot;
                secondRoot = temporary;
            }
            parent[secondRoot] = firstRoot;
            size[firstRoot] += size[secondRoot];
            return true;
        }
    }

    private static List<Edge> edges(Edge... values) {
        List<Edge> result = new ArrayList<>();
        for (Edge edge : values) {
            result.add(edge);
        }
        return result;
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

    /** Assertion-enabled self-tests; run with {@code java -ea java/KruskalMST.java}. */
    public static void main(String[] args) {
        boolean assertionsEnabled = false;
        assert assertionsEnabled = true;
        if (!assertionsEnabled) {
            throw new IllegalStateException("Run tests with assertions enabled (-ea)");
        }

        // Standard graph: MST uses weights 4, 5, and 10.
        assert minimumSpanningTree(4, edges(
                new Edge(0, 1, 10), new Edge(0, 2, 6), new Edge(0, 3, 5),
                new Edge(1, 3, 15), new Edge(2, 3, 4))) == 19L;

        // Negative weights are valid, and the sum is accumulated as a long.
        assert minimumSpanningTree(3, edges(
                new Edge(0, 1, -5), new Edge(1, 2, -2), new Edge(0, 2, 4))) == -7L;

        assert minimumSpanningTree(1, edges()) == 0L;
        assert minimumSpanningTree(0, edges()) == 0L;

        expectIllegalArgument(
                () -> minimumSpanningTree(2, edges(new Edge(0, 2, 1))),
                "invalid to endpoint");
        expectIllegalArgument(
                () -> minimumSpanningTree(3, edges(new Edge(0, 1, 1))),
                "disconnected");

        System.out.println("KruskalMST tests passed");
    }
}
