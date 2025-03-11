// using c++20
#include <iostream>
#include <cassert>
#include <string>
#include <cctype>
#include <stdexcept>

using namespace std;

template<typename T>
class StackADT {
public:
    virtual bool isEmpty() const = 0;

    virtual void push(const T & value) = 0;
    virtual T peek() const = 0;
    virtual bool pop() = 0;
};

constexpr int MIN_ARRAY_SIZE=64;

template<typename T, int N>
class ArrayStack final : StackADT<T> {
private:
    int topIndex;
    T array[N] {}; //Diff
public:
    ArrayStack() : topIndex(-1) {
        static_assert(N >= MIN_ARRAY_SIZE);
    }

    bool isEmpty() const override {
        return topIndex == -1;
    }

    void push(const T & value) override {
        if(topIndex == N - 1) {
            throw std::length_error("Max array exceeded.");
        }
        array[++topIndex] = value;
    }

    T peek() const override {
        if(isEmpty()) {
            throw std::logic_error("Peek on empty ArrayStack.");
        }

        return array[topIndex];
    }

    bool pop() override {
        if(isEmpty()) {
            return false;
        }
        topIndex--;
        return true;
    }
};

void testArrayStack() {
    ArrayStack<int, MIN_ARRAY_SIZE> stack0;
    assert(stack0.isEmpty());
    stack0.push(10);
    assert(stack0.peek() == 10);
    stack0.push(20);
    assert(stack0.peek() == 20);
    assert(stack0.pop());
    assert(stack0.peek() == 10);
    assert(stack0.pop());
    assert(stack0.isEmpty());
}

template<typename T>
class Node {
private:
    T value;
    Node* next;

public:
    Node(T value) : value(value), next(nullptr) {}
    Node(T value, Node* next) : value(value), next(next) {}

    T getValue() const {
        return value;
    }

    Node* getNext() const {
        return next;
    }

    void setNext(Node* n) {
        next = n;
    }

    void setValue(const T & v) {
        value = v;
    }
};

template<typename T>
class ListStack : public StackADT<T> {
private:
    Node<T>* top;
public:
    ListStack() : top(nullptr) {}
    ~ListStack() {
        while(pop()) {}
    }

    // Copy constructor [Diff from here]
    ListStack(const ListStack & other) {
        Node<T>* current = other.top;
        Node<T>* newTop = nullptr;
        Node<T>* tail = nullptr;

        while (current) {
            Node<T>* newNode = new Node<T>(current->getValue());
            if (!newTop) {
                newTop = newNode;
                tail = newNode;
            } else {
                tail->setNext(newNode);
                tail = newNode;
            }
            current = current->getNext();
        }
        top = newTop;
    }

    // Move constructor, Hint: Don't forget to make a "hollow" data structure.
    ListStack(ListStack && other) noexcept : top(other.top){
        other.top = nullptr;
    }

    bool isEmpty() const override {
        return top == nullptr;
    }

    void push(const T & value) override {
        top = new Node<T>(value, top);
    }

    T peek() const override {
        if(isEmpty()) {
            throw std::logic_error("Peek on empty ListStack."); // Till here
        }
        return top->getValue();
    }

    bool pop() override {
        if(isEmpty()) {
            return false;
        }
        Node<T>* temp = top;
        top = top->getNext();
        delete temp;
        return true;
    }
};

void testListStack() {
    ListStack<int> stack0;
    assert(stack0.isEmpty());
    stack0.push(10);
    assert(stack0.peek() == 10);
    stack0.push(20);
    assert(stack0.peek() == 20);
    assert(stack0.pop());
    assert(stack0.peek() == 10);
    assert(stack0.pop());
    assert(stack0.isEmpty());

    // Test copy constructor
    stack0.push(1);
    stack0.push(2);
    stack0.push(3);

    ListStack<int> stack1(stack0);
    assert(!stack0.isEmpty());
    assert(stack0.peek() == 3);
    assert(!stack1.isEmpty());
    assert(stack1.peek() == 3);

    // Test move constructor.
    ListStack<int> stack2(std::move(stack0));
    assert(stack0.isEmpty());
    assert(!stack2.isEmpty());
    assert(stack2.peek() == 3);
}

