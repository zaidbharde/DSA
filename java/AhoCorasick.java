import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Objects;

/**
 * Multi-pattern matching over Java strings (UTF-16 code units).
 *
 * <p>Patterns must be non-null and non-empty. Pattern indexes are their positions in the
 * constructor's list; duplicates are retained as distinct indexes. Match intervals are
 * half-open UTF-16 offsets {@code [start, end)}.</p>
 *
 * <p>Let P be the sum of pattern lengths in UTF-16 code units, L the maximum pattern
 * length, and S the number of trie states (S <= P + 1). Trie insertion takes O(P)
 * expected time. This sparse-map failure/output-link builder may follow up to L suffix
 * links for each trie edge, so its conservative construction bound is O(P * L) expected
 * time, assuming expected O(1) {@link HashMap} operations. Scanning takes O(n + z) expected
 * time for a text of n code units and z matches:
 * failure transitions are amortized linear in n, and each reported match is emitted once.
 * Storage is O(P + S) expected: trie nodes, sparse maps, pattern IDs, and output links;
 * output links share suffix outputs rather than copying them into every state. These
 * bounds assume ordinary expected-time hash operations; adversarial hash behavior can
 * increase map-operation costs.</p>
 */
public final class AhoCorasick {
    /** A match of pattern {@code patternIndex} over the half-open UTF-16 range [start, end). */
    public record Match(int patternIndex, int start, int end) { }

    private static final class Node {
        private final Map<Character, Node> next = new HashMap<>();
        private Node failure;
        /** Nearest terminal state strictly on this state's failure chain, or null. */
        private Node outputLink;
        /** Pattern indexes ending exactly at this state; duplicates have distinct indexes. */
        private final List<Integer> patternIndexes = new ArrayList<>();
    }

    private final Node root = new Node();
    private final List<String> patterns;

    /** Builds an automaton from patterns in index order. */
    public AhoCorasick(List<String> patterns) {
        Objects.requireNonNull(patterns, "patterns");
        List<String> copy = new ArrayList<>(patterns.size());
        for (int i = 0; i < patterns.size(); i++) {
            String pattern = Objects.requireNonNull(patterns.get(i), "pattern at index " + i);
            if (pattern.isEmpty()) {
                throw new IllegalArgumentException("empty patterns are not supported (index " + i + ")");
            }
            copy.add(pattern);
        }
        this.patterns = List.copyOf(copy);
        buildTrie();
        buildFailureAndOutputLinks();
    }

    /** Returns the patterns as an immutable snapshot in their original index order. */
    public List<String> patterns() {
        return patterns;
    }

    /**
     * Returns all matches in text scan order. Matches ending at the same offset are
     * reported longest/current-state outputs first, followed by terminal suffixes.
     * The input strings and this automaton are not modified.
     */
    public List<Match> findAll(String text) {
        Objects.requireNonNull(text, "text");
        List<Match> matches = new ArrayList<>();
        Node state = root;
        for (int offset = 0; offset < text.length(); offset++) {
            char ch = text.charAt(offset);
            Node next = state.next.get(ch);
            while (next == null && state != root) {
                state = state.failure;
                next = state.next.get(ch);
            }
            state = (next == null) ? root : next;
            emitMatches(state, offset + 1, matches);
        }
        return List.copyOf(matches);
    }

    private void buildTrie() {
        for (int patternIndex = 0; patternIndex < patterns.size(); patternIndex++) {
            Node node = root;
            String pattern = patterns.get(patternIndex);
            for (int i = 0; i < pattern.length(); i++) {
                char ch = pattern.charAt(i);
                node = node.next.computeIfAbsent(ch, ignored -> new Node());
            }
            node.patternIndexes.add(patternIndex);
        }
    }

