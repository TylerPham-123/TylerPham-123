// using c++20
#include <iostream>
#include <cassert>
#include <string>
#include <cctype>
#include <stack>

using namespace std;

template<typename T>
class StackADT
{
public:
    virtual bool isEmpty() const = 0;

    virtual void push(const T & value) = 0;
    virtual T peek() const = 0;
    virtual bool pop() = 0;
};

constexpr int MIN_ARRAY_SIZE=64;

template<typename T, int N>
class ArrayStack final : StackADT<T> 
{
private:
    int topIndex;
    int array[N] {};
public:
    ArrayStack() : topIndex(-1) 
    {
        static_assert(N >= MIN_ARRAY_SIZE);
    }

    bool isEmpty() const override 
    {
        // TODO
        return topIndex == -1; // Stack is empty if topIndex is -1
    }

    void push(const T & value) override 
    {
        if(topIndex == N - 1) 
        {
            // Check for overflow
            throw std::length_error("Max array exceeded.");
        }
        // TODO
        topIndex++; // Increment topIndex
        array[topIndex] = value; // Store the value
    }

    T peek() const override 
    {
        if(isEmpty()) 
        {
            throw std::logic_error("Peek on empty ArrayStack.");
        }

        // TODO
        return array[topIndex]; // Return the top element
    }

    bool pop() override 
    {
        if(isEmpty()) 
        {
            return false; // Cannot pop from an empty stack
        }
        // TODO
        topIndex--; // Decrement topIndex
        return true;
    }
};

