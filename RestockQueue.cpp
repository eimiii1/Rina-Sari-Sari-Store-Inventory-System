#include "RestockQueue.h"

RestockQueue::RestockQueue() : front_(nullptr), rear_(nullptr), count(0) {}

RestockQueue::~RestockQueue() {
    Node* cur = front_;
    while (cur) {
        Node* next = cur->next;
        delete cur;
        cur = next;
    }
}

void RestockQueue::enqueue(const RestockRequest& request) {
    Node* node = new Node(request);
    if (!rear_) {
        front_ = rear_ = node;
    } else {
        rear_->next = node;
        rear_ = node;
    }
    count++;
}

bool RestockQueue::dequeue(RestockRequest& out) {
    if (!front_) return false;
    Node* old = front_;
    out = old->data;
    front_ = front_->next;
    if (!front_) rear_ = nullptr; // queue just became empty
    delete old;
    count--;
    return true;
}

bool RestockQueue::contains(const std::string& productName) const {
    Node* cur = front_;
    while (cur) {
        if (cur->data.productName == productName) return true;
        cur = cur->next;
    }
    return false;
}

// Removes every node matching productName (there should only ever be
// one, since enqueue() is only called after a contains() check, but
// this stays correct even if that invariant is ever broken). Has to
// carefully re-link around the removed node(s) and keep rear_ pointing
// somewhere valid if the last node happens to be the one removed.
bool RestockQueue::removeByName(const std::string& productName) {
    Node* prev = nullptr;
    Node* cur = front_;
    bool removedAny = false;

    while (cur) {
        if (cur->data.productName == productName) {
            Node* toDelete = cur;
            if (prev) prev->next = cur->next; else front_ = cur->next;
            if (cur == rear_) rear_ = prev;
            cur = cur->next;
            delete toDelete;
            count--;
            removedAny = true;
        } else {
            prev = cur;
            cur = cur->next;
        }
    }
    return removedAny;
}

std::vector<RestockRequest> RestockQueue::listAll() const {
    std::vector<RestockRequest> result;
    Node* cur = front_;
    while (cur) {
        result.push_back(cur->data);
        cur = cur->next;
    }
    return result;
}
