#include <ciel/span.h>
#include <ciel/thread_alloc_core.h>

NAMESPACE_CIEL_BEGIN

void thread_allocator_core::process_message_queue_unlinked() noexcept {
    void* request;

    while ((request = message_queue_.dequeue()) != nullptr) {
        span* s = get_span(request);

        CIEL_PRECONDITION(s->magic_number_ == MagicNumber);

        s->deallocate_after_thread_died();
    }

    // Deal with stuck head_.
    message_queue_.clear();
    if ((request = message_queue_.dequeue()) != nullptr) {
        span* s = get_span(request);

        CIEL_PRECONDITION(s->magic_number_ == MagicNumber);

        s->deallocate_after_thread_died();
    }

    CIEL_POSTCONDITION(message_queue_.dequeue() == nullptr);
}

NAMESPACE_CIEL_END