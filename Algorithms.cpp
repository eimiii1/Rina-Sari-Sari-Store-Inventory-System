#include "Algorithms.h"
#include <algorithm>
#include <cctype>

namespace {
    std::string toLower(const std::string& s) {
        std::string result = s;
        std::transform(result.begin(), result.end(), result.begin(),
                        [](unsigned char c) { return std::tolower(c); });
        return result;
    }
}

namespace Algorithms {

std::vector<CSVHandler::SaleRecord> linearSearchSales(
    const std::vector<CSVHandler::SaleRecord>& sales,
    const std::string& productName) {

    std::vector<CSVHandler::SaleRecord> matches;
    std::string target = toLower(productName);

    // The defining trait of linear search: check every element, one at
    // a time, front to back, with no assumption that the data is sorted.
    for (size_t i = 0; i < sales.size(); i++) {
        if (toLower(sales[i].productName) == target) {
            matches.push_back(sales[i]);
        }
    }
    return matches;
}

const Product* binarySearchProduct(
    const std::vector<Product>& sortedProducts,
    const std::string& name) {

    std::string target = toLower(name);
    int low = 0;
    int high = static_cast<int>(sortedProducts.size()) - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        std::string midName = toLower(sortedProducts[mid].name);

        if (midName == target) {
            return &sortedProducts[mid];
        } else if (midName < target) {
            low = mid + 1;   // target is alphabetically after mid — discard the left half
        } else {
            high = mid - 1;  // target is alphabetically before mid — discard the right half
        }
    }
    return nullptr; // exhausted the search space without a match
}

void bubbleSortByQuantity(std::vector<Product>& products) {
    size_t n = products.size();
    for (size_t i = 0; i < n; i++) {
        bool swappedThisPass = false;
        // Each pass bubbles the largest remaining quantity to the end;
        // shrinking the inner bound by i avoids re-comparing elements
        // already known to be in their final position.
        for (size_t j = 0; j + 1 < n - i; j++) {
            if (products[j].quantity > products[j + 1].quantity) {
                std::swap(products[j], products[j + 1]);
                swappedThisPass = true;
            }
        }
        if (!swappedThisPass) break; // already sorted — nothing left to do
    }
}

namespace {
    // Merges two already-sorted slices arr[left..mid] and arr[mid+1..right]
    // back into arr, descending by total. Needs temporary copies of both
    // halves since the merge writes into the same range it reads from.
    void merge(std::vector<CSVHandler::SaleRecord>& arr, int left, int mid, int right) {
        std::vector<CSVHandler::SaleRecord> leftHalf(arr.begin() + left, arr.begin() + mid + 1);
        std::vector<CSVHandler::SaleRecord> rightHalf(arr.begin() + mid + 1, arr.begin() + right + 1);

        size_t i = 0, j = 0;
        int k = left;
        while (i < leftHalf.size() && j < rightHalf.size()) {
            if (leftHalf[i].total >= rightHalf[j].total) { // descending: bigger total goes first
                arr[k++] = leftHalf[i++];
            } else {
                arr[k++] = rightHalf[j++];
            }
        }
        while (i < leftHalf.size()) arr[k++] = leftHalf[i++];
        while (j < rightHalf.size()) arr[k++] = rightHalf[j++];
    }

    void mergeSortRange(std::vector<CSVHandler::SaleRecord>& arr, int left, int right) {
        if (left >= right) return; // 0 or 1 element — already sorted
        int mid = left + (right - left) / 2;
        mergeSortRange(arr, left, mid);       // sort left half
        mergeSortRange(arr, mid + 1, right);  // sort right half
        merge(arr, left, mid, right);         // combine the two sorted halves
    }
}

void mergeSortByTotal(std::vector<CSVHandler::SaleRecord>& sales) {
    if (sales.empty()) return;
    mergeSortRange(sales, 0, static_cast<int>(sales.size()) - 1);
}

} // namespace Algorithms
