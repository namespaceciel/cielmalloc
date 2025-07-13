#ifndef CIELMALLOC_THREAD_ALLOCATOR_HPP_
#define CIELMALLOC_THREAD_ALLOCATOR_HPP_

#include <ciel/core/array.hpp>
#include <ciel/core/config.hpp>
#include <ciel/core/finally.hpp>
#include <cielmalloc/global_allocator.hpp>
#include <cielmalloc/list.hpp>
#include <cielmalloc/medium_slab.hpp>
#include <cielmalloc/pool.hpp>
#include <cielmalloc/sizeclass.hpp>
#include <cielmalloc/small_slab.hpp>

#include <cstddef>

namespace cielmalloc {

class thread_allocator {
private:
    ciel::array<list<medium_slab>, NumMediumClassesRange.first, NumMediumClassesRange.second> medium_slabs_;
    ciel::array<list<meta_slab>, NumSmallClassesRange.first, NumSmallClassesRange.second> small_slabs_;
    list<small_slab> small_slabs_headquarter_;
    remote_allocator* remote_alloc_;

    struct remote_cache {
        static constexpr size_t RemoteSlotBits = 6;
        static constexpr size_t RemoteSlots    = cielmalloc::one_at_bit(RemoteSlotBits);
        static constexpr size_t RemoteMask     = RemoteSlots - 1;

#ifdef CIEL_IS_DEBUGGING
        static constexpr size_t CacheSizeThreshold = 1;
#else
        static constexpr size_t CacheSizeThreshold = cielmalloc::one_at_bit(20);
#endif

        size_t size{0};
        remote_list remote_lists[RemoteSlots];

        void enqueue(const uintptr_t target_id, void* p, const uint8_t sizeclass) noexcept {
            CIELMALLOC_LOG("CielMalloc: message {} of {} enqueues", p, target_id);

            size += cielmalloc::sizeclass_to_size(sizeclass);

            remote* r = static_cast<remote*>(p);
            r->set_sizeclass(sizeclass);
            r->set_target_id(target_id);

            remote_list& l = remote_lists[target_id & RemoteMask];
            l.insert(r);
        }

        void post(const size_t my_id) noexcept {
            CIELMALLOC_LOG("CielMalloc: messages all post from alloc_id {}", my_id);

            size = 0;

            size_t shift = 0;
            while (true) {
                const size_t my_slot_index = (my_id >> shift) & RemoteMask;
                for (size_t i = 0; i < RemoteSlots; ++i) {
                    if CIEL_UNLIKELY (i == my_slot_index) {
                        continue;
                    }

                    remote_list& l = remote_lists[i];
                    if (!l.empty()) {
                        remote* last  = l.last;
                        remote* first = l.clear();

                        // TODO: It's UB since first could be medium_slab*,
                        // but it wouldn't cause a problem since we only use the common Base part.
                        slab_remote* slab = small_slab::get_slab(first);
                        slab->remote_alloc()->message_queue.push(first, last);
                    }
                }

                remote_list& my_slot = remote_lists[my_slot_index];
                if (my_slot.empty()) {
                    break;
                }

                remote* r = my_slot.clear();
                shift += RemoteSlotBits;
                while (r != nullptr) {
                    const size_t slot_index = (r->target_id() >> shift) & RemoteMask;
                    remote_list& l          = remote_lists[slot_index];
                    l.insert(r); // Guarantee to not change r->non_atomic_next.

                    r = r->non_atomic_next;
                }
            }
        }

    }; // struct remote_cache

    remote_cache remote_caches;

    thread_allocator() noexcept
        : remote_alloc_(pool<remote_allocator>::get().acquire()) {
        CIELMALLOC_LOG("CielMalloc: thread_allocator constructs from alloc_id {}", remote_alloc()->id());
    }

public:
    thread_allocator(const thread_allocator&)            = delete;
    thread_allocator& operator=(const thread_allocator&) = delete;

    // Post all messages, give remote_alloc_ back to pool.
    ~thread_allocator() {
        post<false>();

        CIELMALLOC_LOG("CielMalloc: thread_allocator destroys from alloc_id {}", remote_alloc()->id());

        pool<remote_allocator>::get().release(remote_alloc_);
    }

