#ifndef CIELMALLOC_INCLUDE_CIELMALLOC_SLAB_HPP_
#define CIELMALLOC_INCLUDE_CIELMALLOC_SLAB_HPP_

#include <ciel/core/config.hpp>
#include <cielmalloc/bits.hpp>
#include <cielmalloc/config.hpp>

#include <atomic>

namespace cielmalloc {

enum struct slab_kind : uint8_t {
    Wild = 0,
    Small,
    Medium,
    // The following enums represent large slab in size of [16MB， 256TB).
    Large

}; // enum struct slab_kind

class alignas(CachelineSize) slab_base {
protected:
    // This default value is never used, the key point here is to make this base class non-POD,
    // so derived classes can make use of padding bytes.
    slab_kind kind_ = slab_kind::Wild;

public:
    CIEL_NODISCARD slab_kind kind() const noexcept {
        return kind_;
    }

}; // class slab_base

} // namespace cielmalloc

#endif // CIELMALLOC_INCLUDE_CIELMALLOC_SLAB_HPP_
