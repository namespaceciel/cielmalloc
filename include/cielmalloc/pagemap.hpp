#ifndef CIELMALLOC_PAGEMAP_HPP_
#define CIELMALLOC_PAGEMAP_HPP_

#include <ciel/core/config.hpp>
#include <ciel/core/message.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/slab.hpp>

#include <array>
#include <atomic>
#include <cstddef>

namespace cielmalloc {

// For a whole 48bit address space, each chunk's size is 16MB (1 << 24),
// so we need 16MB flat map to determine the kind of every chunk.
class pagemap {
private:
    std::array<std::atomic<slab_kind>, LargeThreshold> map_{};

public:
    void store(void* p, const slab_kind type) noexcept {
        const uintptr_t i = reinterpret_cast<uintptr_t>(p) >> LargeThresholdBits;

        CIEL_ASSERT(i < LargeThreshold);

        map_[i].store(type, std::memory_order_relaxed);
    }

    CIEL_NODISCARD slab_kind load(void* p) const noexcept {
        const uintptr_t i = reinterpret_cast<uintptr_t>(p) >> LargeThresholdBits;

        CIEL_ASSERT(i < LargeThreshold);

        return map_[i].load(std::memory_order_relaxed);
    }

    CIEL_NODISCARD slab_kind exchange(void* p, const slab_kind type = slab_kind::Wild) noexcept {
        const uintptr_t i = reinterpret_cast<uintptr_t>(p) >> LargeThresholdBits;

        CIEL_ASSERT(i < LargeThreshold);

        return map_[i].exchange(type, std::memory_order_relaxed);
    }

}; // class pagemap

inline pagemap global_pagemap;

} // namespace cielmalloc

#endif // CIELMALLOC_PAGEMAP_HPP_
