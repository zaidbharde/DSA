import java.util.Arrays;

/**
 * Suffix-array and LCP construction for Java strings.
 *
 * <p>Characters are compared as unsigned UTF-16 code units (the values returned
 * by {@link String#charAt(int)}), not as Unicode code points or locale-aware
 * text. The LCP array uses the convention {@code lcp[r]} = the common-prefix
 * length of suffix-array entries {@code r - 1} and {@code r}, with
 * {@code lcp[0] == 0}.</p>
 *
 * <p>Null arguments and malformed suffix arrays are rejected with
 * {@link IllegalArgumentException}. Empty strings are valid and produce empty
 * arrays.</p>
 */
public final class SuffixArray {
    private SuffixArray() {
        // Utility class.
    }

    /**
     * Constructs the suffix array by prefix doubling and comparison sorting.
     *
     * <p>Runs in O(n log^2 n) time and uses O(n) auxiliary storage, where n is
     * {@code text.length()} in UTF-16 code units.</p>
     *
     * @param text input text; may be empty but not null
     * @return starting offsets of all suffixes, in lexicographic UTF-16
     *         code-unit order
     * @throws IllegalArgumentException if {@code text} is null
     */
    public static int[] suffixArray(String text) {
        if (text == null) {
            throw new IllegalArgumentException("text must not be null");
        }

        final int n = text.length();
        Integer[] order = new Integer[n];
        if (n == 0) {
            return new int[0];
        }

        int[] rank = new int[n];
        for (int i = 0; i < n; i++) {
            order[i] = i;
            rank[i] = text.charAt(i);
        }

        // Force one sorting pass to establish correct ordered equivalence
        // classes, including when equal code units are nonadjacent in text.
        int classes = 0;

        for (int width = 1; classes < n; width = width > n / 2 ? n : width * 2) {
            final int[] currentRank = rank;
            final int currentWidth = width;
            Arrays.sort(order, (left, right) -> {
                int firstComparison = Integer.compare(currentRank[left], currentRank[right]);
                if (firstComparison != 0) {
                    return firstComparison;
                }
                int leftSecond = secondRank(left, currentWidth, currentRank, n);
                int rightSecond = secondRank(right, currentWidth, currentRank, n);
                return Integer.compare(leftSecond, rightSecond);
            });

            int[] nextRank = new int[n];
            int nextClass = 0;
            nextRank[order[0]] = 0;
            for (int i = 1; i < n; i++) {
                int previous = order[i - 1];
                int current = order[i];
                if (currentRank[previous] != currentRank[current]
                        || secondRank(previous, currentWidth, currentRank, n)
                        != secondRank(current, currentWidth, currentRank, n)) {
                    nextClass++;
                }
                nextRank[current] = nextClass;
            }
            rank = nextRank;
            classes = nextClass + 1;
        }

        int[] suffixes = new int[n];
        for (int i = 0; i < n; i++) {
            suffixes[i] = order[i];
        }
        return suffixes;
    }

    /**
     * Constructs the Kasai LCP array in O(n) time and O(n) auxiliary storage.
     *
     * @param text input text; may be empty but not null
     * @param suffixes suffix array for {@code text}, containing every offset
     *                 from 0 through {@code text.length() - 1} exactly once
     * @return LCP values with {@code lcp[0] == 0}; LCP lengths count UTF-16
     *         code units
     * @throws IllegalArgumentException if an argument is null or
     *                                  {@code suffixes} is not a permutation
     *                                  of the valid suffix offsets
     */
    public static int[] lcpArray(String text, int[] suffixes) {
        if (text == null) {
            throw new IllegalArgumentException("text must not be null");
        }
        if (suffixes == null) {
            throw new IllegalArgumentException("suffixes must not be null");
        }

        final int n = text.length();
        if (suffixes.length != n) {
            throw new IllegalArgumentException("suffix array length must equal text length");
        }

        int[] inverse = new int[n];
        boolean[] seen = new boolean[n];
        for (int r = 0; r < n; r++) {
            int start = suffixes[r];
            if (start < 0 || start >= n || seen[start]) {
                throw new IllegalArgumentException("suffix array must be a permutation of valid offsets");
            }
            seen[start] = true;
            inverse[start] = r;
        }

        int[] lcp = new int[n];
        int common = 0;
        for (int start = 0; start < n; start++) {
            int r = inverse[start];
            if (r == 0) {
                common = 0;
                continue;
            }

            int previousStart = suffixes[r - 1];
            while (start + common < n && previousStart + common < n
                    && text.charAt(start + common) == text.charAt(previousStart + common)) {
                common++;
            }
            lcp[r] = common;
            if (common > 0) {
                common--;
            }
        }
        return lcp;
    }

