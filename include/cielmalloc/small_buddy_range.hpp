#ifndef CIELMALLOC_SMALL_BUDDY_RANGE_HPP_
#define CIELMALLOC_SMALL_BUDDY_RANGE_HPP_

#include <ciel/core/config.hpp>
#include <cielmalloc/bits.hpp>
#include <cielmalloc/buddy.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/pal.hpp>

#include <cstddef>
#include <utility>

namespace cielmalloc {

struct small_buddy_range {
    template<class ParentRange>
    class type : public ParentRange {
    private:
        using Node                       = RBNodeInplace;
        static constexpr size_t BitBegin = cielmalloc::next_pow2_bits(sizeof(Node));
        static constexpr size_t BitEnd   = pal::page_size_bits;

        inline static thread_local buddy<Node, BitBegin, BitEnd> buddy_;

    public:
        static constexpr size_t MinBlockSize = cielmalloc::one_at_bit(BitBegin);
        static constexpr size_t MaxBlockSize = cielmalloc::one_at_bit(BitEnd);

        using Base = ParentRange;

        CIEL_NODISCARD void* alloc_range(size_t size) noexcept {
            CIELMALLOC_LOG("In small_buddy_range::alloc_range, size: {}", size);

            CIEL_ASSERT(ciel::is_pow2(size));
            CIEL_ASSERT(size >= MinBlockSize);

            void* ptr = nullptr;

            if (size >= MaxBlockSize) {
                ptr = Base::alloc_range(size);

            } else {
                ptr = buddy_.allocate_block(size, [this]() noexcept {
                    return std::make_pair(Base::alloc_range(MaxBlockSize), MaxBlockSize);
                });
            }

            CIELMALLOC_LOG("In small_buddy_range::alloc_range, returned ptr: {}", ptr);

            return ptr;
        }

        void dealloc_range(void* ptr, size_t size) noexcept {
            CIELMALLOC_LOG("In small_buddy_range::dealloc_range, ptr: {}, size: {}", ptr, size);

            CIEL_ASSERT(ptr != nullptr);
            CIEL_ASSERT(ciel::is_pow2(size));
            CIEL_ASSERT(size >= MinBlockSize);

            if (size >= MaxBlockSize) {
                Base::dealloc_range(ptr, size);
                return;
            }

            ptr = buddy_.deallocate_block(ptr, size);
            if (ptr != nullptr) {
                Base::dealloc_range(ptr, MaxBlockSize);
            }
        }

    }; // class type

}; // struct small_buddy_range

} // namespace cielmalloc

#endif // CIELMALLOC_SMALL_BUDDY_RANGE_HPP_
