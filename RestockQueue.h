#ifndef RESTOCK_QUEUE_H
#define RESTOCK_QUEUE_H

#include <string>
#include <vector>

struct RestockRequest {
    std::string productName;
    std::string requestedAt;
};

// Classic linked-list-based Queue (FIFO). Whenever a product's stock
// drops to or below its reorder level, it gets enqueued here as a
// restock request. Requests are handled oldest-first — the store
// should restock whatever ran low first, not whatever just dipped
// low a minute ago.
class RestockQueue {
private:
    struct Node {
        RestockRequest data;
        Node* next;
        explicit Node(const RestockRequest& r) : data(r), next(nullptr) {}
    };

    Node* front_;
    Node* rear_;
    int count;

public:
    RestockQueue();
    ~RestockQueue();

    // Owns raw pointers — disable copying so two queues never end up
    // double-freeing the same nodes.
    RestockQueue(const RestockQueue&) = delete;
    RestockQueue& operator=(const RestockQueue&) = delete;

    void enqueue(const RestockRequest& request);
    bool dequeue(RestockRequest& out);                    // false if empty
    bool contains(const std::string& productName) const;
    bool removeByName(const std::string& productName);    // true if something was removed

    bool isEmpty() const { return front_ == nullptr; }
    int size() const { return count; }

    std::vector<RestockRequest> listAll() const; // front-to-back, non-destructive
};

#endif
