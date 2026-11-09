//! Least-recently-used cache backed by a hash map and a safe doubly linked list.
//!
//! `get` and `put` have expected O(1) time complexity; storage is O(capacity).
//! A successful `get` or any `put` (including an update) makes the key most
//! recently used. `get` borrows its key and returns a cloned value, leaving
//! ownership of both the key and returned value with the caller. `put` takes
//! ownership of its key and value. A cache with capacity zero stores nothing.

use std::cell::RefCell;
use std::collections::HashMap;
use std::hash::Hash;
use std::rc::{Rc, Weak};

type Link<K, V> = Rc<RefCell<Node<K, V>>>;

struct Node<K, V> {
    key: Option<K>,
    value: Option<V>,
    prev: Weak<RefCell<Node<K, V>>>,
    next: Option<Link<K, V>>,
}

impl<K, V> Node<K, V> {
    fn sentinel() -> Self {
        Self {
            key: None,
            value: None,
            prev: Weak::new(),
            next: None,
        }
    }

    fn entry(key: K, value: V) -> Self {
        Self {
            key: Some(key),
            value: Some(value),
            prev: Weak::new(),
            next: None,
        }
    }
}

/// A least-recently-used cache.
///
/// The list runs from the least recently used node immediately after `head`
/// to the most recently used node immediately before `tail`. The list uses
/// strong forward links and weak backward links, so it does not form an
/// ownership cycle.
pub struct LruCache<K, V> {
    capacity: usize,
    entries: HashMap<K, Link<K, V>>,
    head: Link<K, V>,
    tail: Link<K, V>,
}

impl<K, V> LruCache<K, V>
where
    K: Eq + Hash + Clone,
{
    /// Create a cache that retains at most `capacity` entries.
    pub fn new(capacity: usize) -> Self {
        let head = Rc::new(RefCell::new(Node::sentinel()));
        let tail = Rc::new(RefCell::new(Node::sentinel()));
        head.borrow_mut().next = Some(Rc::clone(&tail));
        tail.borrow_mut().prev = Rc::downgrade(&head);

        Self {
            capacity,
            entries: HashMap::with_capacity(capacity),
            head,
            tail,
        }
    }

    /// Return the value for `key`, cloned, and mark the entry most recent.
    ///
    /// Returns `None` when the key is absent. The clone keeps ownership
    /// semantics uncomplicated while the cache remains internally mutable.
    pub fn get(&mut self, key: &K) -> Option<V>
    where
        V: Clone,
    {
        let node = Rc::clone(self.entries.get(key)?);
        self.move_to_most_recent(&node);
        let value = node.borrow().value.as_ref().cloned();
        value
    }

    /// Insert or update an entry, evicting the least-recently-used key if full.
    ///
    /// Updating an existing key replaces its value and promotes it to most
    /// recent. In a zero-capacity cache, the input is simply discarded.
    pub fn put(&mut self, key: K, value: V) {
        if self.capacity == 0 {
            return;
        }

        if let Some(node) = self.entries.get(&key).cloned() {
            node.borrow_mut().value = Some(value);
            self.move_to_most_recent(&node);
            return;
        }

        if self.entries.len() == self.capacity {
            let least_recent = {
                let head = self.head.borrow();
                Rc::clone(
                    head.next
                        .as_ref()
                        .expect("head sentinel must link to tail"),
                )
            };
            let evicted_key = least_recent
                .borrow()
                .key
                .as_ref()
                .expect("nonempty cache must have an entry after head")
                .clone();
            self.unlink(&least_recent);
            self.entries.remove(&evicted_key);
        }

        let node = Rc::new(RefCell::new(Node::entry(key.clone(), value)));
        self.insert_before_tail(&node);
        self.entries.insert(key, node);
    }

    /// Number of entries currently stored.
    pub fn len(&self) -> usize {
        self.entries.len()
    }

    /// Whether the cache currently has no entries.
    pub fn is_empty(&self) -> bool {
        self.entries.is_empty()
    }

    /// Maximum number of entries retained by this cache.
    pub fn capacity(&self) -> usize {
        self.capacity
    }

    fn move_to_most_recent(&self, node: &Link<K, V>) {
        let already_most_recent = node
            .borrow()
            .next
            .as_ref()
            .is_some_and(|next| Rc::ptr_eq(next, &self.tail));
        if already_most_recent {
            return;
        }
        self.unlink(node);
        self.insert_before_tail(node);
    }

    fn unlink(&self, node: &Link<K, V>) {
        let (previous, next) = {
            let borrowed = node.borrow();
            (
                borrowed
                    .prev
                    .upgrade()
                    .expect("linked node must have a live predecessor"),
                Rc::clone(
                    borrowed
                        .next
                        .as_ref()
                        .expect("linked node must have a successor"),
                ),
            )
        };
        previous.borrow_mut().next = Some(Rc::clone(&next));
        next.borrow_mut().prev = Rc::downgrade(&previous);
        let mut borrowed = node.borrow_mut();
        borrowed.prev = Weak::new();
        borrowed.next = None;
    }

    fn insert_before_tail(&self, node: &Link<K, V>) {
        let previous = self
            .tail
            .borrow()
            .prev
            .upgrade()
            .expect("tail sentinel must have a live predecessor");
        {
            let mut borrowed = node.borrow_mut();
            borrowed.prev = Rc::downgrade(&previous);
            borrowed.next = Some(Rc::clone(&self.tail));
        }
        previous.borrow_mut().next = Some(Rc::clone(node));
        self.tail.borrow_mut().prev = Rc::downgrade(node);
    }
}

