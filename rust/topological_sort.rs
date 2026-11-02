use std::collections::VecDeque;

/// An error returned when a topological ordering cannot be produced.
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum TopologicalSortError {
    /// An edge endpoint is not in `0..vertex_count`.
    InvalidEndpoint {
        edge_index: usize,
        endpoint: usize,
        vertex_count: usize,
    },
    /// The graph contains a directed cycle.
    Cycle,
}

/// Returns a deterministic topological ordering of a directed graph.
///
/// Vertices are indexed from `0` to `vertex_count - 1`, and each edge is a
/// `(from, to)` pair. Initial zero-indegree vertices are queued in ascending
/// order; newly ready vertices are appended in input edge order, matching FIFO
/// Kahn traversal. Returns [`TopologicalSortError::InvalidEndpoint`] when an
/// endpoint is outside the vertex range, or [`TopologicalSortError::Cycle`]
/// when no complete ordering exists.
///
/// Time complexity: O(V + E). Space complexity: O(V + E), where V is the
/// vertex count and E is the number of edges.
pub fn topological_sort(
    vertex_count: usize,
    edges: &[(usize, usize)],
) -> Result<Vec<usize>, TopologicalSortError> {
    let mut adjacency = vec![Vec::new(); vertex_count];
    let mut in_degree = vec![0; vertex_count];

    for (edge_index, &(from, to)) in edges.iter().enumerate() {
        if from >= vertex_count {
            return Err(TopologicalSortError::InvalidEndpoint {
                edge_index,
                endpoint: from,
                vertex_count,
            });
        }
        if to >= vertex_count {
            return Err(TopologicalSortError::InvalidEndpoint {
                edge_index,
                endpoint: to,
                vertex_count,
            });
        }

        adjacency[from].push(to);
        in_degree[to] += 1;
    }

    let mut ready = VecDeque::new();
    for (vertex, &degree) in in_degree.iter().enumerate() {
        if degree == 0 {
            ready.push_back(vertex);
        }
    }

    let mut ordering = Vec::with_capacity(vertex_count);
    while let Some(vertex) = ready.pop_front() {
        ordering.push(vertex);

        for &neighbor in &adjacency[vertex] {
            in_degree[neighbor] -= 1;
            if in_degree[neighbor] == 0 {
                ready.push_back(neighbor);
            }
        }
    }

    if ordering.len() == vertex_count {
        Ok(ordering)
    } else {
        Err(TopologicalSortError::Cycle)
    }
}

#[cfg(test)]
mod tests {
    use super::{topological_sort, TopologicalSortError};

    #[test]
    fn empty_and_disconnected_dags_are_ordered_deterministically() {
        assert_eq!(topological_sort(0, &[]), Ok(vec![]));

        let edges = [(0, 1), (3, 4)];
        assert_eq!(topological_sort(6, &edges), Ok(vec![0, 2, 3, 5, 1, 4]));
    }

    #[test]
    fn respects_edges_and_matches_fifo_edge_order() {
        let edges = [(5, 2), (5, 0), (4, 0), (4, 1), (2, 3), (3, 1)];
        let ordering = topological_sort(6, &edges).unwrap();
        assert_eq!(ordering, vec![4, 5, 2, 0, 3, 1]);

        let mut position = vec![0; ordering.len()];
        for (index, &vertex) in ordering.iter().enumerate() {
            position[vertex] = index;
        }
        for &(from, to) in &edges {
            assert!(position[from] < position[to], "edge {from}->{to} violated");
        }
    }

    #[test]
    fn reports_cycles_and_self_loops() {
        assert_eq!(
            topological_sort(3, &[(0, 1), (1, 2), (2, 0)]),
            Err(TopologicalSortError::Cycle)
        );
        assert_eq!(
            topological_sort(1, &[(0, 0)]),
            Err(TopologicalSortError::Cycle)
        );
    }

    #[test]
    fn reports_out_of_range_endpoints() {
        assert_eq!(
            topological_sort(2, &[(0, 2)]),
            Err(TopologicalSortError::InvalidEndpoint {
                edge_index: 0,
                endpoint: 2,
                vertex_count: 2,
            })
        );
        assert_eq!(
            topological_sort(2, &[(usize::MAX, 1)]),
            Err(TopologicalSortError::InvalidEndpoint {
                edge_index: 0,
                endpoint: usize::MAX,
                vertex_count: 2,
            })
        );
    }
}
