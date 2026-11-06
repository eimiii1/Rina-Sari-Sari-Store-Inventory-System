#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "Product.h"
#include "CSVHandler.h"
#include <vector>
#include <string>

// The four algorithms the course asks for, each attached to a feature
// that actually needs that specific one — not four interchangeable
// ways to do the same thing.
namespace Algorithms {

    // Linear Search — O(n). Scans every sale in order and collects every
    // match. The sales log is stored chronologically, not sorted by
    // product name, so a straight scan is the honest choice here (and
    // "first match only" wouldn't be useful — a product can be sold
    // many times).
    std::vector<CSVHandler::SaleRecord> linearSearchSales(
        const std::vector<CSVHandler::SaleRecord>& sales,
        const std::string& productName);

    // Binary Search — O(log n). Requires `sortedProducts` to already be
    // sorted by name (InventoryBST's in-order traversal guarantees
    // that). Returns a pointer into that same vector, or nullptr — so
    // the caller must keep the vector alive while using the result.
    const Product* binarySearchProduct(
        const std::vector<Product>& sortedProducts,
        const std::string& name);

    // Bubble Sort — O(n^2). Fine here because a store's low-stock list
    // is always small. Sorts ascending by quantity in place, so the
    // most urgently-needed restock shows up first.
    void bubbleSortByQuantity(std::vector<Product>& products);

    // Merge Sort — O(n log n), divide and conquer. Sorts descending by
    // sale total in place — chosen over bubble sort here because the
    // full sales log can grow large over time, where O(n^2) would start
    // to matter.
    void mergeSortByTotal(std::vector<CSVHandler::SaleRecord>& sales);

} // namespace Algorithms

#endif