#[cfg(test)]
mod tests {
    use super::LruCache;

    fn assert_consistent<K, V>(cache: &LruCache<K, V>)
    where
        K: Eq + std::hash::Hash + Clone + std::fmt::Debug,
    {
        assert!(cache.entries.len() <= cache.capacity);
        let mut previous = std::rc::Rc::clone(&cache.head);
        let mut current = cache
            .head
            .borrow()
            .next
            .as_ref()
            .expect("head must have a next link")
            .clone();
        let mut visited = 0;

        while !std::rc::Rc::ptr_eq(&current, &cache.tail) {
            assert!(visited < cache.entries.len(), "list contains a cycle");
            assert!(std::rc::Rc::ptr_eq(
                &current
                    .borrow()
                    .prev
                    .upgrade()
                    .expect("entry must have a live predecessor"),
                &previous
            ));
            let key = current
                .borrow()
                .key
                .as_ref()
                .expect("list entry must have a key")
                .clone();
            let mapped = cache.entries.get(&key).expect("entry missing from map");
            assert!(std::rc::Rc::ptr_eq(mapped, &current));
            previous = std::rc::Rc::clone(&current);
            let next = current
                .borrow()
                .next
                .as_ref()
                .expect("entry must have a successor")
                .clone();
            current = next;
            visited += 1;
        }

        assert_eq!(visited, cache.entries.len());
        assert!(std::rc::Rc::ptr_eq(
            &cache
                .tail
                .borrow()
                .prev
                .upgrade()
                .expect("tail must have a live predecessor"),
            &previous
        ));
    }

    #[test]
    fn get_promotes_entry_and_eviction_uses_recency() {
        let mut cache = LruCache::new(2);
        cache.put("a", 1);
        cache.put("b", 2);
        assert_eq!(cache.get(&"a"), Some(1));
        cache.put("c", 3);
        assert_eq!(cache.get(&"b"), None);
        assert_eq!(cache.get(&"a"), Some(1));
        assert_eq!(cache.get(&"c"), Some(3));
        assert_consistent(&cache);
    }

    #[test]
    fn update_replaces_value_and_promotes_key() {
        let mut cache = LruCache::new(2);
        cache.put(1, String::from("old"));
        cache.put(2, String::from("two"));
        cache.put(1, String::from("new"));
        cache.put(3, String::from("three"));
        assert_eq!(cache.get(&1).as_deref(), Some("new"));
        assert_eq!(cache.get(&2), None);
        assert_eq!(cache.get(&3).as_deref(), Some("three"));
        assert_eq!(cache.len(), 2);
        assert_consistent(&cache);
    }

    #[test]
    fn capacity_zero_discards_and_capacity_one_keeps_only_latest() {
        let mut zero = LruCache::new(0);
        zero.put("ignored", 9);
        assert_eq!(zero.get(&"ignored"), None);
        assert!(zero.is_empty());
        assert_consistent(&zero);

        let mut one = LruCache::new(1);
        one.put("first", 1);
        one.put("second", 2);
        assert_eq!(one.get(&"first"), None);
        assert_eq!(one.get(&"second"), Some(2));
        assert_eq!(one.capacity(), 1);
        assert_consistent(&one);
    }

    #[test]
    fn keys_and_values_are_generic_and_get_returns_a_clone() {
        let first_key = String::from("first");
        let mut cache = LruCache::new(2);
        cache.put(first_key.clone(), vec![1, 2]);
        cache.put(String::from("second"), vec![3]);

        let retrieved = cache.get(&first_key).expect("key should be present");
        assert_eq!(retrieved, vec![1, 2]);
        assert_eq!(first_key, "first");
        assert_consistent(&cache);
    }

    #[test]
    fn deterministic_operation_trace_preserves_list_map_invariants() {
        let mut cache = LruCache::new(3);
        let operations = [
            (true, 1, 10),
            (true, 2, 20),
            (true, 3, 30),
            (false, 1, 0),
            (true, 4, 40),
            (false, 2, 0),
            (true, 3, 31),
            (true, 5, 50),
            (false, 4, 0),
            (false, 3, 0),
        ];

        for (is_put, key, value) in operations {
            if is_put {
                cache.put(key, value);
            } else {
                let _ = cache.get(&key);
            }
            assert_consistent(&cache);
        }

        assert_eq!(cache.get(&2), None);
        assert_eq!(cache.get(&3), Some(31));
        assert_eq!(cache.get(&4), Some(40));
        assert_eq!(cache.get(&5), Some(50));
    }
}
