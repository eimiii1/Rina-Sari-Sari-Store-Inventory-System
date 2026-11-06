#ifndef CSV_HANDLER_H
#define CSV_HANDLER_H

#include "Product.h"
#include <string>
#include <vector>

namespace CSVHandler {
    // Loads products from a CSV file. Returns an empty vector (and prints
    // a notice) if the file doesn't exist yet — that's expected on first run.
    std::vector<Product> loadInventory(const std::string& filename);

    // Overwrites the file with the given product list (one row per product).
    void saveInventory(const std::string& filename, const std::vector<Product>& products);

    // Appends one line per sale transaction to a separate sales log.
    // unitCost is recorded too so sales reports can compute profit later
    // without having to go look up the (possibly since-changed) product price.
    void logSale(const std::string& filename, const std::string& timestamp,
                 const std::string& productName, int quantity,
                 double unitCost, double unitPrice, double total);

    struct SaleRecord {
        std::string timestamp;
        std::string productName;
        int quantity;
        double unitCost;
        double unitPrice;
        double total;
    };

    // Reads the full sales log for reporting (e.g. today's/this week's totals).
    std::vector<SaleRecord> loadSales(const std::string& filename);

    // Drops the most recently appended sale (used by "undo last sale").
    // Rewrites the whole file minus its last row — safe to call on an
    // empty/missing file, in which case it's a no-op.
    void removeLastSale(const std::string& filename);
}

#endif
