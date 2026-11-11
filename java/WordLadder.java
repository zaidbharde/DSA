import java.util.ArrayDeque;
import java.util.ArrayList;
import java.util.Collection;
import java.util.HashMap;
import java.util.HashSet;
import java.util.Map;
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
 * Building the lookup structures takes O(N*L) expected time. Bidirectional BFS
 * expands at most O(N+1) words in total, making at most O((N+1)*L*A)
 * substitution probes. Each probe constructs and hashes a length-L string, so
 * expected runtime is O(N*L + (N+1)*L*L*A). This is the same asymptotic expected
 * worst-case bound as one-direction BFS; bidirectional search can reduce the
 * number of explored words substantially in practice but does not improve the
 * asymptotic worst case. With adversarial hash collisions, a hash-table lookup
 * can take O(N) comparisons of O(L) strings, yielding a conservative
 * O(N*N*L + (N+1)*N*L*L*A) worst-case time bound, including lookup-structure
 * construction. Auxiliary memory is O(N+A+L) for the word set, distance maps,
 * frontiers, alphabet, and temporary buffer.</p>
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

        Set<String> beginFrontier = new HashSet<>();
        Set<String> endFrontier = new HashSet<>();
        Map<String, Integer> beginDistances = new HashMap<>();
        Map<String, Integer> endDistances = new HashMap<>();
        beginFrontier.add(beginWord);
        endFrontier.add(endWord);
        beginDistances.put(beginWord, 0);
        endDistances.put(endWord, 0);
        char[] candidate = new char[wordLength];

        while (!beginFrontier.isEmpty() && !endFrontier.isEmpty()) {
            // Expand one complete BFS layer from the side with fewer words.
            Set<String> frontier;
            Map<String, Integer> distances;
            Map<String, Integer> oppositeDistances;
            boolean expandingBeginSide;
            if (beginFrontier.size() <= endFrontier.size()) {
                frontier = beginFrontier;
                distances = beginDistances;
                oppositeDistances = endDistances;
                expandingBeginSide = true;
            } else {
                frontier = endFrontier;
                distances = endDistances;
                oppositeDistances = beginDistances;
                expandingBeginSide = false;
            }

            Set<String> nextFrontier = new HashSet<>();
            for (String currentWord : frontier) {
                currentWord.getChars(0, wordLength, candidate, 0);
                int nextDistance = distances.get(currentWord) + 1;
                for (int position = 0; position < wordLength; position++) {
                    char original = candidate[position];
                    for (char replacement : alphabet) {
                        if (replacement == original) {
                            continue;
                        }
                        candidate[position] = replacement;
                        String neighbor = new String(candidate);
                        if (!words.contains(neighbor)) {
                            continue;
                        }

                        Integer oppositeDistance = oppositeDistances.get(neighbor);
                        if (oppositeDistance != null) {
                            return nextDistance + oppositeDistance + 1;
                        }
                        if (!distances.containsKey(neighbor)) {
                            distances.put(neighbor, nextDistance);
                            nextFrontier.add(neighbor);
                        }
                    }
                    candidate[position] = original;
                }
            }

            if (expandingBeginSide) {
                beginFrontier = nextFrontier;
            } else {
                endFrontier = nextFrontier;
            }
        }
        return 0;
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

    /** Simple one-direction BFS used only as an independent test oracle. */
    private static int referenceBfs(String beginWord, String endWord,
                                    Collection<String> dictionary) {
        if (beginWord.equals(endWord)) {
            return 1;
        }
        Set<String> words = new HashSet<>(dictionary);
        if (!words.contains(endWord)) {
            return 0;
        }
        Queue<String> queue = new ArrayDeque<>();
        Map<String, Integer> distances = new HashMap<>();
        queue.add(beginWord);
        distances.put(beginWord, 0);
        while (!queue.isEmpty()) {
            String current = queue.remove();
            int nextDistance = distances.get(current) + 1;
            for (String word : words) {
                if (distances.containsKey(word) || !differsByOneCharacter(current, word)) {
                    continue;
                }
                if (word.equals(endWord)) {
                    return nextDistance + 1;
                }
                distances.put(word, nextDistance);
                queue.add(word);
            }
        }
        return 0;
    }

    private static boolean differsByOneCharacter(String first, String second) {
        int differences = 0;
        for (int index = 0; index < first.length(); index++) {
            if (first.charAt(index) != second.charAt(index) && ++differences > 1) {
                return false;
            }
        }
        return differences == 1;
    }

    private static void compareAgainstReferenceForSmallDictionaries() {
        ArrayList<String> universe = new ArrayList<>();
        char[] alphabet = {'a', 'b', 'c'};
        for (char first : alphabet) {
            for (char second : alphabet) {
                universe.add(new String(new char[]{first, second}));
            }
        }

        // Exhaust every dictionary over this small universe and every endpoint pair.
        for (int mask = 0; mask < (1 << universe.size()); mask++) {
            ArrayList<String> dictionary = new ArrayList<>();
            for (int index = 0; index < universe.size(); index++) {
                if ((mask & (1 << index)) != 0) {
                    dictionary.add(universe.get(index));
                }
            }
            for (String beginWord : universe) {
                for (String endWord : universe) {
                    int expected = referenceBfs(beginWord, endWord, dictionary);
                    int actual = shortestTransformationLength(beginWord, endWord, dictionary);
                    assert actual == expected : "Mismatch for begin=" + beginWord
                            + ", end=" + endWord + ", dictionary=" + dictionary
                            + ": expected " + expected + " but got " + actual;
                }
            }
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

        compareAgainstReferenceForSmallDictionaries();
        System.out.println("WordLadder assertions passed.");
    }
}
