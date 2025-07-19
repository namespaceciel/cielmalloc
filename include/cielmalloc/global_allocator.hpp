#ifndef CIELMALLOC_GLOBAL_ALLOCATOR_HPP_
#define CIELMALLOC_GLOBAL_ALLOCATOR_HPP_

#include <ciel/core/array.hpp>
#include <ciel/core/config.hpp>
#include <ciel/core/message.hpp>
#include <ciel/core/treiber_stack.hpp>
#include <cielmalloc/large_slab.hpp>
#include <cielmalloc/pagemap.hpp>
#include <cielmalloc/pal.hpp>
#include <cielmalloc/sizeclass.hpp>

#include <cstddef>

namespace cielmalloc {

class global_allocator {
private:
    ciel::array<ciel::treiber_stack<large_slab>, NumLargeClassesRange.first, NumLargeClassesRange.second> large_stack_;

public:
    global_allocator()                                   = default;
    global_allocator(const global_allocator&)            = delete;
    global_allocator& operator=(const global_allocator&) = delete;

    CIEL_NODISCARD void* allocate(size_t size) noexcept {
        const uint8_t sizeclass = cielmalloc::next_pow2_bits(size);
        size                    = cielmalloc::one_at_bit(sizeclass);

        void* res = large_stack_[sizeclass].pop();
        if (res == nullptr) {
            res = pal::reserve<true>(size);
            global_pagemap.store(res, static_cast<slab_kind>(sizeclass));
        }

        return res;
    }

    void deallocate(void* p) noexcept {
        deallocate(p, global_pagemap.load(p));
    }

    void deallocate(void* p, const slab_kind kind) noexcept {
        CIEL_ASSERT(global_pagemap.load(p) == kind);

        const uint8_t sizeclass = static_cast<uint8_t>(kind);

        // Save one page to be used as treiber_stack's node.
        pal::decommit(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) + pal::page_size),
                      cielmalloc::one_at_bit(sizeclass) - pal::page_size);

        large_stack_[sizeclass].push(static_cast<large_slab*>(p));
    }

}; // class global_allocator

inline global_allocator global_alloc;

} // namespace cielmalloc

#endif // CIELMALLOC_GLOBAL_ALLOCATOR_HPP_
