#include <iostream>
#include <cassert>
#include <string>
#include <stdexcept>
#include <memory>
#include <vector>

using namespace std;

template <typename T>
class ListInterface
{
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

template <typename T, int N>
class ArrayList final : public ListInterface<T>
{
private:
    int itemCount;
    int maxItems;
    int items[N]{};

public:
    ArrayList() : itemCount(0), maxItems(N)
    {
        static_assert(N >= MIN_ARRAY_SIZE);
    }

    bool isEmpty() const
    {
        return itemCount == 0;
    }

    int getLength() const
    {
        return itemCount;
    };

    bool insert(int newPosition, const T &newEntry)
    {
        bool ableToInsert = (newPosition >= 1) &&
                            (newPosition <= itemCount + 1) &&
                            (itemCount < maxItems);
        if (ableToInsert)
        {
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

    bool remove(int position)
    {
        bool ableToRemove = (position >= 1) && (position <= itemCount);
        if (ableToRemove)
        {
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

    void clear()
    {
        itemCount = 0;
    };

    T getEntry(int position) const
    {
        // Enforce precondition
        bool ableToGet = (position >= 1) && (position <= itemCount);
        if (ableToGet)
            return items[position - 1];
        else
        {
            string message = "getEntry() called with an empty list or ";
            message = message + "invalid position.";
            throw(std::invalid_argument(message));
        } // end if
    } // end getEntry

    void setEntry(int position, const T &newEntry)
    {
        // Enforce precondition
        bool ableToSet = (position >= 1) && (position <= itemCount);
        if (ableToSet)
            items[position - 1] = newEntry;
        else
        {
            string message = "setEntry() called with an empty list or ";
            message = message + "invalid position.";
            throw(std::invalid_argument(message));
        } // end if
    }
};

void testArrayList()
{
    ArrayList<int, MIN_ARRAY_SIZE> array0;
    assert(array0.isEmpty());
    assert(array0.getLength() == 0);

    // Example of how to test that an exception throw.
    try
    {
        array0.getEntry(1);
        assert(false);
    }
    catch (std::invalid_argument &err)
    {
    }

    // TODO: Finish adding tests.

    // Test inserting elements
    assert(array0.insert(1, 10));
    assert(array0.insert(2, 20));
    assert(array0.insert(3, 30));
    assert(array0.getLength() == 3);
    assert(array0.getEntry(1) == 10);
    assert(array0.getEntry(2) == 20);
    assert(array0.getEntry(3) == 30);

    // Test removing elements
    assert(array0.remove(2));
    assert(array0.getLength() == 2);
    assert(array0.getEntry(1) == 10);
    assert(array0.getEntry(2) == 30);

    // Test setting an entry
    array0.setEntry(2, 40);
    assert(array0.getEntry(2) == 40);

    // Test clearing the list
    array0.clear();
    assert(array0.isEmpty());
}

template <typename T>
class Node
{
private:
    T value;
    Node *next;

public:
    Node(T value) : value(value), next(nullptr) {}

    Node(T value, Node *next) : value(value), next(next) {}

    T getItem() const
    {
        return value;
    }

    Node *getNext() const
    {
        return next;
    }

    void setNext(Node *n)
    {
        next = n;
    }

    void setItem(const T &v)
    {
        value = v;
    }
};

template <class ItemType>
class LinkedList : public ListInterface<ItemType>
{
private:
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
    Node<ItemType> *getNodeAt(int position) const
    {
        // Debugging check of precondition
        assert((position >= 1) && (position <= itemCount));
        // Count from the beginning of the chain
        Node<ItemType> *curPtr = headPtr;
        for (int skip = 1; skip < position; skip++)
            curPtr = curPtr->getNext();
        return curPtr;
    }

public:
    LinkedList() : headPtr(nullptr), itemCount(0) {}

    ~LinkedList()
    {
        clear();
    };

    bool isEmpty() const
    {
        return itemCount == 0;
    }

    int getLength() const
    {
        return itemCount;
    }

    bool insert(int newPosition, const ItemType &newEntry)
    {
        bool ableToInsert = (newPosition >= 1) &&
                            (newPosition <= itemCount + 1);
        if (ableToInsert)
        {
            // Create a new node containing the new entry
            Node<ItemType> *newNodePtr = new Node<ItemType>(newEntry);
            // Attach new node to chain
            if (newPosition == 1)
            {
                // Insert new node at beginning of chain
                newNodePtr->setNext(headPtr);
                headPtr = newNodePtr;
            }
            else
            {
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

    bool remove(int position)
    {
        bool ableToRemove = (position >= 1) && (position <= itemCount);
        if (ableToRemove)
        {
            Node<ItemType> *curPtr = nullptr;
            if (position == 1)
            {
                // Remove the first node in the chain
                curPtr = headPtr; // Save pointer to node
                headPtr = headPtr->getNext();
            }
            else
            {
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

    void clear()
    {
        while (!isEmpty())
            remove(1);
    }

    /** @throw invalid_argument if position < 1 or position > getLength(). */
    ItemType getEntry(int position) const
    {
        // Enforce precondition
        bool ableToGet = (position >= 1) && (position <= itemCount);
        if (ableToGet)
        {
            Node<ItemType> *nodePtr = getNodeAt(position);
            return nodePtr->getItem();
        }
        else
        {
            string message = "getEntry() called with an empty list or ";
            message = message + "invalid position.";
            throw(std::invalid_argument(message));
        }
    }

    /** @throw invalid_argument if position < 1 or position > getLength(). */
    void setEntry(int position, const ItemType &newEntry)
    {
        Node<ItemType> *n = getNodeAt(position);
        n->setItem(newEntry);
    }
}; // end LinkedList

void testLinkedList()
{
    LinkedList<int> list0;
    // TODO: Add tests
    assert(list0.isEmpty());

    // Test inserting elements
    assert(list0.insert(1, 10));
    assert(list0.insert(2, 20));
    assert(list0.insert(3, 30));
    assert(list0.getLength() == 3);
    assert(list0.getEntry(1) == 10);
    assert(list0.getEntry(2) == 20);
    assert(list0.getEntry(3) == 30);

    // Test removing elements
    assert(list0.remove(2));
    assert(list0.getLength() == 2);
    assert(list0.getEntry(1) == 10);
    assert(list0.getEntry(2) == 30);

    // Test setting an entry
    list0.setEntry(2, 40);
    assert(list0.getEntry(2) == 40);

    // Test clearing the list
    list0.clear();
    assert(list0.isEmpty());
}

// ***** PART 2 *****

template <typename ItemType>
void insertionSort(ListInterface<ItemType> &list)
{
    // TODO
    int n = list.getLength();
    for (int unsorted = 2; unsorted <= n; ++unsorted)
    {
        ItemType nextItem = list.getEntry(unsorted);
        int loc = unsorted - 1;

        while ((loc > 0) && (list.getEntry(loc) > nextItem))
        {
            list.setEntry(loc + 1, list.getEntry(loc));
            --loc;
        }
        list.setEntry(loc + 1, nextItem);
    }
}

void fillRandom(LinkedList<int> &list, int n)
{
    for (int i = 0; i < n; ++i)
    {
        int j = std::rand();
        list.insert(1, j);
    }
}

bool isSorted(const LinkedList<int> &list)
{
    int n = list.getLength();
    if (n <= 1)
    {
        return true;
    }

    for (int i = 1; i < n; ++i)
    {
        if (list.getEntry(i) > list.getEntry(i + 1))
        {
            return false;
        }
    }

    return true;
}

void testLinkedInsertionSort()
{
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

void testArrayInsertionSort()
{
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

// ***** PART 3 ****

enum class PlaybackMode
{
    LOOP,
    RANDOM,
    ONCE
};

template <typename T>
class Playlist
{
private:
    LinkedList<T> songs;
    int currentSongIndex;
    PlaybackMode mode;
    std::vector<int> randomHistory;
    bool isShuffled;

    void shuffleSongs()
    {
        std::srand(time(nullptr));
        randomHistory.clear();
        isShuffled = true;
    }

public:
    Playlist() : currentSongIndex(0), mode(PlaybackMode::ONCE), isShuffled(false) {}

    void addSong(const T &song)
    {
        songs.insert(songs.getLength() + 1, song);
    }

    void removeSong(int position)
    {
        if (position >= 1 && position <= songs.getLength())
        {
            songs.remove(position);
            if (currentSongIndex >= position)
            {
                currentSongIndex = std::max(0, currentSongIndex - 1);
            }
        }
        else
        {
            throw std::invalid_argument("Invalid position to remove song.");
        }
    }

    T playCurrentSong() const
    {
        if (songs.isEmpty())
        {
            throw std::runtime_error("Playlist is empty. No song to play.");
        }
        return songs.getEntry(currentSongIndex + 1);
    }

    void nextSong()
    {
        if (songs.isEmpty())
        {
            throw std::runtime_error("Playlist is empty. No next song.");
        }

        switch (mode)
        {
        case PlaybackMode::LOOP:
            currentSongIndex = (currentSongIndex + 1) % songs.getLength();
            break;
        case PlaybackMode::RANDOM:
            if (!isShuffled)
            {
                shuffleSongs();
            }
            if (randomHistory.size() == songs.getLength())
            {
                randomHistory.clear();
            }
            do
            {
                currentSongIndex = std::rand() % songs.getLength();
            } while (std::find(randomHistory.begin(), randomHistory.end(), currentSongIndex) != randomHistory.end()); // I dont know why it says "not matching function," so I try to "Debug anyway" and the code runs just good. Please give me a comment on this.
            randomHistory.push_back(currentSongIndex);
            break;
        case PlaybackMode::ONCE:
            if (currentSongIndex + 1 < songs.getLength())
            {
                currentSongIndex++;
            }
            else
            {
                throw std::runtime_error("End of playlist reached.");
            }
            break;
        }
    }

    void previousSong()
    {
        if (songs.isEmpty())
        {
            throw std::runtime_error("Playlist is empty. No previous song.");
        }

        if (mode == PlaybackMode::RANDOM)
        {
            if (randomHistory.empty())
            {
                throw std::runtime_error("No previous song in RANDOM mode.");
            }
            randomHistory.pop_back();
            if (!randomHistory.empty())
            {
                currentSongIndex = randomHistory.back();
            }
        }
        else
        {
            currentSongIndex = (currentSongIndex - 1 + songs.getLength()) % songs.getLength();
        }
    }

    void setMode(PlaybackMode newMode)
    {
        mode = newMode;
        if (mode == PlaybackMode::RANDOM)
        {
            shuffleSongs();
        }
    }

    PlaybackMode getMode() const
    {
        return mode;
    }

    void clearPlaylist()
    {
        songs.clear();
        currentSongIndex = 0;
        randomHistory.clear();
        isShuffled = false;
    }

    int getTotalSongs() const
    {
        return songs.getLength();
    }

    T peekNextSong() const
    {
        if (songs.isEmpty())
        {
            throw std::runtime_error("Playlist is empty. No next song.");
        }

        int nextIndex = currentSongIndex;
        switch (mode)
        {
        case PlaybackMode::LOOP:
            nextIndex = (currentSongIndex + 1) % songs.getLength();
            break;
        case PlaybackMode::RANDOM:
            if (!isShuffled)
            {
                throw std::runtime_error("Cannot peek next song in RANDOM mode.");
            }
            nextIndex = (currentSongIndex + 1) % songs.getLength();
            break;
        case PlaybackMode::ONCE:
            if (currentSongIndex + 1 < songs.getLength())
            {
                nextIndex = currentSongIndex + 1;
            }
            else
            {
                throw std::runtime_error("End of playlist reached.");
            }
            break;
        }
        return songs.getEntry(nextIndex + 1);
    }
};

// Test Cases for Playlist ADT

void testPlaylist()
{
    Playlist<std::string> playlist;

    // Test adding songs
    playlist.addSong("Song 1");
    playlist.addSong("Song 2");
    playlist.addSong("Song 3");
    assert(playlist.getTotalSongs() == 3);

    // Test playing the current song
    assert(playlist.playCurrentSong() == "Song 1");

    // Test skipping to the next song
    playlist.nextSong();
    assert(playlist.playCurrentSong() == "Song 2");

    // Test going back to the previous song
    playlist.previousSong();
    assert(playlist.playCurrentSong() == "Song 1");

    // Test removing a song
    playlist.removeSong(2); // Remove "Song 2"
    assert(playlist.getTotalSongs() == 2);
    assert(playlist.playCurrentSong() == "Song 1");

    // Test clearing the playlist
    playlist.clearPlaylist();
    assert(playlist.getTotalSongs() == 0);

    // Test exception handling for empty playlist
    try
    {
        playlist.playCurrentSong();
        assert(false); // Should not reach here
    }
    catch (const std::runtime_error &e)
    {
    }

    try
    {
        playlist.nextSong();
        assert(false); // Should not reach here
    }
    catch (const std::runtime_error &e)
    {
    }
}

// ***** PART 4 ****

template <class ItemType>
class SmartLinkedList : public ListInterface<ItemType>
{
private:
    // TODO: Finish with smart pointers.
    //       Use LinkedList implementation as a guide replacing raw pointers with shared pointers.
    struct Node
    {
        ItemType value;
        std::shared_ptr<Node> next;
        Node(const ItemType &value) : value(value), next(nullptr) {}
    };

    std::shared_ptr<Node> headPtr;
    int itemCount;

    std::shared_ptr<Node> getNodeAt(int position) const
    {
        assert(position >= 1 && position <= itemCount);
        auto curPtr = headPtr;
        for (int skip = 1; skip < position; ++skip)
        {
            curPtr = curPtr->next;
        }
        return curPtr;
    }

public:
    // TODO: Finish with smart pointers.
    SmartLinkedList() : headPtr(nullptr), itemCount(0) {}

    bool isEmpty() const override
    {
        return itemCount == 0;
    }

    int getLength() const override
    {
        return itemCount;
    }

    bool insert(int newPosition, const ItemType &newEntry) override
    {
        bool ableToInsert = (newPosition >= 1) && (newPosition <= itemCount + 1);
        if (ableToInsert)
        {
            auto newNodePtr = std::make_shared<Node>(newEntry);
            if (newPosition == 1)
            {
                newNodePtr->next = headPtr;
                headPtr = newNodePtr;
            }
            else
            {
                auto prevPtr = getNodeAt(newPosition - 1);
                newNodePtr->next = prevPtr->next;
                prevPtr->next = newNodePtr;
            }
            ++itemCount;
        }
        return ableToInsert;
    }

    bool remove(int position) override
    {
        bool ableToRemove = (position >= 1) && (position <= itemCount);
        if (ableToRemove)
        {
            if (position == 1)
            {
                headPtr = headPtr->next;
            }
            else
            {
                auto prevPtr = getNodeAt(position - 1);
                prevPtr->next = prevPtr->next->next;
            }
            --itemCount;
        }
        return ableToRemove;
    }

    void clear() override
    {
        headPtr = nullptr;
        itemCount = 0;
    }

    ItemType getEntry(int position) const override
    {
        bool ableToGet = (position >= 1) && (position <= itemCount);
        if (ableToGet)
        {
            return getNodeAt(position)->value;
        }
        else
        {
            throw std::invalid_argument("getEntry() called with an invalid position.");
        }
    }

    void setEntry(int position, const ItemType &newEntry) override
    {
        auto nodePtr = getNodeAt(position);
        nodePtr->value = newEntry;
    }
}; // end SmartLinkedList

void testSmartLinkedList()
{
    // SmartLinkedList<int> list0;
    //  TODO: Add tests
    SmartLinkedList<int> list0;
    assert(list0.isEmpty());

    // Test inserting elements
    assert(list0.insert(1, 10));
    assert(list0.insert(2, 20));
    assert(list0.insert(3, 30));
    assert(list0.getLength() == 3);
    assert(list0.getEntry(1) == 10);
    assert(list0.getEntry(2) == 20);
    assert(list0.getEntry(3) == 30);

    // Test removing elements
    assert(list0.remove(2));
    assert(list0.getLength() == 2);
    assert(list0.getEntry(1) == 10);
    assert(list0.getEntry(2) == 30);

    // Test setting an entry
    list0.setEntry(2, 40);
    assert(list0.getEntry(2) == 40);

    // Test clearing the list
    list0.clear();
    assert(list0.isEmpty());
}

int main()
{
    std::srand(0);

    testArrayList();
    testLinkedList();
    testLinkedInsertionSort();
    testArrayInsertionSort();
    testSmartLinkedList();
    testPlaylist(); // Test the Playlist ADT

    return 0;
}