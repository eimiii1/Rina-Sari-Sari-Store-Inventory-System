#include "InventoryBST.h"
#include <algorithm>
#include <cctype>

InventoryBST::InventoryBST() : root(nullptr), count(0) {}

InventoryBST::~InventoryBST() {
    destroy(root);
}

void InventoryBST::destroy(Node* node) {
    if (!node) return;
    destroy(node->left);
    destroy(node->right);
    delete node;
}

std::string InventoryBST::toLower(const std::string& s) {
    std::string result = s;
    std::transform(result.begin(), result.end(), result.begin(),
                    [](unsigned char c) { return std::tolower(c); });
    return result;
}

// --- insert -----------------------------------------------------------
// Standard BST insert: go left if the new key is smaller, right if
// bigger, and place the new node when we fall off the tree.
InventoryBST::Node* InventoryBST::insertNode(Node* node, const Product& p) {
    if (!node) {
        count++;
        return new Node(p);
    }
    std::string key = toLower(p.name);
    std::string nodeKey = toLower(node->data.name);

    if (key < nodeKey) {
        node->left = insertNode(node->left, p);
    } else if (key > nodeKey) {
        node->right = insertNode(node->right, p);
    } else {
        // Same name already exists — treat as an update rather than
        // inserting a duplicate node (keeps names unique in the tree).
        node->data = p;
    }
    return node;
}

void InventoryBST::insert(const Product& p) {
    root = insertNode(root, p);
}

// --- search -------------------------------------------------------------
InventoryBST::Node* InventoryBST::findNode(Node* node, const std::string& lowerName) const {
    if (!node) return nullptr;
    std::string nodeKey = toLower(node->data.name);
    if (lowerName == nodeKey) return node;
    if (lowerName < nodeKey) return findNode(node->left, lowerName);
    return findNode(node->right, lowerName);
}

Product* InventoryBST::search(const std::string& name) {
    Node* found = findNode(root, toLower(name));
    return found ? &found->data : nullptr;
}

// --- remove -------------------------------------------------------------
InventoryBST::Node* InventoryBST::findMinNode(Node* node) const {
    while (node && node->left) node = node->left;
    return node;
}

InventoryBST::Node* InventoryBST::removeNode(Node* node, const std::string& lowerName, bool& removed) {
    if (!node) return nullptr;

    std::string nodeKey = toLower(node->data.name);
    if (lowerName < nodeKey) {
        node->left = removeNode(node->left, lowerName, removed);
    } else if (lowerName > nodeKey) {
        node->right = removeNode(node->right, lowerName, removed);
    } else {
        // Found the node to remove.
        removed = true;
        count--;

        if (!node->left && !node->right) {
            delete node;
            return nullptr;
        }
        if (!node->left) {
            Node* right = node->right;
            delete node;
            return right;
        }
        if (!node->right) {
            Node* left = node->left;
            delete node;
            return left;
        }
        // Two children: replace this node's data with its in-order
        // successor (smallest value in the right subtree), then
        // delete that successor node from the right subtree instead.
        Node* successor = findMinNode(node->right);
        node->data = successor->data;
        bool dummy = false;
        node->right = removeNode(node->right, toLower(successor->data.name), dummy);
        count++; // the recursive call above already decremented count; undo the double-count
    }
    return node;
}

bool InventoryBST::remove(const std::string& name) {
    bool removed = false;
    root = removeNode(root, toLower(name), removed);
    return removed;
}

// --- mutations ------------------------------------------------------------
bool InventoryBST::updateStock(const std::string& name, int delta) {
    Product* p = search(name);
    if (!p) return false;
    if (p->quantity + delta < 0) return false; // can't go negative
    p->quantity += delta;
    return true;
}

bool InventoryBST::updatePricing(const std::string& name, double newCost, double newSelling, int newReorderLevel) {
    Product* p = search(name);
    if (!p) return false;
    p->costPrice = newCost;
    p->sellingPrice = newSelling;
    p->reorderLevel = newReorderLevel;
    return true;
}

// --- traversal ------------------------------------------------------------
void InventoryBST::inorderCollect(Node* node, std::vector<Product>& out) const {
    if (!node) return;
    inorderCollect(node->left, out);
    out.push_back(node->data);
    inorderCollect(node->right, out);
}

std::vector<Product> InventoryBST::getAllSorted() const {
    std::vector<Product> result;
    inorderCollect(root, result);
    return result;
}

std::vector<Product> InventoryBST::getLowStock() const {
    std::vector<Product> all = getAllSorted();
    std::vector<Product> low;
    for (const auto& p : all) {
        if (p.isLowStock()) low.push_back(p);
    }
    return low;
}
