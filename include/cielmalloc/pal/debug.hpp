#ifndef CIELMALLOC_INCLUDE_CIELMALLOC_PAL_DEBUG_HPP_
#define CIELMALLOC_INCLUDE_CIELMALLOC_PAL_DEBUG_HPP_

#include <ciel/core/alignment.hpp>
#include <ciel/core/config.hpp>
#include <ciel/core/message.hpp>
#include <cielmalloc/config.hpp>

#include <new>

namespace cielmalloc {

struct pal {
    template<bool = false>
    CIEL_NODISCARD static void* reserve(size_t size) noexcept {
        CIEL_ASSERT(ciel::is_pow2(size));
        CIEL_ASSERT(size >= OSPageSize);

        void* res = ::operator new(size, static_cast<std::align_val_t>(size));
        // zero(res, size);
        return res;
    }

    static void commit(void*, size_t) noexcept {}

    static void decommit(void* p, size_t size) noexcept {
#ifdef CIEL_IS_DEBUGGING
        // std::memset(p, 0b10101010, size); // decommitted size could be larger than initially committed size.
#endif
        CIEL_UNUSED(p, size);
    }

    void zero(void* p, size_t size) noexcept {
        std::memset(p, 0, size);
    }

}; // struct pal

} // namespace cielmalloc

#endif // CIELMALLOC_INCLUDE_CIELMALLOC_PAL_APPLE_HPP_
