#ifndef CIELMALLOC_INCLUDE_CIEL_SPAN_H_
#define CIELMALLOC_INCLUDE_CIEL_SPAN_H_

#include <ciel/config.h>
#include <ciel/system.h>

#include <atomic>
#include <utility>

NAMESPACE_CIEL_BEGIN

class thread_allocator_core;

// Each span represents a memory block of 64KB in both size and alignment,
// and some metadata is on top, so that every block can find out which span is its owner
// by shifting to top address.
//
// Each span is owned by one thread.
//
struct span {
    // Magic number to indicate that this span is allocated by CielMalloc.
    size_t magic_number_;
    // thread_allocator it belongs to.
    thread_allocator_core* belong_to_;
    // When it becomes 0 again, this span is already a whole block and can be recycled.
    size_t allocated_num_;
    // Each span contains the same size objects.
    size_t obj_size_;
    // A forward linklist of free blocks of obj_size_.
    // freed3 -> freed2 -> freed1 -> high-water mark
    void* free_;
    // Used as bidirectional linklist in span_list.
    span* prev_;
    span* next_;

    enum class DeallocatedResult {
        // Nothing happens.
        None,
        // If this was run out of capacity, and has one new freed node now,
        // we insert it back to head.
        BackFromZero,
        // This span is release-able.
        BackToOnePiece

    };  // enum class DeallocatedResult

    void init(const size_t, thread_allocator_core*) noexcept;

    CIEL_NODISCARD void* allocate() noexcept;

    // Should call empty() everytime after calling allocate().
    CIEL_NODISCARD bool empty() const noexcept;

    CIEL_NODISCARD DeallocatedResult deallocate(void*) noexcept;

    // This is a callback when thread died.
    // Since it won't use free_ blocks anymore, we just decrement the allocated_num_,
    // and when it returns to zero, pass this span to headquarter_allocator.
    void deallocate_after_thread_died() noexcept;

};  // struct span

CIEL_NODISCARD span* get_span(void*) noexcept;

NAMESPACE_CIEL_END

#endif // CIELMALLOC_INCLUDE_CIEL_SPAN_H_