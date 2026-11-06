use std::cmp::Reverse;
use std::collections::BinaryHeap;
use std::convert::TryFrom;
use std::fmt;
use std::num::TryFromIntError;

/// One edge in a minimum spanning forest, oriented from an already-visited
/// vertex to the vertex it adds to the forest.
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct SelectedEdge {
    pub from: usize,
    pub to: usize,
    pub weight: i64,
}

/// The minimum spanning forest of an undirected graph.
#[derive(Debug, Clone, PartialEq, Eq)]
pub struct ForestResult {
    pub total_weight: i64,
    pub component_count: usize,
    pub selected_edges: Vec<SelectedEdge>,
}

/// Failures returned by [`minimum_spanning_forest`].
#[derive(Debug, Clone, PartialEq, Eq)]
pub enum ForestError {
    /// An edge endpoint is not a valid vertex index for the supplied graph.
    InvalidEndpoint {
        endpoint: usize,
        vertex_count: usize,
    },
    /// The exact total weight cannot be represented as an `i64`.
    TotalWeightOverflow,
}

impl fmt::Display for ForestError {
    fn fmt(&self, formatter: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::InvalidEndpoint {
                endpoint,
                vertex_count,
            } => write!(
                formatter,
                "edge endpoint {endpoint} is outside a graph with {vertex_count} vertices"
            ),
            Self::TotalWeightOverflow => {
                write!(formatter, "minimum spanning forest total overflows i64")
            }
        }
    }
}

impl std::error::Error for ForestError {}

/// Computes a minimum spanning forest for an undirected graph.
///
/// `edges` contains `(u, v, weight)` tuples; each tuple describes one
/// undirected edge. Negative weights, parallel edges, and self-loops are
/// supported. Every endpoint is validated before traversal. The result has
/// one selected edge per non-root vertex in each component, and an empty graph
/// has zero components.
///
/// A wider accumulator is used so an intermediate partial sum outside the
/// `i64` range does not mask a representable final total. An error is returned
/// if the final exact total does not fit in `i64`.
///
/// Time: O((V + E) log V) for a simple undirected graph using a binary heap;
/// space: O(V + E), including the adjacency list, visited flags, heap, and
/// result. Here V is the vertex count and E is the number of supplied edges.
pub fn minimum_spanning_forest(
    vertex_count: usize,
    edges: &[(usize, usize, i64)],
) -> Result<ForestResult, ForestError> {
    let mut adjacency = vec![Vec::<(usize, i64)>::new(); vertex_count];

    for &(from, to, weight) in edges {
        if from >= vertex_count {
            return Err(ForestError::InvalidEndpoint {
                endpoint: from,
                vertex_count,
            });
        }
        if to >= vertex_count {
            return Err(ForestError::InvalidEndpoint {
                endpoint: to,
                vertex_count,
            });
        }
        adjacency[from].push((to, weight));
        adjacency[to].push((from, weight));
    }

    // Reverse changes BinaryHeap's max ordering into a min-priority queue.
    let mut candidates = BinaryHeap::<Reverse<(i64, usize, usize)>>::new();
    let mut visited = vec![false; vertex_count];
    let mut total_weight = 0_i128;
    let mut component_count = 0;
    let mut selected_edges = Vec::with_capacity(vertex_count.saturating_sub(1));

    for root in 0..vertex_count {
        if visited[root] {
            continue;
        }

        component_count += 1;
        visited[root] = true;
        for &(to, weight) in &adjacency[root] {
            if !visited[to] {
                candidates.push(Reverse((weight, root, to)));
            }
        }

        while let Some(Reverse((weight, from, to))) = candidates.pop() {
            if visited[to] {
                continue;
            }

            visited[to] = true;
            total_weight += i128::from(weight);
            selected_edges.push(SelectedEdge { from, to, weight });

            for &(next, next_weight) in &adjacency[to] {
                if !visited[next] {
                    candidates.push(Reverse((next_weight, to, next)));
                }
            }
        }
    }

    let total_weight = i64::try_from(total_weight).map_err(|_error: TryFromIntError| {
        ForestError::TotalWeightOverflow
    })?;

    Ok(ForestResult {
        total_weight,
        component_count,
        selected_edges,
    })
}

#[cfg(test)]
mod tests {
    use super::{minimum_spanning_forest, ForestError, ForestResult};

    #[test]
    fn connected_graph_selects_a_minimum_tree() {
        let edges = [(0, 1, 1), (0, 2, 4), (1, 2, 2), (1, 3, 5), (2, 3, 3)];

        let result = minimum_spanning_forest(4, &edges).expect("valid connected graph");

        assert_eq!(result.total_weight, 6);
        assert_eq!(result.component_count, 1);
        assert_eq!(result.selected_edges.len(), 3);
    }

    #[test]
    fn disconnected_graph_returns_a_forest() {
        let edges = [(0, 1, 4), (1, 2, 1), (3, 4, 2)];

        let result = minimum_spanning_forest(5, &edges).expect("valid disconnected graph");

        assert_eq!(result.total_weight, 7);
        assert_eq!(result.component_count, 2);
        assert_eq!(result.selected_edges.len(), 3);
    }

    #[test]
    fn negative_weights_are_supported() {
        let edges = [(0, 1, -5), (1, 2, 2), (0, 2, 1)];

        let result = minimum_spanning_forest(3, &edges).expect("valid negative-weight graph");

        assert_eq!(result.total_weight, -4);
        assert_eq!(result.component_count, 1);
        assert_eq!(result.selected_edges.len(), 2);
    }

    #[test]
    fn empty_and_singleton_graphs_have_no_selected_edges() {
        assert_eq!(
            minimum_spanning_forest(0, &[]).expect("empty graph"),
            ForestResult {
                total_weight: 0,
                component_count: 0,
                selected_edges: vec![],
            }
        );
        assert_eq!(
            minimum_spanning_forest(1, &[]).expect("singleton graph"),
            ForestResult {
                total_weight: 0,
                component_count: 1,
                selected_edges: vec![],
            }
        );
    }

    #[test]
    fn invalid_endpoints_return_a_typed_error() {
        let error = minimum_spanning_forest(2, &[(0, 2, 1)]).unwrap_err();

        assert_eq!(
            error,
            ForestError::InvalidEndpoint {
                endpoint: 2,
                vertex_count: 2,
            }
        );
    }

    #[test]
    fn final_total_overflow_returns_a_typed_error() {
        let edges = [(0, 1, i64::MAX), (1, 2, 1)];

        assert_eq!(
            minimum_spanning_forest(3, &edges).unwrap_err(),
            ForestError::TotalWeightOverflow
        );
    }

    #[test]
    fn intermediate_sum_overflow_can_still_have_a_valid_final_total() {
        let edges = [(0, 1, i64::MAX), (1, 2, 1), (2, 3, -1)];

        let result = minimum_spanning_forest(4, &edges).expect("final total fits in i64");

        assert_eq!(result.total_weight, i64::MAX);
        assert_eq!(result.component_count, 1);
        assert_eq!(result.selected_edges.len(), 3);
    }
}
