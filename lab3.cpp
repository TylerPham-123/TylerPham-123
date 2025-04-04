#include <iostream>
#include <cassert>
#include <string>
#include <stdexcept>

using namespace std;

template<typename T> // Makes the ListInterface an abstract base class that can define the common operations for lists of any type T
class ListInterface {
public:
    virtual bool isEmpty() const = 0;
    virtual int getLength() const = 0;
    virtual bool insert(int newPosition, const T &newEntry) = 0;
    virtual bool remove(int position) = 0;
    virtual void clear() = 0;
    virtual T getEntry(int position) const = 0;
    virtual void setEntry(int position, const T &newEntry) = 0;
};

constexpr int MIN_ARRAY_SIZE = 64;

// **** PART 1 *****

template<typename T, int N>
class ArrayList final : public ListInterface<T> {
private:
    int itemCount;
    int maxItems;
    int items[N]{};
public:
    ArrayList() : itemCount(0), maxItems(N) {
        static_assert(N >= MIN_ARRAY_SIZE);
    }

    bool isEmpty() const {
        return itemCount == 0;
    }

    int getLength() const {
        return itemCount;
    };

    bool insert(int newPosition, const T &newEntry) {
        bool ableToInsert = (newPosition >= 1) &&
                            (newPosition <= itemCount + 1) &&
                            (itemCount < maxItems);
        if (ableToInsert) {
            // Make room for new entry by shifting all entries at
            // positions >= newPosition toward the end of the array
            // (no shift if newPosition == itemCount + 1)
            for (int pos = itemCount; pos >= newPosition; pos--)
                items[pos] = items[pos - 1];
            // Insert new entry
            items[newPosition - 1] = newEntry;
            itemCount++; // Increase count of entries
        } // end if
        return ableToInsert;
    }

    bool remove(int position) {
        bool ableToRemove = (position >= 1) && (position <= itemCount);
        if (ableToRemove) {
            // Remove entry by shifting all entries after the one at
            // position toward the beginning of the array
            // (no shift if position == itemCount)
            for (int fromIndex = position, toIndex = fromIndex - 1;
                 fromIndex < itemCount; fromIndex++, toIndex++)
                items[toIndex] = items[fromIndex];
            itemCount--; // Decrease count of entries
        } // end if

        return ableToRemove;
    };

    void clear() {
        itemCount = 0;
    };

    T getEntry(int position) const {
        // Enforce precondition
        bool ableToGet = (position >= 1) && (position <= itemCount);
        if (ableToGet)
            return items[position - 1];
        else {
            string message = "getEntry() called with an empty list or ";
            message = message + "invalid position.";
            throw (std::invalid_argument(message));
        } // end if
    } // end getEntry

    void setEntry(int position, const T &newEntry) {
        // Enforce precondition
        bool ableToSet = (position >= 1) && (position <= itemCount);
        if (ableToSet)
            items[position - 1] = newEntry;
        else {
            string message = "setEntry() called with an empty list or ";
            message = message + "invalid position.";
            throw (std::invalid_argument(message));
        } // end if
    }
};

void testArrayList() {
    ArrayList<int, MIN_ARRAY_SIZE> array0;
    assert(array0.isEmpty());
    assert(array0.getLength() == 0);

    // Example of how to test that an exception throw.
    try {
        array0.getEntry(1);
        assert(false);
    }
    catch (std::invalid_argument &err) {}

    // TODO: Finish adding tests.
}

template<typename T>
class Node {
private:
    T value;
    Node *next;

public:
    Node(T value) : value(value), next(nullptr) {}

    Node(T value, Node *next) : value(value), next(next) {}

    T getItem() const {
        return value;
    }

    Node *getNext() const {
        return next;
    }

    void setNext(Node *n) {
        next = n;
    }

    void setItem(const T &v) {
        value = v;
    }
};

template<class ItemType>
class LinkedList : public ListInterface<ItemType> {
private :
    // Pointer to first node in the chain (contains the first entry in the list)
    Node<ItemType> *headPtr;

    // Current count of list items
    int itemCount;

    // Locates a specified node in a linked list.
    // @pre position is the number of the desired node;
    // position >= 1 and position <= itemCount.
    // @post The node is found and a pointer to it is returned.
    // @param position The number of the node to locate.
    // @return A pointer to the node at the given position.
    Node<ItemType> *getNodeAt(int position) const {
        // Debugging check of precondition
        assert ((position >= 1) && (position <= itemCount));
        // Count from the beginning of the chain
        Node<ItemType> *curPtr = headPtr;
        for (int skip = 1; skip < position; skip++)
            curPtr = curPtr->getNext();
        return curPtr;
    }

public :
    LinkedList() : headPtr(nullptr), itemCount(0) {}

    ~LinkedList() {
        clear();
    };

    bool isEmpty() const {
        return itemCount == 0;
    }

    int getLength() const {
        return itemCount;
    }

    bool insert(int newPosition, const ItemType &newEntry) {
        bool ableToInsert = (newPosition >= 1) &&
                            (newPosition <= itemCount + 1);
        if (ableToInsert) {
            // Create a new node containing the new entry
            Node<ItemType> *newNodePtr = new Node<ItemType>(newEntry);
            // Attach new node to chain
            if (newPosition == 1) {
                // Insert new node at beginning of chain
                newNodePtr->setNext(headPtr);
                headPtr = newNodePtr;
            } else {
                // Find node that will be before new node
                Node<ItemType> *prevPtr = getNodeAt(newPosition - 1);
                // Insert new node after node to which prevPtr points
                newNodePtr->setNext(prevPtr->getNext());
                prevPtr->setNext(newNodePtr);
            } // end if
            itemCount++; // Increase count of entries
        } // end if
        return ableToInsert;
    }

