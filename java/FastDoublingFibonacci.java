import java.math.BigInteger;
import java.util.Objects;

/**
 * Computes exact Fibonacci numbers using fast doubling.
 *
 * <p>There are O(log n) doubling levels, but this is not O(log n) bit-time:
 * Fibonacci values have Theta(n) bits, so BigInteger operation and output
 * costs grow with the result size. The recursive helper uses O(log n) stack
 * frames. That space bound excludes the BigInteger values and temporary
 * arithmetic results retained or created during computation.</p>
 */
public final class FastDoublingFibonacci {
    private static final BigInteger ZERO = BigInteger.ZERO;
    private static final BigInteger ONE = BigInteger.ONE;
    private static final BigInteger TWO = BigInteger.valueOf(2);

    private FastDoublingFibonacci() {
    }

    /**
     * Returns F(index), with F(0) = 0 and F(1) = 1.
     *
     * @param index a nonnegative arbitrary-precision Fibonacci index
     * @return the exact Fibonacci value
     * @throws NullPointerException if index is null
     * @throws IllegalArgumentException if index is negative
     */
    public static BigInteger fibonacci(BigInteger index) {
        Objects.requireNonNull(index, "index must not be null");
        if (index.signum() < 0) {
            throw new IllegalArgumentException("index must be nonnegative");
        }
        return fibonacciPair(index)[0];
    }

    /** Returns the pair (F(index), F(index + 1)). */
    private static BigInteger[] fibonacciPair(BigInteger index) {
        if (index.signum() == 0) {
            return new BigInteger[] {ZERO, ONE};
        }

        BigInteger[] halfPair = fibonacciPair(index.shiftRight(1));
        BigInteger a = halfPair[0];
        BigInteger b = halfPair[1];
        BigInteger even = a.multiply(b.shiftLeft(1).subtract(a));
        BigInteger odd = a.multiply(a).add(b.multiply(b));
        if (index.testBit(0)) {
            return new BigInteger[] {odd, even.add(odd)};
        }
        return new BigInteger[] {even, odd};
    }

    private static BigInteger iterativeReference(int index) {
        BigInteger previous = ZERO;
        BigInteger current = ONE;
        for (int i = 0; i < index; i++) {
            BigInteger next = previous.add(current);
            previous = current;
            current = next;
        }
        return previous;
    }

    private static void expectNegativeIndexRejected() {
        boolean rejected = false;
        try {
            fibonacci(BigInteger.valueOf(-1));
        } catch (IllegalArgumentException expected) {
            rejected = true;
        }
        assert rejected : "negative index should be rejected";
    }

    public static void main(String[] args) {
        assert fibonacci(ZERO).equals(ZERO);
        assert fibonacci(ONE).equals(ONE);
        assert fibonacci(TWO).equals(BigInteger.valueOf(1));
        assert fibonacci(BigInteger.TEN).equals(BigInteger.valueOf(55));

        for (int index = 0; index <= 100; index++) {
            assert fibonacci(BigInteger.valueOf(index)).equals(iterativeReference(index))
                    : "mismatch at index " + index;
        }

        String expectedFibonacci1000 = "43466557686937456435688527675040625802564660517371780402481729089536555417949051890403879840079255169295922593080322634775209689623239873322471161642996440906533187938298969649928516003704476137795166849228875";
        assert fibonacci(BigInteger.valueOf(1000)).toString().equals(expectedFibonacci1000);
        expectNegativeIndexRejected();
    }
}
