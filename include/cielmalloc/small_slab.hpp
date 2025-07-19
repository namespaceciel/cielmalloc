#ifndef CIELMALLOC_SMALL_SLAB_HPP_
#define CIELMALLOC_SMALL_SLAB_HPP_

#include <ciel/core/alignment.hpp>
#include <ciel/core/config.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/list.hpp>
#include <cielmalloc/pal.hpp>
#include <cielmalloc/remote_allocator.hpp>
#include <cielmalloc/sizeclass.hpp>
#include <cielmalloc/slab.hpp>

#include <array>
#include <cstddef>
#include <cstdint>

namespace cielmalloc {

// Located in front of a 16MB small_slab, storing some metadata to describe a 64KB slab.
class meta_slab : public list_node_base {
private:
    struct list_view {
        list_view* next;

    }; // struct list_view

    uint16_t slots_;
    uint8_t sizeclass_;
    uint8_t index_;
    list_view* head_;

    friend class small_slab;
    friend class thread_allocator;

    CIEL_NODISCARD uintptr_t chunk_address() const noexcept {
        return reinterpret_cast<uintptr_t>(this) & LargeMask;
    }

    CIEL_NODISCARD uintptr_t slab_address() const noexcept {
        return chunk_address() + (index_ + 1) * MediumThreshold;
    }

    // TODO: Temporarily used by small_slab.
    CIEL_NODISCARD uintptr_t slab_address(const uint8_t index) const noexcept {
        return chunk_address() + (index + 1) * MediumThreshold;
    }

public:
    CIEL_NODISCARD static meta_slab* get_meta_slab(void* p) noexcept;

    CIEL_NODISCARD uint8_t sizeclass() const noexcept {
        return sizeclass_;
    }

    CIEL_NODISCARD bool empty() const noexcept {
        CIEL_ASSERT((static_cast<uint8_t>(slots_ == 0) ^ static_cast<uint8_t>(head_ == nullptr)) == 0);

        return slots_ == 0;
    }

    CIEL_NODISCARD bool full() const noexcept {
        return slots_ == cielmalloc::small_slab_slots(sizeclass_);
    }

    void init(const uint8_t index, const uint8_t sc) {
        sizeclass_ = sc;
        slots_     = cielmalloc::small_slab_slots(sc);
        index_     = index;
        head_      = nullptr;

        CIEL_ASSERT(slots_ != 0);

        const size_t allocated_size = cielmalloc::sizeclass_to_size(sc);
        uintptr_t begin             = slab_address();
        const uintptr_t end         = begin + MediumThreshold;
        for (; begin + allocated_size <= end; begin += allocated_size) {
            CIEL_ASSERT(ciel::is_aligned(begin, cielmalloc::sizeclass_to_alignment(sc)));

            list_view* new_head = reinterpret_cast<list_view*>(begin);
            new_head->next      = head_;
            head_               = new_head;
        }
    }

    CIEL_NODISCARD void* allocate() noexcept {
        CIEL_ASSERT(!empty());
        --slots_;

        list_view* res = head_;
        head_          = head_->next;
        return res;
    }

    // Return true if the small_slab was drained before this deallocation.
    CIEL_NODISCARD bool deallocate(void* p) noexcept {
        CIEL_ASSERT(ciel::is_aligned(p, cielmalloc::sizeclass_to_alignment(sizeclass_)));
        CIEL_ASSERT((reinterpret_cast<uintptr_t>(p) & MediumMask) == slab_address());

        const bool was_empty = empty();
        ++slots_;

        list_view* ptr = static_cast<list_view*>(p);
        ptr->next      = head_;
        head_          = ptr;

        return was_empty;
    }

}; // class meta_slab

// This is the view of a 16MB small_slab when it is being used to allocate 64KB slabs.
// size: [ 16B,    32B,   48B,    64B,  80B,  96B,  112B,  128B,
//         160B,   192B,  224B,   256B,
//         320B,   384B,  448B,   512B,
//         640B,   768B,  896B,   1KB
//         1.25KB, 1.5KB, 1.75KB, 2KB,
//         2.5KB,  3KB,   3.5KB,  4KB,
//         5KB,    6KB,   7KB,    8KB,
//         10KB,   12KB,  14KB,   16KB,
//         20KB,   24KB,  28KB,   32KB,
//         40KB,   48KB,  56KB,   64KB )
// 16MB slab is divided in 256 64KB slabs, the first slab contains some metadata,
// while last 255 slabs can be used in allocating.
class small_slab : public slab_remote,
                   public list_node_base {
private:
    std::array<meta_slab, 255> meta_;
    // A reverse stack (top at index 0) stored all valid meta_slabs.
    std::array<uint8_t, 255> stack_;
    uint8_t top_;

    friend class meta_slab;
    friend class thread_allocator;

    CIEL_NODISCARD uint8_t slab_index(void* ptr) const noexcept {
        CIEL_ASSERT(ciel::is_aligned(this, LargeThreshold));

        const uintptr_t chunk_address = reinterpret_cast<uintptr_t>(this);
        const uintptr_t p             = reinterpret_cast<uintptr_t>(ptr);

        CIEL_ASSERT((p & LargeMask) == chunk_address);
        CIEL_ASSERT((p & MediumMask) != chunk_address);

        return (p & MediumMask) - chunk_address - 1;
    }

public:
    // Every small_slab is 16MB aligned, so every object allocated here can get slab address by setting low 24 bits to
    // zero.
    CIEL_NODISCARD static small_slab* get_slab(void* p) noexcept {
        return reinterpret_cast<small_slab*>(reinterpret_cast<uintptr_t>(p) & LargeMask);
    }

    CIEL_NODISCARD bool empty() const noexcept {
        return top_ == stack_.size();
    }

    CIEL_NODISCARD bool full() const noexcept {
        return top_ == 0;
    }

    void init(remote_allocator* alloc) noexcept {
        CIEL_ASSERT(alloc != nullptr);

        remote_alloc_ = alloc;
        top_          = 0;
        for (uint8_t i = 0; i < stack_.size(); ++i) {
            stack_[i] = i;
        }
    }

    CIEL_NODISCARD meta_slab* allocate_slab(uint8_t sizeclass) noexcept {
        CIEL_ASSERT(!empty());

        const uint8_t index = stack_[top_++];
        meta_slab* slab     = &meta_[index];

        slab->init(index, sizeclass);
        return slab;
    }

    void deallocate_slab(meta_slab* slab) noexcept {
        CIEL_ASSERT(ciel::is_aligned(slab, MediumThreshold));
        CIEL_ASSERT(top_ > 0);

        stack_[--top_] = slab->index_;

        pal::decommit(reinterpret_cast<void*>(slab->slab_address()), MediumThreshold);
    }

}; // class small_slab

static_assert(sizeof(small_slab) < MediumThreshold);

CIEL_NODISCARD inline meta_slab* meta_slab::get_meta_slab(void* p) noexcept {
    const uint8_t index = ((reinterpret_cast<uintptr_t>(p) & ~LargeMask) >> MediumThresholdBits) - 1;
    return &(small_slab::get_slab(p)->meta_[index]);
}

} // namespace cielmalloc

#endif // CIELMALLOC_SMALL_SLAB_HPP_