    bool remove(int position) {
        bool ableToRemove = (position >= 1) && (position <= itemCount);
        if (ableToRemove) {
            Node<ItemType> *curPtr = nullptr;
            if (position == 1) {
                // Remove the first node in the chain
                curPtr = headPtr; // Save pointer to node
                headPtr = headPtr->getNext();
            } else {
                // Find node that is before the one to delete
                Node<ItemType> *prevPtr = getNodeAt(position - 1);
                // Point to node to delete
                curPtr = prevPtr->getNext();
                // Disconnect indicated node from chain by connecting the
                // prior node with the one after
                prevPtr->setNext(curPtr->getNext());
            } // end if
            // Return node to system
            curPtr->setNext(nullptr);
            delete curPtr;
            curPtr = nullptr;
            itemCount--; // Decrease count of entries
        } // end if
        return ableToRemove;
    }

    void clear() {
        while (!isEmpty())
            remove(1);
    }

    /** @throw invalid_argument if position < 1 or position > getLength(). */
    ItemType getEntry(int position) const {
        // Enforce precondition
        bool ableToGet = (position >= 1) && (position <= itemCount);
        if (ableToGet) {
            Node<ItemType> *nodePtr = getNodeAt(position);
            return nodePtr->getItem();
        } else {
            string message = "getEntry() called with an empty list or ";
            message = message + "invalid position.";
            throw (std::invalid_argument(message));
        }
    }

    /** @throw invalid_argument if position < 1 or position > getLength(). */
    void setEntry(int position, const ItemType &newEntry) {
        Node<ItemType> *n = getNodeAt(position);
        n->setItem(newEntry);
    }
}; // end LinkedList

void testLinkedList() {
    LinkedList<int> list0;
    // TODO: Add tests
}

// ***** PART 2 *****

template<typename ItemType>
void insertionSort(ListInterface<ItemType> & list) {
    // TODO
}

void fillRandom(LinkedList<int> & list, int n) {
    for(int i=0; i<n; ++i) {
        int j = std::rand();
        list.insert(1, j);
    }
}

bool isSorted(const LinkedList<int> & list) {
    int n = list.getLength();
    if(n <= 1) {
        return true;
    }

    for(int i=1; i<n; ++i) {
        if(list.getEntry(i) > list.getEntry(i+1)) {
            return false;
        }
    }

    return true;
}

void testLinkedInsertionSort() {
    LinkedList<int> list0;
    insertionSort(list0);
    assert(list0.isEmpty());

    LinkedList<int> list1;
    list1.insert(1, 10);
    insertionSort(list1);
    assert(list1.getLength() == 1);

    LinkedList<int> list2;
    list2.insert(1, 10);
    list2.insert(1, 20);
    insertionSort(list2);
    assert(list2.getEntry(1) == 10);
    assert(list2.getEntry(2) == 20);

    LinkedList<int> list3;
    list3.insert(1, 10);
    list3.insert(1, 20);
    list3.insert(1, 30);

    insertionSort(list3);

    assert(list3.getEntry(1) == 10);
    assert(list3.getEntry(2) == 20);
    assert(list3.getEntry(3) == 30);

    LinkedList<int> listRandom;
    fillRandom(listRandom, 32);
    insertionSort(listRandom);
    assert(isSorted(listRandom));
}

void testArrayInsertionSort() {
    ArrayList<int, MIN_ARRAY_SIZE> list0;
    insertionSort(list0);
    assert(list0.isEmpty());

    ArrayList<int, MIN_ARRAY_SIZE> list1;
    list1.insert(1, 10);
    insertionSort(list1);
    assert(list1.getLength() == 1);

    ArrayList<int, MIN_ARRAY_SIZE> list2;
    list2.insert(1, 10);
    list2.insert(1, 20);
    insertionSort(list2);
    assert(list2.getEntry(1) == 10);
    assert(list2.getEntry(2) == 20);

    ArrayList<int, MIN_ARRAY_SIZE> list3;
    list3.insert(1, 10);
    list3.insert(1, 20);
    list3.insert(1, 30);

    insertionSort(list3);

    assert(list3.getEntry(1) == 10);
    assert(list3.getEntry(2) == 20);
    assert(list3.getEntry(3) == 30);

    LinkedList<int> listRandom;
    fillRandom(listRandom, 32);
    insertionSort(listRandom);
    assert(isSorted(listRandom));
}

// ***** PART 3 ****

// TODO: Add Playlist ADT here.

// ***** PART 4 ****

template<class ItemType>
class SmartLinkedList : public ListInterface<ItemType> {
private :
    // TODO: Finish with smart pointers.
    //       Use LinkedList implementation as a guide replacing raw pointers with shared pointers.
public:
    // TODO: Finish with smart pointers.
}; // end SmartLinkedList

void testSmartLinkedList() {
    //SmartLinkedList<int> list0;
    // TODO: Add tests
}

int main() {
    std::srand(0);

    testArrayList();
    testLinkedList();
    testLinkedInsertionSort();
    testArrayInsertionSort();
    testSmartLinkedList();
    return 0;
}