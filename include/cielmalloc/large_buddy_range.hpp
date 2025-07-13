#ifndef CIELMALLOC_LARGE_BUDDY_RANGE_HPP_
#define CIELMALLOC_LARGE_BUDDY_RANGE_HPP_

#include <ciel/core/config.hpp>
#include <cielmalloc/bits.hpp>
#include <cielmalloc/buddy.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/pagemap.hpp>
#include <cielmalloc/pal.hpp>

#include <cstddef>
#include <utility>

namespace cielmalloc {

struct large_buddy_range {
    template<class ParentRange>
    class type : public ParentRange {
    private:
        using Node                        = RBNodePagemap;
        static constexpr size_t BitBegin  = pal::page_size_bits;
        static constexpr size_t BitEnd    = AddressBits;
        static constexpr size_t BitRefill = 24;

        inline static buddy<Node, BitBegin, BitEnd> buddy_;

    public:
        static constexpr size_t MinBlockSize = cielmalloc::one_at_bit(BitBegin);
        static constexpr size_t MaxBlockSize = cielmalloc::one_at_bit(BitEnd);
        static constexpr size_t RefillSize   = cielmalloc::one_at_bit(BitRefill);

        using Base = ParentRange;

        CIEL_NODISCARD void* alloc_range(size_t size) noexcept {
            CIELMALLOC_LOG("In large_buddy_range::alloc_range, size: {}", size);

            CIEL_ASSERT(ciel::is_pow2(size));
            CIEL_ASSERT(MinBlockSize <= size);
            CIEL_ASSERT(size < MaxBlockSize);

            void* ptr = buddy_.allocate_block(size, [this, size]() noexcept {
                const size_t refill_size = ciel::max(size, RefillSize);
                return std::make_pair(Base::alloc_range(refill_size), refill_size);
            });

            CIELMALLOC_LOG("In large_buddy_range::alloc_range, returned ptr: {}", ptr);

            return ptr;
        }

        void dealloc_range(void* ptr, size_t size) noexcept {
            CIELMALLOC_LOG("In large_buddy_range::dealloc_range, ptr: {}, size: {}", ptr, size);

            CIEL_ASSERT(ptr != nullptr);
            CIEL_ASSERT(ciel::is_pow2(size));
            CIEL_ASSERT(MinBlockSize <= size);
            CIEL_ASSERT(size < MaxBlockSize);

            ptr = buddy_.deallocate_block(ptr, size);

            CIEL_ASSERT(ptr == nullptr);
            CIEL_UNUSED(ptr);
        }

    }; // class type

}; // struct large_buddy_range

} // namespace cielmalloc

#endif // CIELMALLOC_LARGE_BUDDY_RANGE_HPP_
