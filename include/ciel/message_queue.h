#ifndef CIELMALLOC_INCLUDE_CIEL_MESSAGE_QUEUE_H_
#define CIELMALLOC_INCLUDE_CIEL_MESSAGE_QUEUE_H_

#include <ciel/config.h>
#include <ciel/remote_request.h>
#include <ciel/system.h>

#include <atomic>

NAMESPACE_CIEL_BEGIN

// A simple lock-free queue based on the Pony language runtime message queue.
// It allows multiple producers and a single consumer.
//
class message_queue {
public:
    CIEL_NODISCARD void* dequeue() noexcept;

    // enqueue only modifies end_ and is accessed by multiple threads.
    void enqueue(void*, void*) noexcept;

    // Reset dummy_head_ to be both begin_ and end_, to free the stuck begin_ node.
    // Should be called after dequeue() == nullptr.
    void clear() noexcept;

private:
    // Dummy-head, this message queue is always not-empty.
    void* dummy_head_{nullptr};
    std::atomic<void*> begin_{&dummy_head_};
    std::atomic<void*> end_{&dummy_head_};

};  // class message_queue

NAMESPACE_CIEL_END

#endif // CIELMALLOC_INCLUDE_CIEL_MESSAGE_QUEUE_H_