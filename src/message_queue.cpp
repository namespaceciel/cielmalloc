#include <ciel/message_queue.h>

NAMESPACE_CIEL_BEGIN

void* message_queue::dequeue() noexcept {
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
    // TODO: Acquire fence
    return first;
}

void message_queue::enqueue(void* first, void* last) noexcept {
    ptr_next(last) = nullptr;
    // TODO: Release fence
    void* prev = end_.exchange(last);
    ptr_next(prev) = first;
}

NAMESPACE_CIEL_END