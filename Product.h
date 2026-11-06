#ifndef PRODUCT_H
#define PRODUCT_H

#include <string>

// One row of the inventory. "code" is a short unique id you assign
// (e.g. "P001") — useful for receipts/reports even though the BST
// itself is keyed by name.
struct Product {
    std::string code;
    std::string name;
    std::string category;
    std::string unit;          // pc, pack, bottle, sachet, etc.
    double costPrice;          // what you paid the supplier
    double sellingPrice;       // what you charge the customer
    int quantity;
    int reorderLevel;          // restock warning threshold

    double margin() const {
        return sellingPrice - costPrice;
    }

    bool isLowStock() const {
        return quantity <= reorderLevel;
    }
};

#endif
