//! Maximum XOR of any pair of unsigned 32-bit integers.
//!
//! Returns `None` when fewer than two values are provided. The input is read-only.
//! The trie processes 32 bits per input value, giving O(32n) time and O(32n)
//! auxiliary space for `n` values (equivalently O(n) for fixed-width `u32`).

/// Returns the largest XOR of any pair of values, or `None` if no pair exists.
///
/// Each value is queried against values inserted earlier, then inserted itself,
/// so a value is never paired with itself by position. The input slice is not
/// modified. Runs in O(32n) time and uses O(32n) auxiliary space for `n` values.
pub fn maximum_xor_pair(values: &[u32]) -> Option<u32> {
    let (&first, remaining) = values.split_first()?;
    if remaining.is_empty() {
        return None;
    }

    let mut trie = vec![TrieNode::default()];
    insert(&mut trie, first);
    let mut maximum = 0;

    for &value in remaining {
        maximum = maximum.max(largest_xor_with(&trie, value));
        insert(&mut trie, value);
    }

    Some(maximum)
}

#[derive(Default)]
struct TrieNode {
    children: [Option<usize>; 2],
}

fn insert(trie: &mut Vec<TrieNode>, value: u32) {
    let mut node_index = 0;
    for bit in (0..32).rev() {
        let branch = ((value >> bit) & 1) as usize;
        let child_index = match trie[node_index].children[branch] {
            Some(index) => index,
            None => {
                let index = trie.len();
                trie.push(TrieNode::default());
                trie[node_index].children[branch] = Some(index);
                index
            }
        };
        node_index = child_index;
    }
}

fn largest_xor_with(trie: &[TrieNode], value: u32) -> u32 {
    let mut node_index = 0;
    let mut result = 0;

    for bit in (0..32).rev() {
        let value_bit = ((value >> bit) & 1) as usize;
        let opposite_branch = value_bit ^ 1;
        let chosen_branch = if trie[node_index].children[opposite_branch].is_some() {
            opposite_branch
        } else {
            value_bit
        };
        if chosen_branch != value_bit {
            result |= 1 << bit;
        }
        node_index = trie[node_index].children[chosen_branch]
            .expect("the trie contains a path for every inserted value");
    }

    result
}

#[cfg(test)]
mod tests {
    use super::maximum_xor_pair;

    fn brute_force(values: &[u32]) -> Option<u32> {
        let mut maximum = None;
        for first in 0..values.len() {
            for second in first + 1..values.len() {
                let candidate = values[first] ^ values[second];
                maximum = Some(maximum.map_or(candidate, |best: u32| best.max(candidate)));
            }
        }
        maximum
    }

    #[test]
    fn standard_example() {
        assert_eq!(maximum_xor_pair(&[3, 10, 5, 25, 2, 8]), Some(28));
    }

    #[test]
    fn duplicate_values_and_zeros() {
        assert_eq!(maximum_xor_pair(&[7, 7]), Some(0));
        assert_eq!(maximum_xor_pair(&[0, 0, 0]), Some(0));
    }

    #[test]
    fn supports_unsigned_high_bits() {
        assert_eq!(maximum_xor_pair(&[0, u32::MAX]), Some(u32::MAX));
        assert_eq!(maximum_xor_pair(&[0, 0x8000_0000]), Some(0x8000_0000));
        let values = [u32::MAX, 0x7fff_ffff, 0x8000_0000];
        assert_eq!(maximum_xor_pair(&values), brute_force(&values));
    }

    #[test]
    fn returns_none_when_there_are_fewer_than_two_values() {
        assert_eq!(maximum_xor_pair(&[]), None);
        assert_eq!(maximum_xor_pair(&[42]), None);
    }

    #[test]
    fn deterministic_inputs_match_brute_force() {
        let mut state = 0x1234_5678_u32;
        for sample in 0..40 {
            let length = 2 + sample % 12;
            let mut values = Vec::with_capacity(length);
            for _ in 0..length {
                state = state.wrapping_mul(1_664_525).wrapping_add(1_013_904_223);
                values.push(state);
            }
            assert_eq!(
                maximum_xor_pair(&values),
                brute_force(&values),
                "deterministic brute-force comparison {sample}"
            );
        }
    }
}
