import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashSet;
import java.util.Queue;
import java.util.Set;

/**
 * Computes the shortest word-transformation length using one-character
 * substitutions, where every intermediate word must be in the dictionary.
 * Words are case-sensitive and are compared as Java UTF-16 strings.
 *
 * <p>The dictionary must be non-null and contain only non-null words whose
 * lengths match the begin word. The end word must be present in the dictionary
 * unless it equals the begin word. Equal begin and end words have length 1;
 * unreachable transformations have length 0. Inputs are not modified.</p>
 *
 * <p>Let N be the number of distinct dictionary words, L the word length, and
 * A the number of distinct UTF-16 characters appearing in the input words.
 * Building the lookup structures takes O(N*L) expected time. BFS makes at
 * most O((N+1)*L*A) substitution probes. In Java, each probe constructs and
 * hashes a length-L string, so the actual expected runtime is
 * O(N*L + (N+1)*L*L*A), or O(N*L*L*A) for nonempty dictionaries. Auxiliary
 * memory is O(N + A + L) for the hash sets, queue, alphabet, and temporary
 * word buffer; input strings are referenced rather than copied.</p>
 */
public final class WordLadder {
    private WordLadder() {
        // Utility class.
    }

    /**
     * Returns the number of words in a shortest one-character-at-a-time
     * transformation from {@code beginWord} to {@code endWord}, including both
     * endpoints.
     *
     * @param beginWord starting word
     * @param endWord target word
     * @param dictionary words allowed as transformation steps; the end word
     *        must be present unless beginWord equals endWord
     * @return shortest sequence length, 1 when the endpoints are equal, or 0
     *         when no transformation exists
     * @throws IllegalArgumentException if an argument is null, endpoint lengths
     *         differ, or the dictionary contains a null or wrong-length word
     */
    public static int shortestTransformationLength(String beginWord, String endWord,
                                                   Collection<String> dictionary) {
        if (beginWord == null) {
            throw new IllegalArgumentException("beginWord must not be null");
        }
        if (endWord == null) {
            throw new IllegalArgumentException("endWord must not be null");
        }
        if (dictionary == null) {
            throw new IllegalArgumentException("dictionary must not be null");
        }
        if (beginWord.length() != endWord.length()) {
            throw new IllegalArgumentException("beginWord and endWord must have equal lengths");
        }

        int wordLength = beginWord.length();
        Set<String> words = new HashSet<>();
        Set<Character> alphabet = new HashSet<>();
        for (int index = 0; index < wordLength; index++) {
            alphabet.add(beginWord.charAt(index));
        }
        for (String word : dictionary) {
            if (word == null) {
                throw new IllegalArgumentException("dictionary must not contain null words");
            }
            if (word.length() != wordLength) {
                throw new IllegalArgumentException("dictionary words must match endpoint length "
                        + wordLength + ": " + word);
            }
            words.add(word);
            for (int index = 0; index < wordLength; index++) {
                alphabet.add(word.charAt(index));
            }
        }

        if (beginWord.equals(endWord)) {
            return 1;
        }
        if (!words.contains(endWord)) {
            return 0;
        }

        Queue<QueueEntry> queue = new ArrayDeque<>();
        Set<String> discovered = new HashSet<>();
        queue.add(new QueueEntry(beginWord, 1));
        discovered.add(beginWord);
        char[] candidate = new char[wordLength];

        while (!queue.isEmpty()) {
            QueueEntry current = queue.remove();
            current.word.getChars(0, wordLength, candidate, 0);
            for (int position = 0; position < wordLength; position++) {
                char original = candidate[position];
                for (char replacement : alphabet) {
                    if (replacement == original) {
                        continue;
                    }
                    candidate[position] = replacement;
                    String neighbor = new String(candidate);
                    if (words.contains(neighbor) && discovered.add(neighbor)) {
                        if (neighbor.equals(endWord)) {
                            return current.distance + 1;
                        }
                        queue.add(new QueueEntry(neighbor, current.distance + 1));
                    }
                }
                candidate[position] = original;
            }
        }
        return 0;
    }

    private static final class QueueEntry {
        private final String word;
        private final int distance;

        private QueueEntry(String word, int distance) {
            this.word = word;
            this.distance = distance;
        }
    }

    private static void expectIllegalArgument(Runnable action, String messagePart) {
        try {
            action.run();
            throw new AssertionError("Expected IllegalArgumentException containing: "
                    + messagePart);
        } catch (IllegalArgumentException exception) {
            assert exception.getMessage().contains(messagePart)
                    : "Unexpected exception message: " + exception.getMessage();
        }
    }

    /** Assertion-enabled self-tests; run with {@code java -ea java/WordLadder.java}. */
    public static void main(String[] args) {
        boolean assertionsEnabled = false;
        assert assertionsEnabled = true;
        if (!assertionsEnabled) {
            throw new IllegalStateException("Run tests with assertions enabled (-ea)");
        }

        ArrayList<String> classic = new ArrayList<>(
                java.util.Arrays.asList("hot", "dot", "dog", "lot", "log", "cog"));
        ArrayList<String> original = new ArrayList<>(classic);
        assert shortestTransformationLength("hit", "cog", classic) == 5;
        assert classic.equals(original) : "The dictionary must not be modified";

        assert shortestTransformationLength("hit", "cog",
                java.util.Arrays.asList("hot", "dot", "dog")) == 0;
        assert shortestTransformationLength("cat", "cot",
                java.util.Arrays.asList("cot")) == 2;
        assert shortestTransformationLength("same", "same",
                java.util.Arrays.asList("same", "word")) == 1;
        assert shortestTransformationLength("a", "c",
                java.util.Arrays.asList("b", "b", "c")) == 2;
        assert shortestTransformationLength("A", "a",
                java.util.Arrays.asList("a")) == 2;
        assert shortestTransformationLength("", "", java.util.Collections.emptyList()) == 1;

        expectIllegalArgument(() -> shortestTransformationLength(null, "a",
                java.util.Collections.emptyList()), "beginWord");
        expectIllegalArgument(() -> shortestTransformationLength("a", null,
                java.util.Collections.emptyList()), "endWord");
        expectIllegalArgument(() -> shortestTransformationLength("a", "a", null),
                "dictionary");
        expectIllegalArgument(() -> shortestTransformationLength("a", "bb",
                java.util.Collections.emptyList()), "equal lengths");
        expectIllegalArgument(() -> shortestTransformationLength("a", "b",
                java.util.Arrays.asList("bb")), "match endpoint length");
        expectIllegalArgument(() -> shortestTransformationLength("a", "b",
                java.util.Arrays.asList((String) null)), "null words");
        expectIllegalArgument(() -> shortestTransformationLength("x", "y",
                java.util.Arrays.asList("bad")), "match endpoint length");

        System.out.println("WordLadder assertions passed.");
    }
}