    CIEL_NODISCARD static thread_allocator& get() noexcept {
        static thread_local thread_allocator res;
        return res;
    }

private:
    CIEL_NODISCARD remote_allocator* remote_alloc() noexcept {
        return remote_alloc_;
    }

    CIEL_NODISCARD void* small_alloc(size_t size) noexcept {
        const uint8_t sizeclass = cielmalloc::size_to_sizeclass(size);

        void* res = nullptr;
        CIEL_DEFER({
            CIEL_ASSERT(ciel::is_aligned(res, cielmalloc::sizeclass_to_alignment(sizeclass)));
            CIELMALLOC_LOG("CielMalloc: small_alloc {} of size {} from alloc_id {}", res,
                           cielmalloc::sizeclass_to_size(sizeclass), remote_alloc()->id());
        });

        list<meta_slab>& slabs = small_slabs_[sizeclass];
        // Check if we have some available meta_slabs in list.
        if CIEL_LIKELY (!slabs.empty()) {
            meta_slab* slab = slabs.front();
            CIEL_ASSERT_M(&(small_slab::get_slab(slab)->meta_[slab->index_]) == slab, "{} != {}",
                          &(small_slab::get_slab(slab)->meta_[slab->index_]), slab);
            CIEL_ASSERT(!slab->empty());

            res = slab->allocate();

            if CIEL_UNLIKELY (slab->empty()) {
                slabs.pop_front();
            }

            return res;
        }

        if (small_slabs_headquarter_.empty()) {
            // Allocate a new slab.
            small_slab* hq = static_cast<small_slab*>(pal::reserve(LargeThreshold));
            pal::commit(hq, sizeof(small_slab));
            hq->init(remote_alloc());

            global_pagemap.store(hq, slab_kind::Small);

            small_slabs_headquarter_.push_front(hq);
        }

        small_slab* hq = small_slabs_headquarter_.front();
        CIEL_ASSERT(!hq->empty());

        meta_slab* slab = hq->allocate_slab(sizeclass);
        CIEL_ASSERT_M(&(small_slab::get_slab(slab)->meta_[slab->index_]) == slab, "{} != {}",
                      &(small_slab::get_slab(slab)->meta_[slab->index_]), slab);

        if CIEL_UNLIKELY (hq->empty()) {
            small_slabs_headquarter_.pop_front();
        }

        res = slab->allocate();

        if CIEL_LIKELY (!slab->empty()) {
            slabs.push_front(slab);
        }

        return res;
    }

    void small_dealloc(void* p) noexcept {
        CIEL_ASSERT(global_pagemap.load(p) == slab_kind::Small);

        meta_slab* slab = meta_slab::get_meta_slab(p);
        small_slab* hq  = small_slab::get_slab(p);
        if (hq->remote_alloc() != remote_alloc()) {
            remote_dealloc(hq->remote_alloc(), p, slab->sizeclass());
            return;
        }

        CIELMALLOC_LOG("CielMalloc: small_dealloc {} from alloc_id {}", p, remote_alloc()->id());

        if CIEL_UNLIKELY (slab->deallocate(p)) {
            list<meta_slab>& slabs = small_slabs_[slab->sizeclass()];
            slabs.push_front(slab);

            // TODO: Back to one piece?
        }
    }

    CIEL_NODISCARD void* medium_alloc(size_t size) noexcept {
        const uint8_t sizeclass = cielmalloc::size_to_sizeclass(size);

        void* res = nullptr;
        CIEL_DEFER({
            CIEL_ASSERT(ciel::is_aligned(res, cielmalloc::sizeclass_to_alignment(sizeclass)));
            CIELMALLOC_LOG("CielMalloc: medium_alloc {} of size {} from alloc_id {}", res,
                           cielmalloc::sizeclass_to_size(sizeclass), remote_alloc()->id());
        });

        list<medium_slab>& slabs = medium_slabs_[sizeclass];
        // Check if we have some available slabs in list.
        if CIEL_LIKELY (!slabs.empty()) {
            medium_slab* slab = slabs.front();
            CIEL_ASSERT(!slab->empty());

            res = slab->allocate();

            if CIEL_UNLIKELY (slab->empty()) {
                slabs.pop_front();
            }

            return res;
        }

        // Allocate a new slab.
        medium_slab* slab = static_cast<medium_slab*>(pal::reserve(LargeThreshold));
        pal::commit(slab, sizeof(medium_slab));
        slab->init(remote_alloc(), sizeclass);

        global_pagemap.store(slab, slab_kind::Medium);

        res = slab->allocate();

        if CIEL_LIKELY (!slab->empty()) {
            slabs.push_front(slab);
        }

        return res;
    }