void testArrayStack() 
{
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
class Node 
{
private:
    T value;
    Node* next;

public:
    Node(T value) : value(value), next(nullptr) {}
    Node(T value, Node* next) : value(value), next(next) {}

    T getValue() const 
    {
        return value;
    }

    Node* getNext() const 
    {
        return next;
    }

    void setNext(Node* n) 
    {
        next = n;
    }

    void setValue(const T & v) 
    {
        value = v;
    }
};

template<typename T>
class ListStack : public StackADT<T> 
{
private:
    Node<T>* top;
public:
    ListStack() : top(nullptr) {}
    ~ListStack() 
    {
        while(pop()) {} // Pop until empty to deallocate memory
    }

    // Copy constructor
    ListStack(const ListStack & other) 
    {
        // TODO
        top = nullptr; // Initialize the new stack as empty
        
        if (other.top == nullptr) return; // if the other stack is empty, return.
        
        ListStack<T> tempStack; // auxiliary stack to reverse the order.
        Node<T>* current = other.top;
        while(current != nullptr)
        {
            tempStack.push(current->getValue());
            current = current->getNext();
        }
        
        while(!tempStack.isEmpty())
        {
            push(tempStack.peek());
            tempStack.pop();
        }
    }

    // Move constructor, Hint: Don't forget to make a "hollow" data structure.
    ListStack(ListStack && other) noexcept 
    {
        // TODO
        top = other.top; // Steal the other stack's top pointer
        other.top = nullptr; // Make the other stack empty
    }

    bool isEmpty() const override 
    {
        // TODO
        return top == nullptr; // Stack is empty if top is nullptr
    }

    void push(const T & value) override 
    {
        // TODO
        Node<T>* newNode = new Node<T>(value, top); // Create a new node
        top = newNode; // Update top to point to the new node
    }

    T peek() const override 
    {
        if(isEmpty()) 
        {
            throw std::logic_error("Peek on empty ArrayStack.");
        }

        // TODO
        return top->getValue(); // Return the value of the top node
    }

    bool pop() override 
    {
        if(isEmpty()) 
        {
            return false; // Cannot pop from an empty stack
        }

        // TODO
        Node<T>* temp = top; // Store the current top node
        top = top->getNext(); // Update top to the next node
        delete temp; // Delete the old top node
        return true;
    }
};

void testListStack() 
{
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

bool areCurleyBracesMatched(const string & inputString) 
{
    // TODO
    ListStack<char> stack; // Use a stack to track opening braces
    
    for (char ch : inputString)
    {
        if (ch == '{')
        {
            stack.push(ch); // Push opening brace onto the stack
        }
        else if (ch == '}')
        {
            if (stack.isEmpty())
            {
                return false; // Closing brace without matching opening brace
            }
            stack.pop(); // Pop matching opening brace
        }
    }
    return stack.isEmpty(); // Return true if the stack is empty (all braces matched)
}

void testAreCurleyBracesMatched() 
{
    assert(areCurleyBracesMatched(""));
    assert(areCurleyBracesMatched("{}"));
    assert(areCurleyBracesMatched("a{bc}d"));
    assert(!areCurleyBracesMatched("{"));
    assert(!areCurleyBracesMatched("}"));
    assert(!areCurleyBracesMatched("a{b{c}"));
};

bool isPalindrome(const string & inputString) 
{
    // TODO
    ListStack<char> stack;
    string reversed;
    
    for (char ch : inputString)
    {
        stack.push(ch); // push every character onto the stack
    }
    
    while (!stack.isEmpty())
    {
        reversed += stack.peek(); // build a reversed string.
        stack.pop();
    }
    return inputString == reversed; // Compare the original and reversed strings
}

void testIsPalindrome() 
{
    assert(isPalindrome(""));
    assert(isPalindrome("a"));
    assert(isPalindrome("aa"));
    assert(isPalindrome("aba"));
    assert(isPalindrome("abba"));
    assert(!isPalindrome("ab"));
    assert(!isPalindrome("abaa"));
}

string reversedString(const string & inputString) 
{
    // TODO
    ListStack<char> stack;
    string reversed;
    
    for (char ch: inputString)
    {
        stack.push(ch);
    }
    
    while (!stack.isEmpty())
    {
        reversed += stack.peek();
        stack.pop();
    }
    return reversed;
}

void testReversedString() 
{
    assert(reversedString("").empty());
    assert(reversedString("a")=="a");
    assert(reversedString("ab")=="ba");
    assert(reversedString("abc")=="cba");
}

// Helper for infixToPostFix.
int precedence(char op) 
{
    if(op == '*' || op == '/') 
    {
        return 2;
    } 
    else 
    {
        assert(op == '+' || op == '-');
        return 1;
    }
}

// Helper for infixToPostFix.
bool isOperator(char ch) 
{
    return ch == '+' || ch == '-' || ch == '*' || ch == '/';
}

// Helper for infixToPostFix.
bool isOperand(char ch) 
{
    return isalpha(ch);
}

string infixToPostFix(const string & infix) 
{
    // TODO
    stack<char> aStack;
    string postfixExp;
    
    for (char ch : infix)
    {
        if (isOperand(ch))
        {
            postfixExp += ch; // Append operand to postfix expression
        }
        else if (ch == '(')
        {
            aStack.push(ch); // Push '(' onto the stack
        }
        else if (isOperator(ch))
        {
            while (!aStack.empty() && aStack.top() != '(' && precedence(ch) <= precedence(aStack.top()))
            {
                postfixExp += aStack.top(); // Append operator from stack to postfix
                aStack.pop();
            }
            aStack.push(ch); // Push current operator onto the stack
        }
        else if (ch == ')') 
        {
            while (!aStack.empty() && aStack.top() != '(')
            {
                postfixExp += aStack.top(); // Append operator from stack to postfix
                aStack.pop();
            }
            aStack.pop(); // Remove the '('
        }
    }
    while (!aStack.empty())
    {
        postfixExp += aStack.top(); // Append remaining operators to postfix
        aStack.pop();
    }
    return postfixExp;
}

void testInfixToPostFix() 
{
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

int main() 
{
    testArrayStack();
    testListStack();
    testAreCurleyBracesMatched();
    testIsPalindrome();
    testReversedString();
    testInfixToPostFix();
    return 0;
}
