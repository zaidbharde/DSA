public class BinarySearch {
    /**
     * Searches an ascending-sorted array and returns the first index containing
     * {@code target}, or -1 if the target is not present.
     *
     * @param arr ascending-sorted array to search
     * @param target value to find
     * @return first matching index, or -1 when not found
     */
    public static int search(int[] arr, int target) {
        int lo = 0;
        int hi = arr.length; // Exclusive upper bound.
        while (lo < hi) {
            // lo and hi are nonnegative valid array bounds; this avoids overflow.
            int mid = lo + ((hi - lo) >>> 1);
            if (arr[mid] < target) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        return lo < arr.length && arr[lo] == target ? lo : -1;
    }

    public static void main(String[] args) {
        boolean assertionsEnabled = false;
        assert assertionsEnabled = true;
        if (!assertionsEnabled) {
            throw new IllegalStateException("Run with -ea to execute BinarySearch tests");
        }

        assert search(new int[0], 10) == -1;
        assert search(new int[] {7}, 7) == 0;
        assert search(new int[] {7}, 6) == -1;

        int[] ordered = {1, 3, 5, 7, 9};
        assert search(ordered, 1) == 0; // First
        assert search(ordered, 5) == 2; // Middle
        assert search(ordered, 9) == 4; // Last
        assert search(ordered, 0) == -1; // Below range
        assert search(ordered, 4) == -1; // Between values
        assert search(ordered, 10) == -1; // Above range

        assert search(new int[] {1, 2, 2, 2, 3}, 2) == 1;
        assert search(new int[] {Integer.MIN_VALUE, Integer.MIN_VALUE, 0,
                Integer.MAX_VALUE, Integer.MAX_VALUE}, Integer.MIN_VALUE) == 0;
        assert search(new int[] {Integer.MIN_VALUE, Integer.MIN_VALUE, 0,
                Integer.MAX_VALUE, Integer.MAX_VALUE}, Integer.MAX_VALUE) == 3;
        assert search(new int[] {Integer.MIN_VALUE, Integer.MAX_VALUE}, 0) == -1;

        int[] arr = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
        System.out.println(search(arr, 23));
    }
}
