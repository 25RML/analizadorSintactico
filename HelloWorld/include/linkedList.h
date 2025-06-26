#pragma once

#include <iostream>

template <typename dataType>
class linkedList {
private:
    class node {
    protected:
        dataType data{};
        node* next{};
        node(dataType data = dataType{}) : data{ data }, next{ nullptr } {}

        friend class linkedList;
    };
    node* head{};
    node* last{};
    std::size_t size{};
public:
    class iterator {
        node* current;
    public:
        iterator(node* ptr) : current(ptr) {}

        dataType& operator*() const { return current->data; }      // Dereference
        dataType* operator->() const { return &(current->data); }  // Arrow operator

        iterator& operator++() { current = current->next; return *this; }   // Prefix ++
        iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; } // Postfix ++

        bool operator==(const iterator& other) const { return current == other.current; }
        bool operator!=(const iterator& other) const { return current != other.current; }
    };
public:
    // ------------------------------------------ CONSTRUCTORS & DESTRUCTORS ------------------------------------------
    linkedList() : head{ nullptr }, last{ nullptr }, size{ 0 } {}
    ~linkedList() {
        node* runner{ head };
        while (runner) {
            node* toErase{ runner };
            runner = runner->next;
            delete(toErase);
        }
    }
    linkedList(const linkedList<dataType>& source) : head{ nullptr }, last{ nullptr }, size{ 0 }
    {
        node* runner{ source.head };
        while (runner) {
            append(runner->data);
            runner = runner->next;
        }
    }
    linkedList(linkedList&& other) noexcept :
        head{ other.head },
        last{ other.last },
        size{ other.size }
    {
        other.head = other.last = nullptr;
        other.size = 0;
    }
    linkedList(std::initializer_list<dataType> listValue) : head{ nullptr }, last{ nullptr }, size{ 0 }
    {
        for (const auto& val : listValue) {
            append(val);
        }
    }
    // ------------------------------------------ OVERLOADED OPERATORS ------------------------------------------
    dataType& operator [](const int index) {
        return getNode(index)->data;
    }
    const dataType& operator [](const int index) const {
        return getNode(index)->data;
    }
    linkedList<dataType>& operator =(const linkedList<dataType>& source) {
        if (this == &source) return *this;
        clear();
        node* runner{ source.head };
        while (runner) {
            append(runner->data);
            runner = runner->next;
        }
        return *this;
    }
    linkedList<dataType>& operator =(linkedList&& other) noexcept {
        if (this != &other) {
            clear();
            head = other.head;
            last = other.last;
            size = other.size;
            other.head = other.last = nullptr;
            other.size = 0;
        }
        return *this;
    }
    // ------------------------------------------ APPEND ------------------------------------------
    dataType& append(const dataType value = dataType{}) {
        if (size == 0) {
            head = new node(value);
            last = head;
            ++size;
            return head->data;
        }
        node* newNode = new node(value);
        last->next = newNode;
        last = newNode;
        ++size;
        return newNode->data;
    }
    void append(const linkedList<dataType>& address)
    {
        node* runner{ address.head };
        while (runner)
        {
            append(runner->data);
            runner = runner->next;
        }
    }
    // ------------------------------------------ APPEND ------------------------------------------
    dataType pop() {
        if (size == 0) throw std::out_of_range("Index out of bounds");
        dataType value = head->data;
        erase(0);
        return value;
    }
    linkedList<dataType> pop(int amount) {
        // Safety Check
        if (amount > static_cast<int>(size)) throw std::out_of_range("Cannot pop more elements than exist in the list");
        // Return a popped linkedList
        linkedList<dataType> poppedList{};
        for (int i{ 0 }; i < amount; ++i) poppedList.append(pop());
        return poppedList;
    }
    // ------------------------------------------ PUSH ------------------------------------------
    void push(const dataType value = dataType{}) {
        node* newNode = new node(value);
        newNode->next = head;
        head = newNode;
        if (size == 0) last = head;
        ++size;
    }
    void push(const linkedList<dataType>& address) {
        pushHelp(address.head);

    }
    // ------------------------------------------ INSERT ------------------------------------------
    void insert(const int index, const dataType value) {
        if (index == 0) {
            this->push(value);
            return;
        }
        node* reference{};
        try {
            reference = getNode(index - 1);
        }
        catch (const std::out_of_range& e) {
            std::cerr << "Out of range error: " << e.what() << std::endl;
            return;
        }
        // Proceed if no problem
        node* newNode = new node(value);
        newNode->next = reference->next;
        reference->next = newNode;
        ++size;
    }
    // ------------------------------------------ ERASE ------------------------------------------
    void erase(const int index) {
        node* reference{};
        if (index == 0) {       // If asked to erase first node
            if (size == 0) throw std::out_of_range("Index out of bounds");
            reference = head;
            head = head->next;
            delete(reference);
            --size;
            if (size == 0) last = head;
            return;
        }
        try {
            if (index >= static_cast<int>(size)) {
                throw std::out_of_range("Index out of bounds");
            }
            reference = getNode(index - 1);
        }
        catch (const std::out_of_range& e) {
            std::cerr << "Out of range error: " << e.what() << std::endl;
            return;
        }
        // Proceed if no problem
        node* toErase = reference->next;
        reference->next = toErase->next;
        delete(toErase);
        --size;
    }
    void clear() {
        node* runner{ head };
        while (runner) {
            node* toErase{ runner };
            runner = runner->next;
            delete(toErase);
        }
        head = nullptr;
        last = nullptr;
        size = 0;
    }
    // ------------------------------------------ FIND ------------------------------------------
