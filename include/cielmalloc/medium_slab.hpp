#ifndef CIELMALLOC_MEDIUM_SLAB_HPP_
#define CIELMALLOC_MEDIUM_SLAB_HPP_

#include <ciel/core/alignment.hpp>
#include <ciel/core/config.hpp>
#include <ciel/core/message.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/list.hpp>
#include <cielmalloc/pal.hpp>
#include <cielmalloc/remote_allocator.hpp>
#include <cielmalloc/sizeclass.hpp>
#include <cielmalloc/slab.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace cielmalloc {

// This is the view of a 16MB area when it is being used to allocate medium sized classes: [64KB, 16MB)
// size: [ 64KB,   80KB,  96KB,   112KB,  128KB,
//         160KB,  192KB, 224KB,  256KB,
//         320KB,  384KB, 448KB,  512KB,
//         640KB,  768KB, 896KB,  1MB,
//         1.25MB, 1.5MB, 1.75MB, 2MB,
//         2.5MB,  3MB,   3.5MB,  4MB,
//         5MB,    6MB,   7MB,    8MB,
//         10MB,   12MB,  14MB,   16MB )
// Given min allocated size 64KB, apart from metadata, this slab will allocate 16MB / 64KB - 1 = 255 objects at most,
// So we use 8bit-sized stack to
class medium_slab : public slab_remote,
                    public list_node_base {
private:
    // A reverse stack (top at index 0) stored all valid objects.
    std::array<uint32_t, 255> stack_;
    uint8_t top_;
    uint8_t slots_;
    uint8_t sizeclass_;

    // Get the offset from the slab for a memory location.
    CIEL_NODISCARD uint32_t pointer_to_index(void* p) const noexcept {
        const uintptr_t b = reinterpret_cast<uintptr_t>(this);
        const uintptr_t e = reinterpret_cast<uintptr_t>(p);

        CIEL_ASSERT(ciel::is_aligned(p, cielmalloc::sizeclass_to_alignment(sizeclass_)));
        CIEL_ASSERT((e & LargeMask) == b);
        CIEL_ASSERT(e - b <= std::numeric_limits<uint32_t>::max());

        return static_cast<uint32_t>(e - b);
    }

public:
    // Every slab is 16MB aligned, so every object allocated here can get slab address by setting low 24 bits to zero.
    CIEL_NODISCARD static medium_slab* get_slab(void* p) noexcept {
        return reinterpret_cast<medium_slab*>(reinterpret_cast<uintptr_t>(p) & LargeMask);
    }

    CIEL_NODISCARD uint8_t sizeclass() const noexcept {
        return sizeclass_;
    }

    CIEL_NODISCARD bool empty() const noexcept {
        return top_ == slots_;
    }

    CIEL_NODISCARD bool full() const noexcept {
        return top_ == 0;
    }

    void init(remote_allocator* alloc, uint8_t sc) noexcept {
        CIEL_ASSERT(ciel::is_aligned(this, LargeThreshold));
        CIEL_ASSERT(NumMediumClassesRange.first <= sc && sc < NumMediumClassesRange.second);
        CIEL_ASSERT(alloc != nullptr);

        remote_alloc_ = alloc;
        kind_         = slab_kind::Medium;
        sizeclass_    = sc;
        top_          = 0;
        slots_        = cielmalloc::medium_slab_slots(sc);

        const size_t allocated_size = cielmalloc::sizeclass_to_size(sc);
        for (uint8_t i = slots_; i > 0; --i) {
            stack_[slots_ - i] = static_cast<uint32_t>(LargeThreshold - (i * allocated_size));
        }
    }

    CIEL_NODISCARD void* allocate() noexcept {
        CIEL_ASSERT(!empty());

        const uint32_t index = stack_[top_++];
        void* p = reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(this) + static_cast<uintptr_t>(index));
        pal::commit(p, cielmalloc::sizeclass_to_size(sizeclass()));

        CIEL_ASSERT(ciel::is_aligned(p, cielmalloc::sizeclass_to_alignment(sizeclass_)));
        CIEL_ASSERT_M(get_slab(p) == this, "Allocated address {} go beyond this slab {}", p, this);

        return p;
    }

    // Return true if the medium_slab was drained before this deallocation.
    CIEL_NODISCARD bool deallocate(void* p) noexcept {
        CIEL_ASSERT_M(get_slab(p) == this, "Deallocated address {} doesn't belong to this slab {}", p, this);
        CIEL_ASSERT(ciel::is_aligned(p, cielmalloc::sizeclass_to_alignment(sizeclass_)));
        CIEL_ASSERT(top_ > 0);

        pal::decommit(p, cielmalloc::sizeclass_to_size(sizeclass()));

        const bool was_empty = empty();
        stack_[--top_]       = pointer_to_index(p);

        return was_empty;
    }

}; // class medium_slab

static_assert(sizeof(medium_slab) < MediumThreshold);

} // namespace cielmalloc

#endif // CIELMALLOC_MEDIUM_SLAB_HPP_
