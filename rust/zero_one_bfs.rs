use std::collections::VecDeque;

/// A directed edge whose weight must be exactly zero or one.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub struct Edge {
    pub from: usize,
    pub to: usize,
    pub weight: i32,
}

/// Errors returned when the graph or source is invalid.
#[derive(Clone, Copy, Debug, Eq, PartialEq)]
pub enum ZeroOneBfsError {
    /// The source index is not a vertex in the graph.
    InvalidSource { source: usize, vertex_count: usize },
    /// An edge endpoint is not a vertex in the graph.
    InvalidEndpoint {
        edge_index: usize,
        endpoint: usize,
        vertex_count: usize,
    },
    /// Edge weights are restricted to zero and one.
    InvalidWeight { edge_index: usize, weight: i32 },
}

/// Computes shortest-path distances from `source` in a directed 0–1 weighted graph.
///
/// The graph is provided as a list of edges. Each edge is validated before the
/// search begins, and invalid sources, endpoints, or weights return an error.
/// The result contains `Some(distance)` for reachable vertices and `None` for
/// disconnected vertices. A deque places zero-weight relaxations at the front
/// and one-weight relaxations at the back, which processes vertices in
/// nondecreasing tentative-distance order without a heap.
///
/// Time: O(V + E), where V is `vertex_count` and E is the number of edges.
/// Auxiliary space: O(V + E) for the adjacency list, distances, and deque.
pub fn zero_one_bfs(
    vertex_count: usize,
    edges: &[Edge],
    source: usize,
) -> Result<Vec<Option<usize>>, ZeroOneBfsError> {
    if source >= vertex_count {
        return Err(ZeroOneBfsError::InvalidSource {
            source,
            vertex_count,
        });
    }

    let mut adjacency = vec![Vec::new(); vertex_count];
    for (edge_index, edge) in edges.iter().enumerate() {
        for endpoint in [edge.from, edge.to] {
            if endpoint >= vertex_count {
                return Err(ZeroOneBfsError::InvalidEndpoint {
                    edge_index,
                    endpoint,
                    vertex_count,
                });
            }
        }
        if edge.weight != 0 && edge.weight != 1 {
            return Err(ZeroOneBfsError::InvalidWeight {
                edge_index,
                weight: edge.weight,
            });
        }
        adjacency[edge.from].push((edge.to, edge.weight as usize));
    }

    let mut distances = vec![None; vertex_count];
    let mut deque = VecDeque::new();
    distances[source] = Some(0);
    deque.push_front((source, 0_usize));

    while let Some((vertex, queued_distance)) = deque.pop_front() {
        // A later relaxation may have made this queued entry obsolete.
        if distances[vertex] != Some(queued_distance) {
            continue;
        }
        for &(neighbor, weight) in &adjacency[vertex] {
            let Some(candidate) = queued_distance.checked_add(weight) else {
                // A path too large to represent as usize cannot improve a distance.
                continue;
            };
            if distances[neighbor].map_or(true, |current| candidate < current) {
                distances[neighbor] = Some(candidate);
                if weight == 0 {
                    deque.push_front((neighbor, candidate));
                } else {
                    deque.push_back((neighbor, candidate));
                }
            }
        }
    }

    Ok(distances)
}

#[cfg(test)]
mod tests {
    use super::{zero_one_bfs, Edge, ZeroOneBfsError};
    use std::cmp::Reverse;
    use std::collections::BinaryHeap;

    fn edge(from: usize, to: usize, weight: i32) -> Edge {
        Edge { from, to, weight }
    }

    fn dijkstra_reference(vertex_count: usize, edges: &[Edge], source: usize) -> Vec<Option<usize>> {
        let mut adjacency = vec![Vec::new(); vertex_count];
        for edge in edges {
            adjacency[edge.from].push((edge.to, edge.weight as usize));
        }
        let mut distances = vec![None; vertex_count];
        let mut heap = BinaryHeap::new();
        distances[source] = Some(0);
        heap.push(Reverse((0_usize, source)));
        while let Some(Reverse((distance, vertex))) = heap.pop() {
            if distances[vertex] != Some(distance) {
                continue;
            }
            for &(neighbor, weight) in &adjacency[vertex] {
                let candidate = distance + weight;
                if distances[neighbor].map_or(true, |current| candidate < current) {
                    distances[neighbor] = Some(candidate);
                    heap.push(Reverse((candidate, neighbor)));
                }
            }
        }
        distances
    }

    #[test]
    fn handles_zero_weight_cycles_and_mixed_routes() {
        let edges = [
            edge(0, 1, 0),
            edge(1, 2, 0),
            edge(2, 0, 0),
            edge(2, 3, 1),
            edge(3, 1, 0),
            edge(0, 4, 1),
            edge(3, 4, 0),
        ];
        assert_eq!(
            zero_one_bfs(5, &edges, 0),
            Ok(vec![Some(0), Some(0), Some(0), Some(1), Some(1)])
        );
    }

    #[test]
    fn represents_disconnected_vertices_and_source_distance() {
        let edges = [edge(0, 1, 1), edge(2, 3, 0)];
        assert_eq!(
            zero_one_bfs(5, &edges, 0),
            Ok(vec![Some(0), Some(1), None, None, None])
        );
        assert_eq!(
            zero_one_bfs(5, &edges, 2),
            Ok(vec![None, None, Some(0), Some(0), None])
        );
    }

    #[test]
    fn rejects_invalid_sources_endpoints_and_weights() {
        assert_eq!(
            zero_one_bfs(2, &[], 2),
            Err(ZeroOneBfsError::InvalidSource {
                source: 2,
                vertex_count: 2
            })
        );
        assert_eq!(
            zero_one_bfs(2, &[edge(0, 2, 1)], 0),
            Err(ZeroOneBfsError::InvalidEndpoint {
                edge_index: 0,
                endpoint: 2,
                vertex_count: 2
            })
        );
        assert_eq!(
            zero_one_bfs(2, &[edge(0, 1, -1)], 0),
            Err(ZeroOneBfsError::InvalidWeight {
                edge_index: 0,
                weight: -1
            })
        );
        assert_eq!(
            zero_one_bfs(2, &[edge(0, 1, 2)], 0),
            Err(ZeroOneBfsError::InvalidWeight {
                edge_index: 0,
                weight: 2
            })
        );
    }

    #[test]
    fn matches_dijkstra_on_deterministic_graphs() {
        // Deterministically generate varied directed graphs, including loops,
        // parallel edges, disconnected components, and both valid weights.
        let mut state = 0x9e37_79b9_u64;
        for vertex_count in 1..=8 {
            for _case in 0..40 {
                let mut edges = Vec::new();
                for from in 0..vertex_count {
                    for to in 0..vertex_count {
                        state = state
                            .wrapping_mul(6_364_136_223_846_793_005)
                            .wrapping_add(1_442_695_040_888_963_407);
                        if state >> 61 != 0 {
                            edges.push(edge(from, to, ((state >> 17) & 1) as i32));
                        }
                    }
                }
                let source = (state as usize) % vertex_count;
                assert_eq!(
                    zero_one_bfs(vertex_count, &edges, source),
                    Ok(dijkstra_reference(vertex_count, &edges, source)),
                    "graph with {vertex_count} vertices and source {source}"
                );
            }
        }
    }
}
