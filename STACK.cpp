#include <iostream>
using namespace std;

class Stack {
    int arr[100];
    int top;
public:
    Stack() : top(-1) {}
    void push(int val) { arr[++top] = val; }
    int pop() { return arr[top--]; }
    int peek() { return arr[top]; }
    bool isEmpty() { return top == -1; }
};

int main() {
    Stack s;
    s.push(10);
    s.push(20);
    s.push(30);
    cout << s.pop() << endl;
    cout << s.peek() << endl;
    return 0;
}