    /** Returns the rank of the second half, with -1 as the end-of-text sentinel. */
    private static int secondRank(int suffixStart, int width, int[] rank, int n) {
        return suffixStart < n - width ? rank[suffixStart + width] : -1;
    }

    /** Assertion-enabled standalone tests; run with {@code java -ea SuffixArray}. */
    public static void main(String[] args) {
        boolean assertionsEnabled = false;
        assert assertionsEnabled = true;
        if (!assertionsEnabled) {
            throw new IllegalStateException("Tests require assertions; run with java -ea SuffixArray");
        }

        checkCase("", new int[] {}, new int[] {});
        checkCase("x", new int[] {0}, new int[] {0});
        checkCase("banana", new int[] {5, 3, 1, 0, 4, 2}, new int[] {0, 1, 3, 0, 0, 2});
        checkCase("aaaa", new int[] {3, 2, 1, 0}, new int[] {0, 1, 2, 3});
        // Deliberately includes a surrogate pair; ordering and LCP are by char units.
        checkCase("\uD83D\uDE00\uE000\uD83D",
                new int[] {3, 0, 1, 2}, new int[] {0, 1, 0, 0});

        checkInvalidArguments();
        System.out.println("SuffixArray tests passed.");
    }

    private static void checkCase(String text, int[] expectedSuffixes, int[] expectedLcp) {
        int[] suffixes = suffixArray(text);
        int[] lcp = lcpArray(text, suffixes);
        assert Arrays.equals(suffixes, expectedSuffixes)
                : "suffix array mismatch for " + printable(text) + ": " + Arrays.toString(suffixes);
        assert Arrays.equals(lcp, expectedLcp)
                : "LCP mismatch for " + printable(text) + ": " + Arrays.toString(lcp);
        assertSuffixOrdering(text, suffixes);
        assertLcpValues(text, suffixes, lcp);
    }

    private static void assertSuffixOrdering(String text, int[] suffixes) {
        for (int i = 1; i < suffixes.length; i++) {
            int left = suffixes[i - 1];
            int right = suffixes[i];
            int limit = Math.min(text.length() - left, text.length() - right);
            int common = 0;
            while (common < limit && text.charAt(left + common) == text.charAt(right + common)) {
                common++;
            }
            assert common == limit ? text.length() - left < text.length() - right
                    : text.charAt(left + common) < text.charAt(right + common)
                    : "suffixes out of order at rank " + i + " for " + printable(text);
        }
    }

    private static void assertLcpValues(String text, int[] suffixes, int[] lcp) {
        assert lcp.length == suffixes.length;
        if (lcp.length > 0) {
            assert lcp[0] == 0;
        }
        for (int i = 1; i < suffixes.length; i++) {
            int left = suffixes[i - 1];
            int right = suffixes[i];
            int common = 0;
            while (left + common < text.length() && right + common < text.length()
                    && text.charAt(left + common) == text.charAt(right + common)) {
                common++;
            }
            assert lcp[i] == common : "incorrect LCP at rank " + i;
        }
    }

    private static void checkInvalidArguments() {
        boolean rejected = false;
        try {
            suffixArray(null);
        } catch (IllegalArgumentException expected) {
            rejected = true;
        }
        assert rejected : "null text must be rejected";

        rejected = false;
        try {
            lcpArray("a", new int[] {1});
        } catch (IllegalArgumentException expected) {
            rejected = true;
        }
        assert rejected : "out-of-range suffix offset must be rejected";
    }

    private static String printable(String text) {
        return Arrays.toString(text.toCharArray());
    }
}
