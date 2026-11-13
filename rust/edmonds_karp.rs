use std::collections::VecDeque;
use std::error::Error;
use std::fmt;

/// A directed edge with an unsigned capacity.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct InputEdge {
    pub from: usize,
    pub to: usize,
    pub capacity: u64,
}

impl InputEdge {
    /// Constructs an edge from `from` to `to` with the given capacity.
    pub fn new(from: usize, to: usize, capacity: u64) -> Self {
        Self { from, to, capacity }
    }
}

/// Errors returned by [`edmonds_karp`].
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum EdmondsKarpError {
    /// A graph must contain at least one vertex.
    InvalidVertexCount,
    /// A source, sink, or edge endpoint is not in `0..vertex_count`.
    InvalidVertex {
        vertex: usize,
        vertex_count: usize,
    },
    /// The maximum flow or a residual capacity cannot be represented by `u64`.
    CapacityOverflow,
}

impl fmt::Display for EdmondsKarpError {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::InvalidVertexCount => formatter.write_str("vertex count must be positive"),
            Self::InvalidVertex {
                vertex,
                vertex_count,
            } => write!(
                formatter,
                "vertex {vertex} is outside the graph with {vertex_count} vertices"
            ),
            Self::CapacityOverflow => {
                formatter.write_str("maximum flow or residual capacity exceeds u64")
            }
        }
    }
}

impl Error for EdmondsKarpError {}

#[derive(Debug)]
struct ResidualEdge {
    to: usize,
    reverse_index: usize,
    residual_capacity: u64,
}

/// Computes the maximum flow from `source` to `sink` in a directed graph.
///
/// Each edge is represented by its own residual pair, so parallel and
/// antiparallel input edges are supported without merging capacities. A
/// self-loop is also accepted. All vertices and edges are validated before
/// returning; equal source and sink yields zero after validation. Returns a
/// typed error for an empty vertex set, invalid vertex indices, or checked
/// `u64` flow/residual arithmetic that overflows.
///
/// Time: O(V * E^2), where V is the vertex count and E is the number of input
/// edges (counting parallel edges); Edmonds–Karp performs O(V * E) augmentations
/// and each BFS scans O(E) residual edges. Auxiliary space: O(V + E), counting
/// the residual graph and the BFS queue/parent arrays, and excluding the input.
pub fn edmonds_karp(
    vertex_count: usize,
    edges: &[InputEdge],
    source: usize,
    sink: usize,
) -> Result<u64, EdmondsKarpError> {
    if vertex_count == 0 {
        return Err(EdmondsKarpError::InvalidVertexCount);
    }
    for vertex in [source, sink] {
        if vertex >= vertex_count {
            return Err(EdmondsKarpError::InvalidVertex {
                vertex,
                vertex_count,
            });
        }
    }
    for edge in edges {
        for vertex in [edge.from, edge.to] {
            if vertex >= vertex_count {
                return Err(EdmondsKarpError::InvalidVertex {
                    vertex,
                    vertex_count,
                });
            }
        }
    }
    if source == sink {
        return Ok(0);
    }

    let mut graph: Vec<Vec<ResidualEdge>> = (0..vertex_count).map(|_| Vec::new()).collect();
    for edge in edges {
        let forward_index = graph[edge.from].len();
        let reverse_index = graph[edge.to].len();
        if edge.from == edge.to {
            graph[edge.from].push(ResidualEdge {
                to: edge.to,
                reverse_index: forward_index + 1,
                residual_capacity: edge.capacity,
            });
            graph[edge.from].push(ResidualEdge {
                to: edge.from,
                reverse_index: forward_index,
                residual_capacity: 0,
            });
        } else {
            graph[edge.from].push(ResidualEdge {
                to: edge.to,
                reverse_index,
                residual_capacity: edge.capacity,
            });
            graph[edge.to].push(ResidualEdge {
                to: edge.from,
                reverse_index: forward_index,
                residual_capacity: 0,
            });
        }
    }

    let mut total_flow = 0_u64;
    loop {
        let mut parent: Vec<Option<(usize, usize)>> = vec![None; vertex_count];
        let mut visited = vec![false; vertex_count];
        let mut queue = VecDeque::new();
        visited[source] = true;
        queue.push_back(source);

        while let Some(vertex) = queue.pop_front() {
            if visited[sink] {
                break;
            }
            for (edge_index, edge) in graph[vertex].iter().enumerate() {
                if edge.residual_capacity > 0 && !visited[edge.to] {
                    visited[edge.to] = true;
                    parent[edge.to] = Some((vertex, edge_index));
                    queue.push_back(edge.to);
                    if edge.to == sink {
                        break;
                    }
                }
            }
        }
        if !visited[sink] {
            return Ok(total_flow);
        }

        let mut augment = u64::MAX;
        let mut vertex = sink;
        while vertex != source {
            let (previous, edge_index) = parent[vertex]
                .expect("every visited non-source vertex has a BFS parent");
            augment = augment.min(graph[previous][edge_index].residual_capacity);
            vertex = previous;
        }
        let new_total = total_flow
            .checked_add(augment)
            .ok_or(EdmondsKarpError::CapacityOverflow)?;

        vertex = sink;
        while vertex != source {
            let (previous, edge_index) = parent[vertex]
                .expect("every visited non-source vertex has a BFS parent");
            let reverse_index = graph[previous][edge_index].reverse_index;
            graph[previous][edge_index].residual_capacity -= augment;
            graph[vertex][reverse_index].residual_capacity = graph[vertex][reverse_index]
                .residual_capacity
                .checked_add(augment)
                .ok_or(EdmondsKarpError::CapacityOverflow)?;
            vertex = previous;
        }
        total_flow = new_total;
    }
}

