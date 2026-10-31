/**
 * A prefix tree for storing and querying strings.
 *
 * Complexity (L = input string length):
 * - insert(word): O(L) time; O(L) additional space in the worst case (new nodes).
 * - search(word): O(L) time; O(1) additional space.
 * - startsWith(prefix): O(L) time; O(1) additional space.
 *
 * Across all inserted words, storage is O(S), where S is the number of
 * characters in the trie (equivalently, the number of distinct non-root
 * prefix nodes); duplicate insertions add no nodes.
 */
export class Trie {
  private readonly root: TrieNode = new TrieNode();

  /** Adds a word to the trie. Re-inserting an existing word is harmless. */
  insert(word: string): void {
    let node = this.root;

    for (const character of word) {
      let child = node.children.get(character);
      if (child === undefined) {
        child = new TrieNode();
        node.children.set(character, child);
      }
      node = child;
    }

    node.isWord = true;
  }

  /** Returns true only if the complete word has been inserted. */
  search(word: string): boolean {
    const node = this.findNode(word);
    return node !== undefined && node.isWord;
  }

  /** Returns true if at least one inserted word begins with the prefix. */
  startsWith(prefix: string): boolean {
    return this.findNode(prefix) !== undefined;
  }

  private findNode(text: string): TrieNode | undefined {
    let node = this.root;

    for (const character of text) {
      const child = node.children.get(character);
      if (child === undefined) {
        return undefined;
      }
      node = child;
    }

    return node;
  }
}

class TrieNode {
  readonly children: Map<string, TrieNode> = new Map<string, TrieNode>();
  isWord = false;
}

// Small local assertion helpers keep the self-tests dependency-free.
function assertEqual(actual: boolean, expected: boolean, description: string): void {
  if (actual !== expected) {
    throw new Error(
      `${description}: expected ${String(expected)}, received ${String(actual)}`,
    );
  }
}

function runSelfTests(): void {
  const trie = new Trie();

  trie.insert("apple");
  assertEqual(trie.search("apple"), true, "exact inserted word matches");
  assertEqual(trie.search("app"), false, "word prefix is not an exact match");
  assertEqual(trie.startsWith("app"), true, "inserted prefix matches");
  assertEqual(trie.startsWith("apples"), false, "missing extended prefix fails");
  assertEqual(trie.search("banana"), false, "missing word fails");

  trie.insert("app");
  assertEqual(trie.search("app"), true, "inserted prefix becomes a word");
  trie.insert("apple");
  assertEqual(trie.search("apple"), true, "duplicate insertion preserves word");

  trie.insert("");
  assertEqual(trie.search(""), true, "inserted empty string matches exactly");
  assertEqual(trie.startsWith(""), true, "empty string is a prefix");
  assertEqual(trie.search("a"), false, "empty string does not create another word");

  console.log("Trie self-tests passed.");
}

runSelfTests();
