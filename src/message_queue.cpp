#include <ciel/message_queue.h>

NAMESPACE_CIEL_BEGIN

CIEL_NODISCARD void* message_queue::dequeue() noexcept {
    // Only if size > 1 can it dequeue.
    if (ptr_next(begin_) == nullptr) {
        return nullptr;

    } else if CIEL_UNLIKELY(begin_ == &dummy_head_) {
        // Remove dummy_head_ and try again.
        begin_ = ptr_next(begin_);
        return dequeue();
    }

    void* first = begin_;
    begin_ = ptr_next(begin_);

    return first;
}

void message_queue::enqueue(void* first, void* last) noexcept {
    ptr_next(last) = nullptr;

    void* prev = end_.exchange(last);

    ptr_next(prev) = first;
}

void message_queue::clear() noexcept {
    CIEL_PRECONDITION(dequeue() == nullptr);

    if (begin_ == &dummy_head_) {
        return;
    }

    enqueue(&dummy_head_, &dummy_head_);
}

NAMESPACE_CIEL_END