#ifdef LINKEDLIST_FIND
    dataType& find(const dataType& search) const {
        node* runner{ head };
        while (runner) {
            if (search == runner->data) break;
            runner = runner->next;
        }
        return runner->data;
    }
#endif // LINKEDLIST_FIND
    std::size_t getSize() const { return size; }
    // ------------------------------------------ MOVE ------------------------------------------
    void move(const int fromIndex, const int toIndex) {
        if (fromIndex == toIndex) return; // No need to move
        if (fromIndex < 0 || toIndex < 0 || fromIndex >= static_cast<int>(size) || toIndex >= static_cast<int>(size)) {
            throw std::out_of_range("Index out of bounds");
        }
        dataType value = getNode(fromIndex)->data;
        erase(fromIndex);
        insert(toIndex, value);
    }
    // ------------------------------------------ BACK ------------------------------------------
    dataType& back() {
        if (!last) throw std::out_of_range("Empty list");
        return last->data;
    }
    const dataType& back() const {
        if (!last) throw std::out_of_range("Empty list");
        return last->data;
    }
    // ------------------------------------------ ITERATORS ------------------------------------------
    iterator begin() const { return iterator(head); }
    iterator end() const { return iterator(nullptr); }
    // ------------------------------------------ DEBUG ------------------------------------------
#ifdef DEBUG_LIST
    void debug() const {
        node* printer{ head };
        //std::cout << '[';
        while (printer) {
            std::cout << ' ';
            std::cout << printer->data;
            printer = printer->next;
            //if (printer) std::cout << ", ";
        }
        //std::cout << " ] Size: " << size;
    }
    void printAll() const {
        node* printer{ head };
        while (printer) {
            std::cout << printer->data;
            printer = printer->next;
        }
    }
    void printReverse() const {
        printReverseHelper(head);
    }
private:
    void printReverseHelper(const node* current) const {
        if (!current) return;
        printReverseHelper(current->next);
        std::cout << current->data;
    }
#endif // !DEBUG_LIST
private:
    // Private Methods (Updaters, etc)
    void pushHelp(const node* current) {
        if (!current) return;
        pushHelp(current->next);
        push(current->data);
    }
    void updateLast(bool lastDelete = false) {
        if (size == 0) {
            last = head;
            return;
        }
        if (!last || lastDelete) last = head;
        while (last->next) last = last->next;
    }
    node* getNode(int index) {
        int sizeL = static_cast<int>(size);
        if (index < 0) {        // Negative Index
            index = sizeL + index;
        }
        if (index >= sizeL || index < 0) {   // Index: Out of Bounds
            throw std::out_of_range("Index out of bounds");
        }
        node* pointer{ head };
        int counter{ 0 };
        while (counter < index) {
            ++counter;
            pointer = pointer->next;
        }
        return pointer;
    }
    node* getNode(int index) const {
        int sizeL = static_cast<int>(size);
        if (index < 0) {        // Negative Index
            index = sizeL + index;
        }
        if (index >= sizeL || index < 0) {   // Index: Out of Bounds
            throw std::out_of_range("Index out of bounds");
        }
        node* pointer{ head };
        int counter{ 0 };
        while (counter < index) {
            ++counter;
            pointer = pointer->next;
        }
        return pointer;
    }
};