#[cfg(test)]
mod tests {
    use super::{edmonds_karp, EdmondsKarpError, InputEdge};

    fn edge(from: usize, to: usize, capacity: u64) -> InputEdge {
        InputEdge::new(from, to, capacity)
    }

    #[test]
    fn matches_classic_six_vertex_network() {
        let edges = [
            edge(0, 1, 16),
            edge(0, 2, 13),
            edge(1, 2, 10),
            edge(2, 1, 4),
            edge(1, 3, 12),
            edge(3, 2, 9),
            edge(2, 4, 14),
            edge(4, 3, 7),
            edge(3, 5, 20),
            edge(4, 5, 4),
        ];
        assert_eq!(edmonds_karp(6, &edges, 0, 5), Ok(23));
    }

    #[test]
    fn disconnected_sink_has_zero_flow() {
        let edges = [edge(0, 1, 9), edge(1, 2, 3)];
        assert_eq!(edmonds_karp(4, &edges, 0, 3), Ok(0));
    }

    #[test]
    fn parallel_and_antiparallel_edges_keep_separate_residual_pairs() {
        let edges = [
            edge(0, 1, 3),
            edge(0, 1, 5),
            edge(1, 0, 11),
            edge(1, 2, 10),
            edge(2, 1, 7),
        ];
        assert_eq!(edmonds_karp(3, &edges, 0, 2), Ok(8));
    }

    #[test]
    fn reverse_residual_edge_allows_rerouting() {
        // The first shortest augmenting path is 0->1->3->5. A second unit
        // requires cancelling 1->3 via its reverse residual edge 3->1.
        let edges = [
            edge(0, 1, 1),
            edge(0, 2, 1),
            edge(1, 3, 1),
            edge(1, 4, 1),
            edge(2, 3, 1),
            edge(3, 5, 1),
            edge(4, 5, 1),
        ];
        assert_eq!(edmonds_karp(6, &edges, 0, 5), Ok(2));
    }

    #[test]
    fn deterministic_small_graphs_match_known_flow_values() {
        let cases: &[(usize, &[InputEdge], usize, usize, u64)] = &[
            (2, &[edge(0, 1, 0)], 0, 1, 0),
            (2, &[edge(0, 1, 17)], 0, 1, 17),
            (
                4,
                &[edge(0, 1, 3), edge(0, 2, 4), edge(1, 3, 3), edge(2, 3, 4)],
                0,
                3,
                7,
            ),
            (
                4,
                &[edge(0, 1, 5), edge(1, 2, 2), edge(2, 3, 8), edge(0, 3, 1)],
                0,
                3,
                3,
            ),
        ];
        for &(vertex_count, edges, source, sink, expected) in cases {
            assert_eq!(
                edmonds_karp(vertex_count, edges, source, sink),
                Ok(expected)
            );
        }
    }

    #[test]
    fn rejects_invalid_vertex_count_and_vertex_indices() {
        assert_eq!(
            edmonds_karp(0, &[], 0, 0),
            Err(EdmondsKarpError::InvalidVertexCount)
        );
        assert_eq!(
            edmonds_karp(2, &[], 2, 1),
            Err(EdmondsKarpError::InvalidVertex {
                vertex: 2,
                vertex_count: 2,
            })
        );
        assert_eq!(
            edmonds_karp(2, &[edge(0, 2, 1)], 0, 1),
            Err(EdmondsKarpError::InvalidVertex {
                vertex: 2,
                vertex_count: 2,
            })
        );
        assert_eq!(
            edmonds_karp(2, &[edge(0, 1, 1)], 0, 3),
            Err(EdmondsKarpError::InvalidVertex {
                vertex: 3,
                vertex_count: 2,
            })
        );
    }

    #[test]
    fn source_equal_to_sink_is_zero_after_input_validation() {
        assert_eq!(edmonds_karp(2, &[edge(0, 1, 7)], 1, 1), Ok(0));
        assert_eq!(
            edmonds_karp(2, &[edge(0, 2, 7)], 1, 1),
            Err(EdmondsKarpError::InvalidVertex {
                vertex: 2,
                vertex_count: 2,
            })
        );
    }

    #[test]
    fn detects_flow_sum_overflow() {
        let edges = [edge(0, 1, u64::MAX), edge(0, 1, 1)];
        assert_eq!(
            edmonds_karp(2, &edges, 0, 1),
            Err(EdmondsKarpError::CapacityOverflow)
        );
    }

    #[test]
    fn self_loops_do_not_corrupt_residual_edge_indices() {
        let edges = [edge(0, 0, u64::MAX), edge(0, 1, 6), edge(1, 1, 3)];
        assert_eq!(edmonds_karp(2, &edges, 0, 1), Ok(6));
    }
}
