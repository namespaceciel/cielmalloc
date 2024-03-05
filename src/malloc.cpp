#include <ciel/malloc.h>
#include <ciel/thread_alloc.h>

#include <new>

CIEL_NODISCARD void* operator new(size_t size) {
    if CIEL_UNLIKELY(size == 0) {
        ++size;
    }

    void* ptr = ciel::malloc(size);

    if CIEL_UNLIKELY(ptr == nullptr) {
        ciel::THROW(std::bad_alloc{});
    }

    return ptr;
}

CIEL_NODISCARD void* operator new[](size_t size) {
    if CIEL_UNLIKELY(size == 0) {
        ++size;
    }

    void* ptr = ciel::malloc(size);

    if CIEL_UNLIKELY(ptr == nullptr) {
        ciel::THROW(std::bad_alloc{});
    }

    return ptr;
}

void operator delete(void* ptr) noexcept {
    ciel::free(ptr);
}

void operator delete[](void* ptr) noexcept {
    ciel::free(ptr);
}

NAMESPACE_CIEL_BEGIN

CIEL_NODISCARD void* malloc(const size_t size) noexcept {
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