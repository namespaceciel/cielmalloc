#ifndef CIELMALLOC_REMOTE_ALLOCATOR_HPP_
#define CIELMALLOC_REMOTE_ALLOCATOR_HPP_

#include <ciel/core/datasizeof.hpp>
#include <ciel/core/message.hpp>
#include <ciel/core/mpsc_queue.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/slab.hpp>

#include <atomic>
#include <cstddef>

namespace cielmalloc {

struct remote {
    static constexpr uintptr_t SizeclassShift = 48;
    static constexpr uintptr_t SizeclassMask  = static_cast<uintptr_t>(0xffff) << SizeclassShift;
    static constexpr uintptr_t TargetMask     = ~SizeclassMask;

    // Use top 16 bits to store belonging sizeclass (only 8 bits needed).
    static_assert(SizeclassMask == 0xffff'0000'0000'0000ULL);
    static_assert(TargetMask == 0x0000'ffff'ffff'ffffULL);

    union {
        std::atomic<remote*> next;
        remote* non_atomic_next{nullptr};
    };

    uintptr_t value;

    void set_target_id(const uintptr_t id) noexcept {
        CIEL_ASSERT(id == (id & TargetMask));

        const auto old_sizeclass = sizeclass();
        CIEL_UNUSED(old_sizeclass);

        value = (id & TargetMask) | (value & SizeclassMask);

        CIEL_ASSERT(target_id() == id);
        CIEL_ASSERT(sizeclass() == old_sizeclass);
    }

    void set_sizeclass(const uint8_t sc) noexcept {
        const auto old_target_id = target_id();
        CIEL_UNUSED(old_target_id);

        value = (value & TargetMask) | ((static_cast<uintptr_t>(sc) << SizeclassShift) & SizeclassMask);

        CIEL_ASSERT(sizeclass() == sc);
        CIEL_ASSERT(target_id() == old_target_id);
    }

    uintptr_t target_id() noexcept {
        return value & TargetMask;
    }

    uint8_t sizeclass() noexcept {
        return (value & SizeclassMask) >> SizeclassShift;
    }

}; // struct remote

static_assert(sizeof(remote) <= MinAllocSize, "");

struct remote_list {
    remote dummy;         // first element: dummy.non_atomic_next
    remote* last{&dummy}; // last element

    void insert(remote* begin, remote* end) noexcept {
        CIEL_ASSERT(last != nullptr);
        CIEL_ASSERT(begin != nullptr);
        CIEL_ASSERT(end != nullptr);

        last->non_atomic_next = begin;
        last                  = end;
    }

    void insert(remote* r) noexcept {
        insert(r, r);
    }

    CIEL_NODISCARD remote* clear() noexcept {
        last->non_atomic_next = nullptr;
        remote* res           = dummy.non_atomic_next;
        last                  = &dummy;
        return res;
    }

    CIEL_NODISCARD bool empty() const noexcept {
        return last == &dummy;
    }

}; // struct remote_list

struct remote_allocator {
    std::atomic<remote_allocator*> next; // Used by pool.
    ciel::mpsc_queue<remote> message_queue;

    uintptr_t id() noexcept {
        return reinterpret_cast<uintptr_t>(&message_queue);
    }

}; // struct remote_allocator

class slab_remote : public slab_base {
protected:
    remote_allocator* remote_alloc_;
    // Keep on a separate cacheline to avoid false sharing.
    unsigned char padding_[CachelineSize - sizeof(void*) * 2];

public:
    remote_allocator* remote_alloc() noexcept {
        return remote_alloc_;
    }

}; // class slab_remote

static_assert(ciel::datasizeof<slab_remote>::value == CachelineSize);
static_assert(alignof(slab_remote) == CachelineSize);

} // namespace cielmalloc

#endif // CIELMALLOC_REMOTE_ALLOCATOR_HPP_
