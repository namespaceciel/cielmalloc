#include <ciel/headquarter_alloc.h>
#include <ciel/system.h>
#include <ciel/thread_alloc_core.h>

#include <mutex>

NAMESPACE_CIEL_BEGIN

CIEL_NODISCARD span* headquarter_allocator::allocate() noexcept {
    span* res = static_cast<span*>(span_stack_.pop());

    if (res == nullptr) {
        return static_cast<span*>(SystemAlloc(1));
    }

    --reserved_span_nums_;

    return res;
}

void headquarter_allocator::deallocate(span* ptr) noexcept {
    if (reserved_span_nums_ >= reserved_span_threshold) {
        SystemFree(ptr);
        return;
    }

    ++reserved_span_nums_;
    span_stack_.push(ptr);
}

std::once_flag headquarter_allocator_construct_once_flag;
CIEL_NODISCARD headquarter_allocator& headquarter_allocator::get_instance() noexcept {
    static std::aligned_storage<sizeof(headquarter_allocator), alignof(headquarter_allocator)> storage;

    std::call_once(headquarter_allocator_construct_once_flag, []{ ::new (&storage) headquarter_allocator{}; });

    return *static_cast<headquarter_allocator*>(static_cast<void*>(&storage));
}

headquarter_allocator::~headquarter_allocator() {
    thread_allocator_core* tac;
    while ((tac = static_cast<thread_allocator_core*>(thread_allocator_core_stack_.pop())) != nullptr) {
        tac->process_message_queue_unlinked();

        HeapFree(GetProcessHeap(), 0, tac);
    }

    while (reserved_span_nums_) {
        void* top = span_stack_.pop();

        CIEL_PRECONDITION(top != nullptr);

        SystemFree(top);
        --reserved_span_nums_;
    }

    CIEL_POSTCONDITION(span_stack_.pop() == nullptr);
    CIEL_POSTCONDITION(thread_allocator_core_stack_.pop() == nullptr);
}

CIEL_NODISCARD thread_allocator_core* headquarter_allocator::get_core() noexcept {
    thread_allocator_core* res = static_cast<thread_allocator_core*>(thread_allocator_core_stack_.pop());

    if (res == nullptr) {
        res = static_cast<thread_allocator_core*>(HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(thread_allocator_core)));

        CIEL_POSTCONDITION(res != nullptr);

        ::new (static_cast<void*>(res)) thread_allocator_core{};
    }

    return res;
}

void headquarter_allocator::release_core(thread_allocator_core* ptr) noexcept {
    thread_allocator_core_stack_.push(ptr);
}

NAMESPACE_CIEL_END