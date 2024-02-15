#ifndef CIELMALLOC_INCLUDE_CIELMALLOC_GLOBAL_ALLOCATOR_HPP_
#define CIELMALLOC_INCLUDE_CIELMALLOC_GLOBAL_ALLOCATOR_HPP_

#include <ciel/core/config.hpp>
#include <ciel/core/message.hpp>
#include <ciel/core/treiber_stack.hpp>
#include <cielmalloc/large_slab.hpp>
#include <cielmalloc/pagemap.hpp>
#include <cielmalloc/pal.hpp>
#include <cielmalloc/sizeclass.hpp>

#include <array>
#include <cstddef>

namespace cielmalloc {

class global_allocator {
private:
    std::array<ciel::treiber_stack<large_slab>, NumLargeClasses> large_stack_;

public:
    global_allocator()                                   = default;
    global_allocator(const global_allocator&)            = delete;
    global_allocator& operator=(const global_allocator&) = delete;

    CIEL_NODISCARD void* allocate(const size_t size) noexcept {
        const uint8_t size_bits = cielmalloc::next_pow2_bits(size);
        CIEL_ASSERT(size_bits >= LargeThresholdBits);

        const uint8_t large_sizeclass = size_bits - LargeThresholdBits;
        CIEL_ASSERT(large_sizeclass < NumLargeClasses);

        void* res = large_stack_[large_sizeclass].pop();
        if (res == nullptr) {
            res = pal::reserve(cielmalloc::one_at_bit(size_bits));
            global_pagemap.store(res, static_cast<slab_kind>(large_sizeclass + static_cast<uint8_t>(slab_kind::Large)));
        }

        pal::commit(res, size);

        return res;
    }

    void deallocate(void* p) noexcept {
        deallocate(p, global_pagemap.load(p));
    }

    void deallocate(void* p, const slab_kind kind) noexcept {
        CIEL_ASSERT(global_pagemap.load(p) == kind);

        const uint8_t large_sizeclass = static_cast<uint8_t>(kind) - static_cast<uint8_t>(slab_kind::Large);
        const uint8_t size_bits       = large_sizeclass + LargeThresholdBits;

        CIEL_ASSERT(large_sizeclass < NumLargeClasses);

        // Save one page to be used as treiber_stack's node.
        pal::decommit(reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(p) + OSPageSize),
                      cielmalloc::one_at_bit(size_bits) - OSPageSize);

        large_stack_[large_sizeclass].push(static_cast<large_slab*>(p));
    }

}; // class global_allocator

inline global_allocator global_alloc;

} // namespace cielmalloc

#endif // CIELMALLOC_INCLUDE_CIELMALLOC_GLOBAL_ALLOCATOR_HPP_
