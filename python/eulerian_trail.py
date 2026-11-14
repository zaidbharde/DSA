"""Find a directed Eulerian trail with Hierholzer's algorithm.

For V vertices and E edges, this implementation takes O(V + E) time and
O(V + E) auxiliary space. The returned vertex sequence uses O(E) space. An
edgeless graph returns [] (including when V is zero); a nonempty graph with no
Eulerian trail returns None. Vertices are labeled 0 through V - 1.
"""
from typing import Iterable, List, Optional


def eulerian_trail(
    vertex_count: int, edges: Iterable[Iterable[int]]
) -> Optional[List[int]]:
    """Return a vertex sequence using each directed edge exactly once.

    ``edges`` is an iterable of directed ``(source, destination)`` pairs;
    parallel edges and self-loops are allowed. The input is consumed once and
    is not modified. ``vertex_count`` must be a nonnegative integer (not bool),
    and each endpoint must be an integer vertex in ``[0, vertex_count)``.
    Invalid vertex counts or endpoints raise ``TypeError`` or ``ValueError``.
    Malformed edge pairs raise ``TypeError`` for non-iterable items and
    ``ValueError`` for items that do not contain exactly two endpoints.

    For an edgeless graph, returns the empty sequence ``[]``. Otherwise returns
    a sequence of E + 1 vertices if a trail exists, or ``None`` if not.

    Runs in O(V + E) time and O(V + E) auxiliary space for adjacency lists,
    degree counts, and traversal state, plus O(E) space for the returned path.
    """
    if isinstance(vertex_count, bool) or not isinstance(vertex_count, int):
        raise TypeError("vertex_count must be an integer")
    if vertex_count < 0:
        raise ValueError("vertex_count must be nonnegative")

    adjacency: List[List[int]] = [[] for _ in range(vertex_count)]
    in_degree = [0] * vertex_count
    out_degree = [0] * vertex_count
    edge_count = 0

    try:
        edge_iterator = iter(edges)
    except TypeError as error:
        raise TypeError("edges must be an iterable of endpoint pairs") from error

    for edge in edge_iterator:
        try:
            source, destination = edge
        except TypeError as error:
            raise TypeError("each edge must be an iterable pair") from error
        except ValueError as error:
            raise ValueError("each edge must contain exactly two endpoints") from error

        for endpoint in (source, destination):
            if isinstance(endpoint, bool) or not isinstance(endpoint, int):
                raise TypeError("edge endpoints must be integers")
            if endpoint < 0 or endpoint >= vertex_count:
                raise ValueError("edge endpoint is outside the vertex range")

        adjacency[source].append(destination)
        out_degree[source] += 1
        in_degree[destination] += 1
        edge_count += 1

    if edge_count == 0:
        return []

    start_vertices = []
    end_vertices = []
    for vertex in range(vertex_count):
        difference = out_degree[vertex] - in_degree[vertex]
        if difference == 1:
            start_vertices.append(vertex)
        elif difference == -1:
            end_vertices.append(vertex)
        elif difference != 0:
            return None

    if not (
        (len(start_vertices) == 1 and len(end_vertices) == 1)
        or (len(start_vertices) == 0 and len(end_vertices) == 0)
    ):
        return None

    if start_vertices:
        start = start_vertices[0]
    else:
        start = next(
            vertex for vertex in range(vertex_count) if out_degree[vertex] > 0
        )

    # Iterative Hierholzer traversal: consuming every edge but leaving some
    # edges in a disconnected component is detected by the final length check.
    stack = [start]
    reversed_trail: List[int] = []
    while stack:
        vertex = stack[-1]
        if adjacency[vertex]:
            stack.append(adjacency[vertex].pop())
        else:
            reversed_trail.append(stack.pop())

    if len(reversed_trail) != edge_count + 1:
        return None

    reversed_trail.reverse()
    return reversed_trail


if __name__ == "__main__":
    from collections import Counter
    import unittest

    class EulerianTrailTests(unittest.TestCase):
        @staticmethod
        def assert_uses_each_edge(test_case, trail, edges):
            """Check the output's directed edge multiset independently."""
            test_case.assertIsNotNone(trail)
            test_case.assertEqual(len(trail), len(edges) + 1)
            actual_edges = Counter(zip(trail, trail[1:]))
            expected_edges = Counter(tuple(edge) for edge in edges)
            test_case.assertEqual(actual_edges, expected_edges)

        def test_circuit(self):
            edges = [(0, 1), (1, 2), (2, 0)]
            trail = eulerian_trail(3, edges)
            self.assert_uses_each_edge(self, trail, edges)
            self.assertEqual(trail[0], trail[-1])

        def test_open_trail(self):
            edges = [(0, 1), (1, 2), (2, 1)]
            trail = eulerian_trail(3, edges)
            self.assert_uses_each_edge(self, trail, edges)
            self.assertEqual((trail[0], trail[-1]), (0, 1))

        def test_disconnected_edge_components_have_no_trail(self):
            self.assertIsNone(eulerian_trail(4, [(0, 1), (1, 0), (2, 3), (3, 2)]))

        def test_isolated_vertices_are_allowed(self):
            edges = [(1, 2), (2, 1)]
            trail = eulerian_trail(5, edges)
            self.assert_uses_each_edge(self, trail, edges)
            self.assertNotIn(0, trail)
            self.assertNotIn(3, trail)

        def test_parallel_edges_are_preserved(self):
            edges = [(0, 1), (0, 1), (1, 0), (1, 0)]
            trail = eulerian_trail(2, edges)
            self.assert_uses_each_edge(self, trail, edges)

        def test_self_loops_are_preserved(self):
            edges = [(0, 0), (0, 1), (1, 1), (1, 0)]
            trail = eulerian_trail(2, edges)
            self.assert_uses_each_edge(self, trail, edges)

        def test_edgeless_graph_returns_empty_sequence(self):
            self.assertEqual(eulerian_trail(3, []), [])
            self.assertEqual(eulerian_trail(0, []), [])

        def test_invalid_vertex_count(self):
            for value in (-1, 1.5, True):
                with self.subTest(vertex_count=value):
                    with self.assertRaises((TypeError, ValueError)):
                        eulerian_trail(value, [])

        def test_invalid_endpoints(self):
            for edges in ([(-1, 0)], [(0, 2)], [("0", 0)], [(True, 0)]):
                with self.subTest(edges=edges):
                    with self.assertRaises((TypeError, ValueError)):
                        eulerian_trail(2, edges)

        def test_malformed_edges(self):
            with self.assertRaises(TypeError):
                eulerian_trail(2, [None])
            for edge in ((), (0,), (0, 1, 0)):
                with self.subTest(edge=edge):
                    with self.assertRaises(ValueError):
                        eulerian_trail(2, [edge])
            with self.assertRaises(TypeError):
                eulerian_trail(2, None)

    unittest.main()
