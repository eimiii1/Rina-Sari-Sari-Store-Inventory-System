#ifndef INVENTORY_BST_H
#define INVENTORY_BST_H

#include "Product.h"
#include <string>
#include <vector>

// Binary Search Tree of Products, keyed by product name (case-insensitive).
// Why a BST here: a sari-sari store's product list needs to support
// fast search (is "Coke 1.5L" in stock?), fast insert (new product
// arrives), fast delete (product discontinued), and an easy way to
// list everything in alphabetical order (in-order traversal does
// this for free). A plain array/vector would need O(n) linear scan
// for search, or sorting every time you want an ordered list.
class InventoryBST {
private:
    struct Node {
        Product data;
        Node* left;
        Node* right;
        explicit Node(const Product& p) : data(p), left(nullptr), right(nullptr) {}
    };

    Node* root;
    int count;

    static std::string toLower(const std::string& s);

    Node* insertNode(Node* node, const Product& p);
    Node* findNode(Node* node, const std::string& lowerName) const;
    Node* findMinNode(Node* node) const;
    Node* removeNode(Node* node, const std::string& lowerName, bool& removed);
    void inorderCollect(Node* node, std::vector<Product>& out) const;
    void destroy(Node* node);

public:
    InventoryBST();
    ~InventoryBST();

    // Disable copying — this class owns raw pointers, and a shallow
    // copy would double-free the nodes when both copies are destroyed.
    InventoryBST(const InventoryBST&) = delete;
    InventoryBST& operator=(const InventoryBST&) = delete;

    void insert(const Product& p);
    Product* search(const std::string& name);          // nullptr if not found
    bool remove(const std::string& name);               // true if something was removed
    bool updateStock(const std::string& name, int delta); // delta can be negative (sale) or positive (restock)
    bool updatePricing(const std::string& name, double newCost, double newSelling, int newReorderLevel);

    std::vector<Product> getAllSorted() const;           // in-order traversal = alphabetical
    std::vector<Product> getLowStock() const;

    int size() const { return count; }
};

#endif
