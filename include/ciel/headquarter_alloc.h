#ifndef CIELMALLOC_INCLUDE_CIEL_HEADQUARTER_ALLOC_H_
#define CIELMALLOC_INCLUDE_CIEL_HEADQUARTER_ALLOC_H_

#include <ciel/config.h>
#include <ciel/treiber_stack.h>

#include <atomic>

NAMESPACE_CIEL_BEGIN

class span;
class thread_allocator_core;

class headquarter_allocator {
public:
    CIEL_NODISCARD span* allocate() noexcept;

    void deallocate(span*) noexcept;

    CIEL_NODISCARD static headquarter_allocator& get_instance() noexcept;

    headquarter_allocator(const headquarter_allocator&) = delete;
    headquarter_allocator& operator=(const headquarter_allocator&) = delete;

    ~headquarter_allocator();

private:
    friend class thread_allocator;

    headquarter_allocator() noexcept = default;

    CIEL_NODISCARD thread_allocator_core* get_core() noexcept;

    void release_core(thread_allocator_core*) noexcept;

    static constexpr size_t reserved_span_threshold = 8;

    treiber_stack span_stack_;
    treiber_stack thread_allocator_core_stack_;
    std::atomic<size_t> reserved_span_nums_{0};

};  // class headquarter_allocator

NAMESPACE_CIEL_END

#endif // CIELMALLOC_INCLUDE_CIEL_HEADQUARTER_ALLOC_H_