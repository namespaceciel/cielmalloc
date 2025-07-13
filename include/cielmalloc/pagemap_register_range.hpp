#ifndef CIELMALLOC_PAGEMAP_REGISTER_RANGE_HPP_
#define CIELMALLOC_PAGEMAP_REGISTER_RANGE_HPP_

#include <ciel/core/config.hpp>
#include <cielmalloc/bits.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/pagemap.hpp>
#include <cielmalloc/pal.hpp>

#include <cstddef>

namespace cielmalloc {

struct pagemap_register_range {
    template<class ParentRange>
    class type : public ParentRange {
    public:
        using Base = ParentRange;

        CIEL_NODISCARD void* alloc_range(size_t size) noexcept {
            CIELMALLOC_LOG("In pagemap_register_range::alloc_range, size: {}", size);

            CIEL_ASSERT(ciel::is_pow2(size));

            void* ptr = Base::alloc_range(size);

            if CIEL_UNLIKELY (ptr != nullptr && !Pagemap::get().register_range(ptr, size)) {
                Base::dealloc_range(ptr, size);
                ptr = nullptr;
            }

            CIELMALLOC_LOG("In pagemap_register_range::alloc_range, returned ptr: {}", ptr);

            return ptr;
        }

        void dealloc_range(void* ptr, size_t size) noexcept {
            CIELMALLOC_LOG("In pagemap_register_range::dealloc_range, ptr: {}, size: {}", ptr, size);

            CIEL_ASSERT(ptr != nullptr);
            CIEL_ASSERT(ciel::is_pow2(size));

            Base::dealloc_range(ptr, size);
        }

    }; // class type

}; // struct pagemap_register_range

} // namespace cielmalloc

#endif // CIELMALLOC_PAGEMAP_REGISTER_RANGE_HPP_