    private void buildFailureAndOutputLinks() {
        root.failure = root;
        ArrayDeque<Node> queue = new ArrayDeque<>();
        for (Node child : root.next.values()) {
            child.failure = root;
            child.outputLink = null;
            queue.addLast(child);
        }

        while (!queue.isEmpty()) {
            Node node = queue.removeFirst();
            for (Map.Entry<Character, Node> edge : node.next.entrySet()) {
                char ch = edge.getKey();
                Node child = edge.getValue();
                Node fallback = node.failure;
                while (fallback != root && !fallback.next.containsKey(ch)) {
                    fallback = fallback.failure;
                }
                Node failureTarget = fallback.next.get(ch);
                child.failure = (failureTarget == null || failureTarget == child)
                        ? root : failureTarget;
                Node failure = child.failure;
                child.outputLink = failure.patternIndexes.isEmpty() ? failure.outputLink : failure;
                queue.addLast(child);
            }
        }
    }

    private void emitMatches(Node state, int end, List<Match> matches) {
        for (Node terminal = state; terminal != null; terminal = terminal.outputLink) {
            for (int patternIndex : terminal.patternIndexes) {
                int start = end - terminalDepthPatternLength(terminal, patternIndex);
                matches.add(new Match(patternIndex, start, end));
            }
        }
    }

    /* Pattern lengths are looked up by ID; keeping this in an instance helper avoids
       storing a length on every terminal state, where several IDs may share a state. */
    private int terminalDepthPatternLength(Node terminal, int patternIndex) {
        return patterns.get(patternIndex).length();
    }

    private static void expectThrows(Class<? extends Throwable> type, Runnable action) {
        boolean thrown = false;
        try {
            action.run();
        } catch (Throwable error) {
            thrown = type.isInstance(error);
        }
        assert thrown : "Expected " + type.getSimpleName();
    }

    private static void runAssertionTests() {
        AhoCorasick suffixes = new AhoCorasick(List.of("he", "she", "his", "hers"));
        assert suffixes.findAll("ushers").equals(List.of(
                new Match(1, 1, 4), new Match(0, 2, 4), new Match(3, 2, 6)));

        AhoCorasick overlaps = new AhoCorasick(List.of("aba", "ba", "a"));
        assert overlaps.findAll("ababa").equals(List.of(
                new Match(2, 0, 1), new Match(0, 0, 3), new Match(1, 1, 3), new Match(2, 2, 3),
                new Match(0, 2, 5), new Match(1, 3, 5), new Match(2, 4, 5)))
                : overlaps.findAll("ababa");

        AhoCorasick duplicates = new AhoCorasick(List.of("cat", "cat"));
        assert duplicates.findAll("cat").equals(List.of(new Match(0, 0, 3), new Match(1, 0, 3)));
        assert duplicates.findAll("dog").isEmpty();
        assert duplicates.findAll("").isEmpty();

        expectThrows(IllegalArgumentException.class, () -> new AhoCorasick(List.of("")));
        expectThrows(NullPointerException.class, () -> new AhoCorasick(null));
        expectThrows(NullPointerException.class, () -> new AhoCorasick(java.util.Arrays.asList("ok", null)));
        expectThrows(NullPointerException.class, () -> duplicates.findAll(null));

        String emoji = "\uD83D\uDE00";
        AhoCorasick unicode = new AhoCorasick(List.of(emoji, "\uDE00"));
        assert unicode.findAll("x" + emoji + "y").equals(List.of(
                new Match(0, 1, 3), new Match(1, 2, 3)));

        List<String> mutablePatterns = new ArrayList<>(List.of("one"));
        AhoCorasick snapshot = new AhoCorasick(mutablePatterns);
        mutablePatterns.set(0, "two");
        assert snapshot.patterns().equals(List.of("one"));
        String unchangedText = new String("one");
        assert snapshot.findAll(unchangedText).equals(List.of(new Match(0, 0, 3)));
        assert unchangedText.equals("one");
        expectThrows(UnsupportedOperationException.class,
                () -> snapshot.findAll("one").add(new Match(0, 0, 3)));
    }

    /** Runs the source-file-launcher assertion tests with {@code java -ea}. */
    public static void main(String[] args) {
        boolean assertionsEnabled = false;
        assert assertionsEnabled = true;
        if (!assertionsEnabled) {
            throw new IllegalStateException("Run tests with assertions enabled: java -ea AhoCorasick.java");
        }
        runAssertionTests();
        System.out.println("AhoCorasick assertion tests passed.");
    }
}