    void medium_dealloc(void* p) noexcept {
        CIEL_ASSERT(global_pagemap.load(p) == slab_kind::Medium);

        medium_slab* slab = medium_slab::get_slab(p);
        if (slab->remote_alloc() != remote_alloc()) {
            remote_dealloc(slab->remote_alloc(), p, slab->sizeclass());
            return;
        }

        CIELMALLOC_LOG("CielMalloc: medium_dealloc {} from alloc_id {}", p, remote_alloc()->id());

        if CIEL_UNLIKELY (slab->deallocate(p)) {
            const uint8_t sizeclass  = slab->sizeclass();
            list<medium_slab>& slabs = medium_slabs_[sizeclass];
            slabs.push_front(slab);
        }
    }

    // Large allocations are rare and need to be reused as soon as possible,
    // so we manage them directly using a global lock-free stack.
    CIEL_NODISCARD void* large_alloc(size_t size) noexcept {
        void* res = global_alloc.allocate(size);

        CIELMALLOC_LOG("CielMalloc: large_alloc {} of size {} from alloc_id {}", res, size, remote_alloc()->id());

        return res;
    }

    void large_dealloc(void* p, slab_kind kind) noexcept {
        CIELMALLOC_LOG("CielMalloc: large_dealloc {} from alloc_id {}", p, remote_alloc()->id());
        global_alloc.deallocate(p, kind);
    }

    template<bool Conditional>
    void post() noexcept {
        if constexpr (Conditional) {
            if CIEL_LIKELY (remote_caches.size < remote_cache::CacheSizeThreshold) {
                return;
            }
        }

        remote_caches.post(remote_alloc()->id());
    }

    void remote_dealloc(remote_allocator* target, void* p, uint8_t sizeclass) {
        CIEL_ASSERT(target != remote_alloc());

        remote_caches.enqueue(target->id(), p, sizeclass);

        post<true>();
    }

    void handle_message_queue() noexcept {
        remote_alloc()->message_queue.process([&](remote* r) {
            const auto sizeclass = r->sizeclass();
            CIEL_ASSERT(sizeclass <= NumSizeclasses);

            if (r->target_id() == remote_alloc()->id()) {
                if (sizeclass < NumSmallClassesRange.second) {
                    small_dealloc(r);

                } else {
                    medium_dealloc(r);
                }

            } else {
                remote_caches.enqueue(r->target_id(), r, sizeclass);
            }

            return true;
        });

        post<true>();
    }

public:
    CIEL_NODISCARD void* allocate(const size_t size) noexcept {
        handle_message_queue();

        const uint8_t sizeclass = cielmalloc::size_to_sizeclass(size);

        if CIEL_LIKELY (sizeclass < NumSmallClassesRange.second) {
            return small_alloc(size);
        }

        if (sizeclass < NumMediumClassesRange.second) {
            return medium_alloc(size);
        }

        return large_alloc(size);
    }

    void deallocate(void* p) noexcept {
        handle_message_queue();

        const slab_kind kind = global_pagemap.load(p);

        switch (kind) {
            case slab_kind::Wild : {
                ciel::unreachable();
            }
            case slab_kind::Small : {
                small_dealloc(p);
                return;
            }
            case slab_kind::Medium : {
                medium_dealloc(p);
                return;
            }
            default : {
                large_dealloc(p, kind);
            }
        }
    }

}; // class thread_allocator

} // namespace cielmalloc

#endif // CIELMALLOC_THREAD_ALLOCATOR_HPP_
