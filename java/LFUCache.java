import java.util.HashMap;
import java.util.Map;
import java.util.Random;

/**
 * An LFU cache with LRU tie-breaking. Updating an existing key counts as an
 * access and therefore increases its frequency. A missing key returns -1.
 * Frequency counters saturate at Long.MAX_VALUE to prevent overflow; accesses
 * at that limit still update recency within the maximum-frequency bucket.
 *
 * <p>Expected average time per get/put is O(1), assuming expected O(1)
 * HashMap operations. Storage is O(capacity).</p>
 */
public final class LFUCache {
    private static final long MAX_FREQUENCY = Long.MAX_VALUE;

    private static final class Node {
        private final int key;
        private int value;
        private long frequency;
        private Node previous;
        private Node next;

        private Node(int key, int value, long frequency) {
            this.key = key;
            this.value = value;
            this.frequency = frequency;
        }
    }

    /** Intrusive doubly-linked list, ordered least to most recently used. */
    private static final class FrequencyList {
        private final Node head = new Node(0, 0, 0L);
        private final Node tail = new Node(0, 0, 0L);
        private int size;

        private FrequencyList() {
            head.next = tail;
            tail.previous = head;
        }

        private void addMostRecent(Node node) {
            Node last = tail.previous;
            last.next = node;
            node.previous = last;
            node.next = tail;
            tail.previous = node;
            size++;
        }

        private void remove(Node node) {
            node.previous.next = node.next;
            node.next.previous = node.previous;
            node.previous = null;
            node.next = null;
            size--;
        }

        private Node removeLeastRecent() {
            if (size == 0) {
                throw new IllegalStateException("cannot evict from an empty frequency list");
            }
            Node leastRecent = head.next;
            remove(leastRecent);
            return leastRecent;
        }

        private boolean isEmpty() {
            return size == 0;
        }
    }

    private final int capacity;
    private final Map<Integer, Node> nodes = new HashMap<>();
    private final Map<Long, FrequencyList> frequencyLists = new HashMap<>();
    private long minimumFrequency;

    /** Creates a cache with the given nonnegative capacity. */
    public LFUCache(int capacity) {
        if (capacity < 0) {
            throw new IllegalArgumentException("capacity must be nonnegative");
        }
        this.capacity = capacity;
    }

    /** Returns the value for key and records an access, or -1 if absent. */
    public int get(int key) {
        Node node = nodes.get(key);
        if (node == null) {
            return -1;
        }
        incrementFrequency(node);
        return node.value;
    }

    /** Inserts or updates key; capacity zero is a no-op. */
    public void put(int key, int value) {
        if (capacity == 0) {
            return;
        }

        Node existing = nodes.get(key);
        if (existing != null) {
            existing.value = value;
            incrementFrequency(existing);
            return;
        }

        if (nodes.size() == capacity) {
            FrequencyList leastFrequent = frequencyLists.get(minimumFrequency);
            Node evicted = leastFrequent.removeLeastRecent();
            nodes.remove(evicted.key);
            if (leastFrequent.isEmpty()) {
                frequencyLists.remove(minimumFrequency);
            }
        }

        Node inserted = new Node(key, value, 1L);
        nodes.put(key, inserted);
        frequencyLists.computeIfAbsent(1L, ignored -> new FrequencyList())
                .addMostRecent(inserted);
        minimumFrequency = 1L;
    }

    private void incrementFrequency(Node node) {
        FrequencyList oldList = frequencyLists.get(node.frequency);
        if (node.frequency == MAX_FREQUENCY) {
            // Keep the counter in range while retaining LRU order at the cap.
            oldList.remove(node);
            oldList.addMostRecent(node);
            return;
        }

        long oldFrequency = node.frequency;
        oldList.remove(node);
        if (oldList.isEmpty()) {
            frequencyLists.remove(oldFrequency);
            if (minimumFrequency == oldFrequency) {
                minimumFrequency = oldFrequency + 1L;
            }
        }

        node.frequency = oldFrequency + 1L;
        frequencyLists.computeIfAbsent(node.frequency, ignored -> new FrequencyList())
                .addMostRecent(node);
    }

    private static final class ReferenceEntry {
        private int value;
        private long frequency = 1L;
        private long lastAccess;

        private ReferenceEntry(int value, long lastAccess) {
            this.value = value;
            this.lastAccess = lastAccess;
        }
    }

    /** Deliberately slow model used to differentially check the implementation. */
    private static final class SlowReference {
        private final int capacity;
        private final Map<Integer, ReferenceEntry> entries = new HashMap<>();
        private long clock;

