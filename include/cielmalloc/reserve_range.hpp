#ifndef CIELMALLOC_RANGE_RESERVE_RANGE_HPP_
#define CIELMALLOC_RANGE_RESERVE_RANGE_HPP_

#include <ciel/core/config.hpp>
#include <cielmalloc/bits.hpp>
#include <cielmalloc/pal.hpp>

#include <cstddef>

namespace cielmalloc {

class reserve_range {
public:
    CIEL_NODISCARD void* alloc_range(size_t size) noexcept {
        return pal::reserve(size);
    }

    void dealloc_range(void*, size_t) noexcept {}

}; // class reserve_range

} // namespace cielmalloc

#endif // CIELMALLOC_RANGE_RESERVE_RANGE_HPP_
