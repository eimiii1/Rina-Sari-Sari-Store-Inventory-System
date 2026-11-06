#include "SaleHistoryStack.h"

SaleHistoryStack::SaleHistoryStack(int initialCapacity)
    : data(new CSVHandler::SaleRecord[initialCapacity]), capacity(initialCapacity), count(0) {}

SaleHistoryStack::~SaleHistoryStack() {
    delete[] data;
}

void SaleHistoryStack::resize(int newCapacity) {
    CSVHandler::SaleRecord* bigger = new CSVHandler::SaleRecord[newCapacity];
    for (int i = 0; i < count; i++) bigger[i] = data[i];
    delete[] data;
    data = bigger;
    capacity = newCapacity;
}

void SaleHistoryStack::push(const CSVHandler::SaleRecord& record) {
    if (count == capacity) resize(capacity * 2);
    data[count] = record;
    count++;
}

bool SaleHistoryStack::pop(CSVHandler::SaleRecord& out) {
    if (count == 0) return false;
    count--;
    out = data[count];
    return true;
}
