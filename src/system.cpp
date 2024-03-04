#include <ciel/system.h>

NAMESPACE_CIEL_BEGIN

void*& ptr_next(void* ptr) noexcept {
    return *static_cast<void**>(ptr);
}

// Is a pointer aligned?
bool is_aligned(void* ptr, const size_t alignment) noexcept {
    CIEL_PRECONDITION(ptr != nullptr);
    CIEL_PRECONDITION(alignment != 0);

    return ((uintptr_t)ptr % alignment) == 0;
}

// Align upwards
uintptr_t align_up(uintptr_t sz, const size_t alignment) noexcept {
    CIEL_PRECONDITION(alignment != 0);

    const uintptr_t mask = alignment - 1;

    if CIEL_LIKELY((alignment & mask) == 0) {  // power of two?
        return (sz + mask) & ~mask;

    } else {
        return ((sz + mask) / alignment) * alignment;
    }
}

// Align downwards
uintptr_t align_down(uintptr_t sz, const size_t alignment) noexcept {
    CIEL_PRECONDITION(alignment != 0);

    uintptr_t mask = alignment - 1;

    if CIEL_LIKELY((alignment & mask) == 0) { // power of two?
        return (sz & ~mask);

    } else {
        return ((sz / alignment) * alignment);
    }
}

void* SystemAlloc(size_t page_num) noexcept {
    void* ptr = VirtualAlloc(0, page_num << PAGE_SHIFT, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    CIEL_POSTCONDITION(ptr != nullptr);

    return ptr;
}

void SystemFree(void* ptr) noexcept {
    CIEL_PRECONDITION(ptr != nullptr);

    VirtualFree(ptr, 0, MEM_RELEASE);
}

NAMESPACE_CIEL_END