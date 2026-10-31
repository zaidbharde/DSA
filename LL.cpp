#include <iostream>
using namespace std;

struct Node {
    int data;
    Node* next;
    Node(int d) : data(d), next(nullptr) {}
};

void insert(Node*& head, int data) {
    Node* n = new Node(data);
    if (!head) { head = n; return; }
    Node* curr = head;
    while (curr->next) curr = curr->next;
    curr->next = n;
}

void display(Node* head) {
    while (head) {
        cout << head->data << " -> ";
        head = head->next;
    }
    cout << "null" << endl;
}

int main() {
    Node* head = nullptr;
    insert(head, 10);
    insert(head, 20);
    insert(head, 30);
    display(head);
    return 0;
}
