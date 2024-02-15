#include <cielmalloc/thread_allocator.hpp>

#include <cstddef>

extern "C" {

CIEL_NODISCARD void* malloc(size_t size) {
    return cielmalloc::thread_allocator::get().allocate(size);
}

void free(void* ptr) {
    if (ptr == nullptr) {
        return;
    }

    cielmalloc::thread_allocator::get().deallocate(ptr);
}

} // extern "C"