        private SlowReference(int capacity) {
            this.capacity = capacity;
        }

        private int get(int key) {
            ReferenceEntry entry = entries.get(key);
            if (entry == null) {
                return -1;
            }
            recordAccess(entry);
            return entry.value;
        }

        private void put(int key, int value) {
            if (capacity == 0) {
                return;
            }
            ReferenceEntry entry = entries.get(key);
            if (entry != null) {
                entry.value = value;
                recordAccess(entry);
                return;
            }
            if (entries.size() == capacity) {
                int victimKey = 0;
                ReferenceEntry victim = null;
                for (Map.Entry<Integer, ReferenceEntry> candidate : entries.entrySet()) {
                    ReferenceEntry candidateEntry = candidate.getValue();
                    if (victim == null
                            || candidateEntry.frequency < victim.frequency
                            || (candidateEntry.frequency == victim.frequency
                                    && candidateEntry.lastAccess < victim.lastAccess)) {
                        victimKey = candidate.getKey();
                        victim = candidateEntry;
                    }
                }
                entries.remove(victimKey);
            }
            entries.put(key, new ReferenceEntry(value, ++clock));
        }

        private void recordAccess(ReferenceEntry entry) {
            if (entry.frequency < MAX_FREQUENCY) {
                entry.frequency++;
            }
            entry.lastAccess = ++clock;
        }
    }

    private static void testEvictionOrderAndFrequency() {
        LFUCache cache = new LFUCache(2);
        cache.put(1, 10);
        cache.put(2, 20);
        assert cache.get(1) == 10; // key 1 is now more frequent
        cache.put(3, 30);          // evicts key 2
        assert cache.get(2) == -1;
        assert cache.get(1) == 10;
        assert cache.get(3) == 30;
    }

    private static void testLruTieBreakAndUpdates() {
        LFUCache cache = new LFUCache(2);
        cache.put(1, 10);
        cache.put(2, 20);
        assert cache.get(1) == 10; // key 1 becomes frequency 2
        cache.put(1, 11);          // update also increments frequency
        cache.put(3, 30);          // among frequency-1 keys, key 2 is older
        assert cache.get(2) == -1;
        assert cache.get(1) == 11;
        assert cache.get(3) == 30;

        LFUCache ties = new LFUCache(2);
        ties.put(4, 40);
        ties.put(5, 50);
        assert ties.get(4) == 40; // key 5 is the LRU at frequency 1
        ties.put(6, 60);
        assert ties.get(5) == -1;
        assert ties.get(4) == 40;
    }

    private static void testMissesAndCapacities() {
        LFUCache zero = new LFUCache(0);
        zero.put(1, 10);
        assert zero.get(1) == -1;

        LFUCache one = new LFUCache(1);
        assert one.get(9) == -1;
        one.put(1, 10);
        assert one.get(1) == 10;
        one.put(1, 12);
        assert one.get(1) == 12;
        one.put(2, 20);
        assert one.get(1) == -1;
        assert one.get(2) == 20;

        boolean rejectedNegativeCapacity = false;
        try {
            new LFUCache(-1);
        } catch (IllegalArgumentException expected) {
            rejectedNegativeCapacity = true;
        }
        assert rejectedNegativeCapacity;
    }

    private static void testRandomizedAgainstReference() {
        Random random = new Random(0x1F0CA5E);
        for (int capacity = 0; capacity <= 5; capacity++) {
            LFUCache actual = new LFUCache(capacity);
            SlowReference reference = new SlowReference(capacity);
            for (int operation = 0; operation < 20_000; operation++) {
                int key = random.nextInt(9) - 4;
                if (random.nextInt(3) == 0) {
                    assert actual.get(key) == reference.get(key)
                            : "get mismatch at capacity " + capacity
                                    + ", operation " + operation;
                } else {
                    int value = random.nextInt();
                    actual.put(key, value);
                    reference.put(key, value);
                }
                for (int probe = -4; probe <= 4; probe++) {
                    assert actual.get(probe) == reference.get(probe)
                            : "state mismatch at capacity " + capacity
                                    + ", operation " + operation
                                    + ", key " + probe;
                }
            }
        }
    }

    public static void main(String[] args) {
        testEvictionOrderAndFrequency();
        testLruTieBreakAndUpdates();
        testMissesAndCapacities();
        testRandomizedAgainstReference();
        System.out.println("LFUCache tests passed");
    }
}
