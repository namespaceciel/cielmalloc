#ifndef CIELMALLOC_INCLUDE_CIEL_THREAD_ALLOC_H_
#define CIELMALLOC_INCLUDE_CIEL_THREAD_ALLOC_H_

#include <ciel/config.h>
#include <ciel/message_queue.h>
#include <ciel/remote_request.h>
#include <ciel/sizeclass.h>
#include <ciel/span.h>

#include <atomic>
#include <utility>

NAMESPACE_CIEL_BEGIN

// A simple bidirectional list with both head and tail stored inside,
// so that we can eject one node before head or after tail.
//
class span_list {
public:
    span_list() noexcept = default;

    span_list(const span_list&) = delete;
    span_list& operator=(const span_list&) = delete;

    ~span_list();

    void push_before_head(span* ptr) noexcept;

    void push_after_tail(span* ptr) noexcept;

    CIEL_NODISCARD span* eject(span* ptr) noexcept;

    void eject_head_after_tail() noexcept;

    CIEL_NODISCARD bool empty() const noexcept;

private:
    friend class thread_allocator;

    span* head_{nullptr};
    span* tail_{nullptr};

};  // class span_list

static constexpr size_t radix_bits = 6;
static constexpr size_t radix_buckets = 1 << radix_bits;

class thread_allocator {
public:
    CIEL_NODISCARD void* allocate(const size_t) noexcept;

    void deallocate(void*) noexcept;

    CIEL_NODISCARD static thread_allocator& get_instance() noexcept;

    thread_allocator(const thread_allocator&) = delete;
    thread_allocator& operator=(const thread_allocator&) = delete;

private:
    friend class remote_request_list;

    thread_allocator() noexcept = default;

    void process_message_queue() noexcept;

    span_list freelist_[FreelistNum]{};

    // Inspired by radix tree from snmalloc,
    // we hash the low 6 bits of every block's belonged thread_allocator's address.
    // When remote_requests_nums_ goes beyond threshold, send the whole list to their message_queue_.
    remote_request_list remote_requests_[radix_buckets]{};

    message_queue message_queue_;

};  // class thread_allocator

NAMESPACE_CIEL_END

#endif // CIELMALLOC_INCLUDE_CIEL_THREAD_ALLOC_H_