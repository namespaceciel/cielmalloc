#ifndef CIELMALLOC_COMMIT_RANGE_HPP_
#define CIELMALLOC_COMMIT_RANGE_HPP_

#include <ciel/core/config.hpp>
#include <cielmalloc/bits.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/pal.hpp>

#include <cstddef>

namespace cielmalloc {

struct commit_range {
    template<class ParentRange>
    class type : public ParentRange {
    public:
        using Base = ParentRange;

        CIEL_NODISCARD void* alloc_range(size_t size) noexcept {
            CIELMALLOC_LOG("In commit_range::alloc_range, size: {}", size);

            CIEL_ASSERT(ciel::is_pow2(size));

            void* ptr = Base::alloc_range(size);

            if CIEL_UNLIKELY (ptr && !pal::commit<NoZero>(ptr, size)) {
                Base::dealloc_range(ptr, size);
                ptr = nullptr;
            }

            CIELMALLOC_LOG("In commit_range::alloc_range, returned ptr: {}", ptr);

            return ptr;
        }

        void dealloc_range(void* ptr, size_t size) noexcept {
            CIELMALLOC_LOG("In commit_range::dealloc_range, ptr: {}, size: {}", ptr, size);

            CIEL_ASSERT(ptr != nullptr);
            CIEL_ASSERT(ciel::is_pow2(size));

            pal::decommit(ptr, size);

            Base::dealloc_range(ptr, size);
        }

    }; // class type

}; // struct commit_range

} // namespace cielmalloc

#endif // CIELMALLOC_COMMIT_RANGE_HPP_
