#ifndef SALE_HISTORY_STACK_H
#define SALE_HISTORY_STACK_H

#include "CSVHandler.h"

// Array-based Stack (LIFO) of recorded sales, so "Undo last sale"
// always reverses the most recently recorded transaction — the one a
// cashier is most likely to have just mistyped. Backed by a manually
// grown raw array (capacity doubles when full) rather than std::vector,
// so the push/pop/resize mechanics are explicit rather than hidden
// behind the STL.
class SaleHistoryStack {
private:
    CSVHandler::SaleRecord* data;
    int capacity;
    int count;

    void resize(int newCapacity);

public:
    explicit SaleHistoryStack(int initialCapacity = 8);
    ~SaleHistoryStack();

    // Owns a raw array — disable copying so two stacks never share
    // (and double-free) the same buffer.
    SaleHistoryStack(const SaleHistoryStack&) = delete;
    SaleHistoryStack& operator=(const SaleHistoryStack&) = delete;

    void push(const CSVHandler::SaleRecord& record);
    bool pop(CSVHandler::SaleRecord& out); // false if empty

    bool isEmpty() const { return count == 0; }
    int size() const { return count; }
};

#endif
