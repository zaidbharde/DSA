public class LinkedList {
    static class Node {
        int data;
        Node next;
        Node(int data) { this.data = data; }
    }

    Node head;

    void insert(int data) {
        Node n = new Node(data);
        if (head == null) { head = n; return; }
        Node curr = head;
        while (curr.next != null) curr = curr.next;
        curr.next = n;
    }

    void display() {
        Node curr = head;
        while (curr != null) {
            System.out.print(curr.data + " -> ");
            curr = curr.next;
        }
        System.out.println("null");
    }

    public static void main(String[] args) {
        LinkedList ll = new LinkedList();
        ll.insert(10);
        ll.insert(20);
        ll.insert(30);
        ll.display();
    }
}
