use std::cmp::Reverse;
use std::collections::BinaryHeap;

/// Computes shortest-path distances from `source` in a graph with nonnegative
/// `u64` edge weights.
///
/// The graph is an adjacency list: each `(neighbor, weight)` pair represents a
/// directed edge. Entries whose neighbor index is outside `graph` are ignored,
/// so malformed adjacency lists are handled safely without indexing panics. If
/// `source` is outside the graph, the returned vector contains only `None`.
/// A `None` distance means either the vertex is unreachable or its shortest
/// distance is larger than `u64::MAX` and therefore cannot be represented.
///
/// Time: O((V + E) log(E + 1)) with a binary heap that may hold duplicate
/// entries; space: O(V + E) auxiliary in the worst case (distance vector and
/// heap). Here V is the number of vertices and E is the number of adjacency
/// entries, including invalid entries.
pub fn dijkstra(graph: &[Vec<(usize, u64)>], source: usize) -> Vec<Option<u64>> {
    let vertex_count = graph.len();
    let mut distances = vec![None; vertex_count];

    if source >= vertex_count {
        return distances;
    }

    let mut heap = BinaryHeap::new();
    distances[source] = Some(0);
    heap.push(Reverse((0_u64, source)));

    while let Some(Reverse((distance, vertex))) = heap.pop() {
        // Ignore stale heap entries superseded by a shorter path.
        if distances[vertex] != Some(distance) {
            continue;
        }

        for &(neighbor, weight) in &graph[vertex] {
            // Ignore malformed edges rather than indexing outside `graph`.
            if neighbor >= vertex_count {
                continue;
            }

            let Some(candidate) = distance.checked_add(weight) else {
                // This route cannot be represented as a u64 distance.
                continue;
            };

            if distances[neighbor].map_or(true, |current| candidate < current) {
                distances[neighbor] = Some(candidate);
                heap.push(Reverse((candidate, neighbor)));
            }
        }
    }

    distances
}

#[cfg(test)]
mod tests {
    use super::dijkstra;

    #[test]
    fn reachable_and_unreachable_vertices() {
        let graph = vec![
            vec![(1, 4), (2, 1)],
            vec![(3, 1)],
            vec![(1, 2), (3, 5)],
            vec![],
            vec![],
        ];

        assert_eq!(dijkstra(&graph, 0), vec![Some(0), Some(3), Some(1), Some(4), None]);
    }

    #[test]
    fn zero_weight_edges_are_supported() {
        let graph = vec![vec![(1, 0)], vec![(2, 0)], vec![]];

        assert_eq!(dijkstra(&graph, 0), vec![Some(0), Some(0), Some(0)]);
    }

    #[test]
    fn overflowing_paths_are_skipped_safely() {
        let graph = vec![
            vec![(1, u64::MAX), (2, 7)],
            vec![(2, 1)],
            vec![],
        ];

        // The path through vertex 1 overflows; the representable path remains.
        assert_eq!(dijkstra(&graph, 0), vec![Some(0), Some(u64::MAX), Some(7)]);
    }

    #[test]
    fn invalid_neighbors_and_source_are_handled_safely() {
        let graph = vec![vec![(1, 2), (99, 1)], vec![]];

        assert_eq!(dijkstra(&graph, 0), vec![Some(0), Some(2)]);
        assert_eq!(dijkstra(&graph, 2), vec![None, None]);
    }
}
