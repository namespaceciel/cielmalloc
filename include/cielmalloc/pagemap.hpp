#ifndef CIELMALLOC_PAGEMAP_HPP_
#define CIELMALLOC_PAGEMAP_HPP_

#include <ciel/core/alignment.hpp>
#include <ciel/core/config.hpp>
#include <ciel/core/message.hpp>
#include <ciel/core/rb_tree.hpp>
#include <cielmalloc/bits.hpp>
#include <cielmalloc/pal.hpp>

#include <cstddef>
#include <cstdint>

namespace cielmalloc {

// This class is not thread-safe.
template<class T, size_t GranularityBits = pal::page_size_bits>
class pagemap {
public:
    static constexpr size_t Shift        = GranularityBits;
    static constexpr size_t Granularity  = cielmalloc::one_at_bit(GranularityBits);
    static constexpr size_t MapSize      = cielmalloc::one_at_bit(AddressBits - GranularityBits);
    static constexpr size_t MapSizeBytes = ciel::align_up(MapSize * sizeof(T), pal::page_size);

private:
    T* body_;

    pagemap() noexcept
        : body_(static_cast<T*>(pal::reserve(MapSizeBytes))) {
        CIEL_ASSERT_M(body_ != nullptr, "Failed to initialise pagemap in reserve");

        if constexpr (pal_supports<LazyCommit>) {
            const bool result = pal::commit_readonly(body_, MapSizeBytes);

            CIEL_ASSERT_M(result, "Failed to initialise pagemap in commit_readonly");
            CIEL_UNUSED(result);
        }
    }

    inline static pagemap instance;

public:
    static pagemap& get() noexcept {
        return instance;
    }

    pagemap(const pagemap&)            = delete;
    pagemap& operator=(const pagemap&) = delete;

    CIEL_NODISCARD constexpr size_t size() const noexcept {
        return MapSize;
    }

    CIEL_NODISCARD bool register_range(uintptr_t ptr, size_t length) noexcept {
        T* first = body_ + (ptr >> Shift);
        T* last  = body_ + ((ptr + length - 1 + Granularity) >> Shift);

        const auto page_start = cielmalloc::transform<char>(first, [](uintptr_t p) noexcept {
            return ciel::align_down(p, pal::page_size);
        });
        const auto page_end   = cielmalloc::transform<char>(last, [](uintptr_t p) noexcept {
            return ciel::align_up(p, pal::page_size);
        });

        return pal::template commit<NoZero>(page_start, page_end - page_start);
    }

    CIEL_NODISCARD bool register_range(void* ptr, size_t length) noexcept {
        return register_range(reinterpret_cast<uintptr_t>(ptr), length);
    }

    template<bool AssumeRegistered>
    CIEL_NODISCARD T& get(uintptr_t ptr) noexcept {
        if constexpr (!AssumeRegistered && !pal_supports<LazyCommit>) {
            register_range(ptr, 1);
        }

        return body_[ptr >> Shift];
    }

    template<bool AssumeRegistered>
    CIEL_NODISCARD T& get(void* ptr) noexcept {
        return get<AssumeRegistered>(reinterpret_cast<uintptr_t>(ptr));
    }

    void set(uintptr_t ptr, const T& value) noexcept {
        body_[ptr >> Shift] = value;
    }

    void set(void* ptr, const T& value) noexcept {
        set(reinterpret_cast<uintptr_t>(ptr), value);
    }

    CIEL_NODISCARD void* get_page_ptr(const T* ptr) const noexcept {
        const size_t index = ptr - body_;

        CIEL_ASSERT_M(index < size(), "index {} out of bound, size is {}", index, size());

        return reinterpret_cast<void*>(index << Shift);
    }

}; // class pagemap

struct RBNodePagemap : ciel::rb_node_base {
    using value_type = uintptr_t;

    CIEL_NODISCARD value_type value() const noexcept {
        return reinterpret_cast<value_type>(pagemap<RBNodePagemap>::get().get_page_ptr(this));
    }

}; // struct RBNodePagemap

using PagemapEntry = RBNodePagemap;
using Pagemap      = pagemap<PagemapEntry>;

} // namespace cielmalloc

#endif // CIELMALLOC_PAGEMAP_HPP_
