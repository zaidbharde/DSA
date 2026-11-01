//! Disjoint-set union (union-find) with path compression and union by size.
//!
//! `find`, `union`, and `connected` take amortized inverse-Ackermann time,
//! conventionally written O(α(n)); `new` takes O(n) time. The structure uses
//! O(n) storage. Public operations that accept indices return `None` for an
//! index outside `0..len()` rather than panicking.

/// A disjoint-set data structure over the elements `0..len()`.
#[derive(Debug, Clone)]
pub struct DisjointSet {
    parent: Vec<usize>,
    size: Vec<usize>,
    components: usize,
}

impl DisjointSet {
    /// Creates `len` singleton components, one for each index in `0..len()`.
    pub fn new(len: usize) -> Self {
        Self {
            parent: (0..len).collect(),
            size: vec![1; len],
            components: len,
        }
    }

    /// Returns the number of elements in the structure.
    pub fn len(&self) -> usize {
        self.parent.len()
    }

    /// Returns whether the structure contains no elements.
    pub fn is_empty(&self) -> bool {
        self.parent.is_empty()
    }

    /// Returns the number of current connected components.
    pub fn component_count(&self) -> usize {
        self.components
    }

    /// Finds and returns the representative of `element`, compressing its path.
    ///
    /// Returns `None` if `element` is not in `0..len()`.
    pub fn find(&mut self, element: usize) -> Option<usize> {
        if element >= self.parent.len() {
            return None;
        }

        let mut root = element;
        while self.parent[root] != root {
            root = self.parent[root];
        }

        let mut current = element;
        while self.parent[current] != current {
            let next = self.parent[current];
            self.parent[current] = root;
            current = next;
        }

        Some(root)
    }

    /// Merges the components containing `a` and `b`.
    ///
    /// Returns `Some(true)` if a merge occurred, `Some(false)` if they were
    /// already in the same component, or `None` if either index is invalid.
    pub fn union(&mut self, a: usize, b: usize) -> Option<bool> {
        if a >= self.parent.len() || b >= self.parent.len() {
            return None;
        }

        let mut root_a = self.find(a)?;
        let mut root_b = self.find(b)?;
        if root_a == root_b {
            return Some(false);
        }

        if self.size[root_a] < self.size[root_b] {
            std::mem::swap(&mut root_a, &mut root_b);
        }
        self.parent[root_b] = root_a;
        self.size[root_a] += self.size[root_b];
        self.components -= 1;
        Some(true)
    }

    /// Returns whether `a` and `b` belong to the same component.
    ///
    /// Returns `None` if either index is invalid.
    pub fn connected(&mut self, a: usize, b: usize) -> Option<bool> {
        if a >= self.parent.len() || b >= self.parent.len() {
            return None;
        }
        Some(self.find(a)? == self.find(b)?)
    }
}

#[cfg(test)]
mod tests {
    use super::DisjointSet;

    #[test]
    fn unions_merge_components_and_update_count() {
        let mut sets = DisjointSet::new(6);
        assert_eq!(sets.component_count(), 6);
        assert_eq!(sets.union(0, 1), Some(true));
        assert_eq!(sets.union(1, 2), Some(true));
        assert_eq!(sets.union(3, 4), Some(true));
        assert_eq!(sets.component_count(), 3);
        assert_eq!(sets.union(2, 4), Some(true));
        assert_eq!(sets.component_count(), 2);
    }

    #[test]
    fn repeated_union_does_not_change_component_count() {
        let mut sets = DisjointSet::new(3);
        assert_eq!(sets.union(0, 1), Some(true));
        assert_eq!(sets.union(1, 0), Some(false));
        assert_eq!(sets.union(0, 0), Some(false));
        assert_eq!(sets.component_count(), 2);
    }

    #[test]
    fn connectedness_tracks_unions() {
        let mut sets = DisjointSet::new(4);
        assert_eq!(sets.connected(0, 1), Some(false));
        assert_eq!(sets.union(0, 1), Some(true));
        assert_eq!(sets.connected(0, 1), Some(true));
        assert_eq!(sets.connected(1, 0), Some(true));
        assert_eq!(sets.connected(0, 2), Some(false));
    }

    #[test]
    fn find_returns_a_shared_representative() {
        let mut sets = DisjointSet::new(4);
        assert_eq!(sets.union(0, 1), Some(true));
        assert_eq!(sets.union(1, 2), Some(true));
        assert_eq!(sets.find(0), sets.find(2));
    }

    #[test]
    fn invalid_indices_are_reported_without_panicking() {
        let mut sets = DisjointSet::new(2);
        assert_eq!(sets.find(2), None);
        assert_eq!(sets.union(0, 2), None);
        assert_eq!(sets.union(3, 0), None);
        assert_eq!(sets.connected(1, 2), None);
        assert_eq!(sets.connected(usize::MAX, 0), None);
        assert_eq!(sets.component_count(), 2);

        let mut empty = DisjointSet::new(0);
        assert_eq!(empty.find(0), None);
        assert_eq!(empty.union(0, 0), None);
        assert_eq!(empty.connected(0, 0), None);
    }

    #[test]
    fn reports_length_and_empty_state() {
        let sets = DisjointSet::new(0);
        assert_eq!(sets.len(), 0);
        assert!(sets.is_empty());
        let sets = DisjointSet::new(2);
        assert_eq!(sets.len(), 2);
        assert!(!sets.is_empty());
    }
}
