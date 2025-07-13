#ifndef CIELMALLOC_INTERFACE_HPP_
#define CIELMALLOC_INTERFACE_HPP_

#include <cielmalloc/thread_allocator.hpp>

#include <cstddef>

namespace cielmalloc {

CIEL_NODISCARD inline void* malloc(size_t size) noexcept {
    // Include size 0 in the first sizeclass.
    size = ((size - 1) >> (Bits - 1)) + size;
    return cielmalloc::thread_allocator::get().allocate(size);
}

inline void free(void* ptr) noexcept {
    if CIEL_UNLIKELY (ptr == nullptr) {
        return;
    }

    cielmalloc::thread_allocator::get().deallocate(ptr);
}

} // namespace cielmalloc

#endif // CIELMALLOC_INTERFACE_HPP_
