public class Stack {
    private final int[] arr;
    private int top;

    /**
     * A fixed-capacity integer stack. push, pop, peek, isEmpty, and size take
     * O(1) time; the backing array uses O(capacity) space.
     */
    Stack(int capacity) {
        if (capacity < 0) {
            throw new IllegalArgumentException("capacity must not be negative");
        }
        arr = new int[capacity];
        top = -1;
    }

    /** Pushes a value, or throws IllegalStateException when the stack is full. */
    void push(int val) {
        if (top == arr.length - 1) {
            throw new IllegalStateException("stack is full");
        }
        arr[++top] = val;
    }

    /** Removes and returns the top value, or throws IllegalStateException when empty. */
    int pop() {
        if (isEmpty()) {
            throw new IllegalStateException("stack is empty");
        }
        return arr[top--];
    }

    /** Returns the top value without removing it, or throws IllegalStateException when empty. */
    int peek() {
        if (isEmpty()) {
            throw new IllegalStateException("stack is empty");
        }
        return arr[top];
    }

    boolean isEmpty() {
        return top == -1;
    }

    int size() {
        return top + 1;
    }

    private static void check(boolean condition, String message) {
        if (!condition) {
            throw new AssertionError(message);
        }
    }

    private static void expectIllegalState(Runnable operation, String message) {
        try {
            operation.run();
        } catch (IllegalStateException expected) {
            return;
        }
        throw new AssertionError(message);
    }

    public static void main(String[] args) {
        Stack s = new Stack(2);
        s.push(10);
        s.push(20); // Reaching the capacity boundary is valid.
        check(s.size() == 2, "size should equal capacity");
        check(s.peek() == 20, "peek should return the top value");
        expectIllegalState(() -> s.push(30), "push on a full stack should fail");
        check(s.size() == 2, "overflow must not change the stack");
        check(s.pop() == 20, "first pop should return the most recently pushed value");
        check(s.pop() == 10, "second pop should return the earlier value");
        check(s.isEmpty(), "stack should be empty after all values are popped");
        expectIllegalState(s::pop, "pop on an empty stack should fail");
        expectIllegalState(s::peek, "peek on an empty stack should fail");

        // Keep the original compact demonstration output.
        Stack demo = new Stack(5);
        demo.push(10);
        demo.push(20);
        demo.push(30);
        System.out.println(demo.pop());
        System.out.println(demo.peek());
        System.out.println("Stack checks passed");
    }
}
