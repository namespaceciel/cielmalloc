#include <ciel/malloc.h>
#include <ciel/thread_alloc.h>

NAMESPACE_CIEL_BEGIN

void* malloc(const size_t size) noexcept {
    if (size == 0) {
        return nullptr;
    }

    return thread_allocator::get_instance().allocate(size);
}

void free(void* ptr) noexcept {
    if (ptr == nullptr) {
        return;
    }

    span* s = get_span(ptr);

    if (s->magic_number_ == MagicNumber) {
        thread_allocator::get_instance().deallocate(ptr);

    } else {
        SystemFree(ptr);
    }
}

NAMESPACE_CIEL_END