bool areCurleyBracesMatched(const string & inputString) {
    ListStack<char> stack;
    for (char ch : inputString) {
        if (ch == '{') {
            stack.push(ch);
        } else if (ch == '}') {
            if (stack.isEmpty()) {
                return false;
            }
            stack.pop();
        }
    }
    return stack.isEmpty();
}

void testAreCurleyBracesMatched() {
    assert(areCurleyBracesMatched(""));
    assert(areCurleyBracesMatched("{}"));
    assert(areCurleyBracesMatched("a{bc}d"));
    assert(!areCurleyBracesMatched("{"));
    assert(!areCurleyBracesMatched("}"));
    assert(!areCurleyBracesMatched("a{b{c}"));
};

bool isPalindrome(const string & inputString) // Diff from here
{
    if (inputString.empty()) {
        return true;
    }

    ListStack<char> stack;
    int mid = inputString.length() / 2;
    for (int i = 0; i < mid; ++i) {
        stack.push(inputString[i]);
    }

    int start = (inputString.length() % 2 == 0) ? mid : mid + 1;
    for (int i = start; i < inputString.length(); ++i) {
        if (stack.isEmpty() || stack.peek() != inputString[i]) {
            return false;
        }
        stack.pop();
    }
    return stack.isEmpty(); // Till here
}

void testIsPalindrome() {
    assert(isPalindrome(""));
    assert(isPalindrome("a"));
    assert(isPalindrome("aa"));
    assert(isPalindrome("aba"));
    assert(isPalindrome("abba"));
    assert(!isPalindrome("ab"));
    assert(!isPalindrome("abaa"));
}

string reversedString(const string & inputString) {
    ListStack<char> stack;
    for (char ch : inputString) {
        stack.push(ch);
    }
    string reversed;
    while (!stack.isEmpty()) {
        reversed += stack.peek();
        stack.pop();
    }
    return reversed; 
}

void testReversedString() {
    assert(reversedString("").empty());
    assert(reversedString("a")=="a");
    assert(reversedString("ab")=="ba");
    assert(reversedString("abc")=="cba");
}

// Helper for infixToPostFix.
int precedence(char op) {
    if(op == '*' || op == '/') {
        return 2;
    } else {
        assert(op == '+' || op == '-');
        return 1;
    }
}

// Helper for infixToPostFix.
bool isOperator(char ch) {
    return ch == '+' || ch == '-' || ch == '*' || ch == '/';
}

// Helper for infixToPostFix.
bool isOperand(char ch) {
    return isalpha(ch);
}

string infixToPostFix(const string & infix) {
    string postfix;
    ListStack<char> operators;

    for (char ch : infix) {
        if (isOperand(ch)) {
            postfix += ch;
        } else if (isOperator(ch)) {
            while (!operators.isEmpty() && operators.peek() != '(' && precedence(ch) <= precedence(operators.peek())) {
                postfix += operators.peek();
                operators.pop();
            }
            operators.push(ch);
        } else if (ch == '(') {
            operators.push(ch);
        } else if (ch == ')') {
            while (!operators.isEmpty() && operators.peek() != '(') {
                postfix += operators.peek();
                operators.pop();
            }
            operators.pop(); // Pop the '('
        }
    }

    while (!operators.isEmpty()) {
        postfix += operators.peek();
        operators.pop();
    }

    return postfix;
}

void testInfixToPostFix() {
    assert(infixToPostFix("").empty());

    assert(infixToPostFix("a") == "a");
    assert(infixToPostFix("a+b") == "ab+");
    assert(infixToPostFix("a*b") == "ab*");

    assert(infixToPostFix("a+b*c") == "abc*+");
    assert(infixToPostFix("a+(b*c)") == "abc*+");
    assert(infixToPostFix("(a+(b*c))") == "abc*+");

    assert(infixToPostFix("(a+b)*c") == "ab+c*");
    assert(infixToPostFix("((a+b)*c)") == "ab+c*");

    assert(infixToPostFix("a*b+c") == "ab*c+");
    assert(infixToPostFix("(a*b)+c") == "ab*c+");
    assert(infixToPostFix("((a*b)+c)") == "ab*c+");
}

int main() {
    testArrayStack();
    testListStack();
    testAreCurleyBracesMatched();
    testIsPalindrome();
    testReversedString();
    testInfixToPostFix();
    return 0;
}