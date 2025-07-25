#ifndef CIELMALLOC_RESERVE_RANGE_HPP_
#define CIELMALLOC_RESERVE_RANGE_HPP_

#include <ciel/core/config.hpp>
#include <cielmalloc/bits.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/pal.hpp>

#include <cstddef>

namespace cielmalloc {

class reserve_range {
public:
    CIEL_NODISCARD void* alloc_range(size_t size) noexcept {
        CIELMALLOC_LOG("In reserve_range::alloc_range, size: {}", size);

        CIEL_ASSERT(ciel::is_pow2(size));

        void* ptr = pal::reserve_aligned(size);

        CIELMALLOC_LOG("In reserve_range::alloc_range, returned ptr: {}", ptr);

        return ptr;
    }

}; // class reserve_range

} // namespace cielmalloc

#endif // CIELMALLOC_RESERVE_RANGE_HPP_
