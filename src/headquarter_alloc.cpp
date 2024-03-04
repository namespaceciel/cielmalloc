#include <ciel/headquarter_alloc.h>
#include <ciel/system.h>

NAMESPACE_CIEL_BEGIN

void* headquarter_allocator::allocate() noexcept {
    void* res = stack_.pop();

    if (res == nullptr) {
        return SystemAlloc(1);
    }

    --reserved_span_nums_;
    return res;
}

void headquarter_allocator::deallocate(void* ptr) noexcept {
    if (reserved_span_nums_ >= reserved_span_threshold) {
        SystemFree(ptr);
        return;
    }

    ++reserved_span_nums_;
    stack_.push(ptr);
}

headquarter_allocator& headquarter_allocator::get_instance() noexcept {
    static headquarter_allocator alloc;
    return alloc;
}

headquarter_allocator::~headquarter_allocator() {
    // TODO: How to guarantee that all thread_allocators have destroyed before this destruction?
    // Or do we actually not need to release them since this is the end of the program?
    while (reserved_span_nums_) {
        void* top = stack_.pop();

        CIEL_PRECONDITION(top != nullptr);

        SystemFree(top);
        --reserved_span_nums_;
    }

    CIEL_POSTCONDITION(stack_.pop() == nullptr);
}

NAMESPACE_CIEL_END