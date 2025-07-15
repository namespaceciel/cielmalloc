#ifndef CIELMALLOC_RANGE_COMMIT_RANGE_HPP_
#define CIELMALLOC_RANGE_COMMIT_RANGE_HPP_

#include <ciel/core/config.hpp>
#include <cielmalloc/bits.hpp>
#include <cielmalloc/pal.hpp>

#include <cstddef>

namespace cielmalloc {

struct commit_range {
    template<class ParentRange>
    class type : public ParentRange {
    public:
        using Base = ParentRange;

        CIEL_NODISCARD void* alloc_range(size_t bit) noexcept {
            void* ptr = Base::alloc_range(bit);

            pal::commit(ptr, cielmalloc::one_at_bit(bit));

            return ptr;
        }

        void dealloc_range(void* ptr, size_t bit) noexcept {
            pal::decommit(ptr, bit);

            Base::dealloc_range(ptr, bit);
        }

    }; // class type

}; // struct commit_range

} // namespace cielmalloc

#endif // CIELMALLOC_RANGE_COMMIT_RANGE_HPP_
