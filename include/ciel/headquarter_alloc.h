#ifndef CIELMALLOC_INCLUDE_CIEL_HEADQUARTER_ALLOC_H_
#define CIELMALLOC_INCLUDE_CIEL_HEADQUARTER_ALLOC_H_

#include <ciel/config.h>
#include <ciel/treiber_stack.h>

#include <atomic>

NAMESPACE_CIEL_BEGIN

class headquarter_allocator {
public:
    void* allocate() noexcept;

    void deallocate(void*) noexcept;

    static headquarter_allocator& get_instance() noexcept;

    headquarter_allocator(const headquarter_allocator&) = delete;
    headquarter_allocator& operator=(const headquarter_allocator&) = delete;

    ~headquarter_allocator();

private:
    headquarter_allocator() noexcept = default;

    static constexpr size_t reserved_span_threshold = 8;

    treiber_stack stack_;
    std::atomic<size_t> reserved_span_nums_{0};

};  // class headquarter_allocator

NAMESPACE_CIEL_END

#endif // CIELMALLOC_INCLUDE_CIEL_HEADQUARTER_ALLOC_H_