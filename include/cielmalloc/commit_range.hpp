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

        CIEL_NODISCARD void* alloc_range(size_t size) noexcept {
            void* ptr = Base::alloc_range(size);

            if (ptr && !pal::commit<NoZero>(ptr, size)) {
                Base::dealloc_range(ptr, size);
                return nullptr;
            }

            return ptr;
        }

        void dealloc_range(void* ptr, size_t size) noexcept {
            pal::decommit(ptr, size);

            Base::dealloc_range(ptr, size);
        }

    }; // class type

}; // struct commit_range

} // namespace cielmalloc

#endif // CIELMALLOC_RANGE_COMMIT_RANGE_HPP_
