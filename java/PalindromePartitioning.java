/**
 * Computes the minimum number of cuts needed to partition a string into
 * palindromic substrings. Palindromes are defined over Java UTF-16 code units
 * (the values returned by {@link String#charAt(int)}), not Unicode code points
 * or grapheme clusters. The input is not modified.
 *
 * <p>For a string of n UTF-16 code units, the dynamic program takes O(n^2) time
 * and O(n^2) auxiliary space for the palindrome table, plus O(n) for the cut
 * counts. This bound includes preprocessing and the optimization pass.</p>
 */
public final class PalindromePartitioning {
    private PalindromePartitioning() {
        // Utility class.
    }

    /**
     * Returns the minimum cuts required to split {@code text} into palindromes.
     * The empty string needs zero cuts, and any one-code-unit string needs zero.
     *
     * @param text string to partition
     * @return minimum number of cuts; zero for an empty string
     * @throws IllegalArgumentException if text is null
     */
    public static int minimumCuts(String text) {
        if (text == null) {
            throw new IllegalArgumentException("text must not be null");
        }

        int length = text.length();
        if (length < 2) {
            return 0;
        }

        // palindrome[start][end] records whether the inclusive UTF-16 range is
        // palindromic. Descending start order makes the inner range available.
        boolean[][] palindrome = new boolean[length][length];
        for (int start = length - 1; start >= 0; start--) {
            for (int end = start; end < length; end++) {
                palindrome[start][end] = text.charAt(start) == text.charAt(end)
                        && (end - start < 2 || palindrome[start + 1][end - 1]);
            }
        }

        // cuts[prefixLength] is the fewest cuts for text[0..prefixLength).
        // The -1 empty-prefix base makes a palindrome from index 0 cost zero.
        int[] cuts = new int[length + 1];
        cuts[0] = -1;
        for (int end = 1; end <= length; end++) {
            cuts[end] = end - 1;
            for (int start = 0; start < end; start++) {
                if (palindrome[start][end - 1]) {
                    cuts[end] = Math.min(cuts[end], cuts[start] + 1);
                }
            }
        }
        return cuts[length];
    }

    private static void expectIllegalArgument(Runnable action, String message) {
        boolean rejected = false;
        try {
            action.run();
        } catch (IllegalArgumentException expected) {
            rejected = true;
        }
        assert rejected : message;
    }

    /** Exhaustive reference implementation for short strings. */
    private static int bruteForceCuts(String text, int start) {
        if (start == text.length()) {
            return -1;
        }
        int best = text.length() - start - 1;
        for (int end = start; end < text.length(); end++) {
            if (isPalindrome(text, start, end)) {
                best = Math.min(best, 1 + bruteForceCuts(text, end + 1));
            }
        }
        return best;
    }

    private static boolean isPalindrome(String text, int left, int right) {
        while (left < right) {
            if (text.charAt(left) != text.charAt(right)) {
                return false;
            }
            left++;
            right--;
        }
        return true;
    }

    private static void compareAllBinaryStrings(int length, String prefix) {
        if (prefix.length() == length) {
            int reference = prefix.isEmpty() ? 0 : bruteForceCuts(prefix, 0);
            assert minimumCuts(prefix) == reference
                    : "dynamic program disagrees with brute force for " + prefix;
            return;
        }
        compareAllBinaryStrings(length, prefix + 'a');
        compareAllBinaryStrings(length, prefix + 'b');
    }

    /** Assertion-based self-tests; run with {@code java -ea}. */
    public static void main(String[] args) {
        assert minimumCuts("") == 0 : "empty string needs no cuts";
        assert minimumCuts("x") == 0 : "single UTF-16 code unit needs no cuts";
        assert minimumCuts("aab") == 1 : "classic minimum-cut example";
        assert minimumCuts("ababbbabbababa") == 3 : "classic longer example";
        assert minimumCuts("racecar") == 0 : "entire string is a palindrome";
        assert minimumCuts("aaaaaa") == 0 : "all repeated characters are a palindrome";
        assert minimumCuts("abcde") == 4 : "no distinct adjacent code units form palindromes";

        // A supplementary Unicode code point occupies two different UTF-16
        // code units, so the string is not one palindrome under this API's rule.
        String supplementary = "\uD83D\uDE00";
        assert supplementary.length() == 2;
        assert minimumCuts(supplementary) == 1 : "UTF-16 code-unit behavior";
        assert minimumCuts(supplementary + supplementary) == 1
                : "overlapping palindromes are measured in UTF-16 code units";

        for (int length = 0; length <= 8; length++) {
            compareAllBinaryStrings(length, "");
        }
        expectIllegalArgument(() -> minimumCuts(null), "null input rejected");
        System.out.println("PalindromePartitioning assertions passed.");
    }
}
