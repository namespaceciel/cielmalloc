#ifndef CTZ_CIELMALLOC_INCLUDE_CIEL_THREAD_ALLOC_CORE_H_
#define CTZ_CIELMALLOC_INCLUDE_CIEL_THREAD_ALLOC_CORE_H_

#include <ciel/config.h>
#include <ciel/message_queue.h>

NAMESPACE_CIEL_BEGIN

class thread_allocator_core {
public:
    // Only be called by headquarter_allocator when program ends.
    void process_message_queue_unlinked() noexcept;

private:
    friend class headquarter_allocator;
    friend class thread_allocator;
    friend class remote_request_list;

    // Used as intrusive linklist next in treiber_stack
    void* dummy_;

    message_queue message_queue_;

};  // class thread_allocator_core

NAMESPACE_CIEL_END

#endif // CTZ_CIELMALLOC_INCLUDE_CIEL_THREAD_ALLOC_CORE_H_