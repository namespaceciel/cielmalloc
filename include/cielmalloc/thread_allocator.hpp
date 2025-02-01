#ifndef CIELMALLOC_INCLUDE_CIELMALLOC_THREAD_ALLOCATOR_HPP_
#define CIELMALLOC_INCLUDE_CIELMALLOC_THREAD_ALLOCATOR_HPP_

#include <ciel/core/config.hpp>
#include <cielmalloc/global_allocator.hpp>
#include <cielmalloc/list.hpp>
#include <cielmalloc/medium_slab.hpp>
#include <cielmalloc/pool.hpp>
#include <cielmalloc/sizeclass.hpp>
#include <cielmalloc/small_slab.hpp>

#include <array>
#include <cstddef>

namespace cielmalloc {

class thread_allocator {
private:
    std::array<list<medium_slab>, NumMediumClasses> medium_slabs_;
    std::array<list<meta_slab>, NumSmallClasses> small_slabs_;
    list<small_slab> small_slabs_headquarter_;
    remote_allocator* remote_alloc_;

    thread_allocator() noexcept
        : remote_alloc_(pool<remote_allocator>::get().acquire()) {}

public:
    thread_allocator(const thread_allocator&)            = delete;
    thread_allocator& operator=(const thread_allocator&) = delete;

    // Give remote_alloc_ back to pool.
    ~thread_allocator() {
        pool<remote_allocator>::get().release(remote_alloc_);
    }

    CIEL_NODISCARD static thread_allocator& get() noexcept {
        static thread_local thread_allocator res;
        return res;
    }

private:
    void handle_message_queue() noexcept {
        // TODO
    }

    CIEL_NODISCARD remote_allocator* remote_alloc() noexcept {
        return remote_alloc_;
    }

    CIEL_NODISCARD void* small_alloc(size_t size) noexcept {
        const uint8_t small_sizeclass = cielmalloc::size_to_sizeclass(size);
        CIEL_ASSERT_M(small_sizeclass < small_slabs_.size(), "{} >= {}", small_sizeclass, small_slabs_.size());

        list<meta_slab>& slabs = small_slabs_[small_sizeclass];
        // Check if we have some available meta_slabs in list.
        if CIEL_LIKELY (!slabs.empty()) {
            meta_slab* slab = slabs.front();
            CIEL_ASSERT_M(&(small_slab::get_slab(slab)->meta_[slab->index_]) == slab, "{} != {}",
                          &(small_slab::get_slab(slab)->meta_[slab->index_]), slab);
            CIEL_ASSERT(!slab->empty());

            void* res = slab->allocate();
            CIEL_ASSERT(ciel::is_aligned(res, cielmalloc::sizeclass_to_alignment(small_sizeclass)));

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

        meta_slab* slab = hq->allocate_slab(small_sizeclass);
        CIEL_ASSERT_M(&(small_slab::get_slab(slab)->meta_[slab->index_]) == slab, "{} != {}",
                      &(small_slab::get_slab(slab)->meta_[slab->index_]), slab);

        if CIEL_UNLIKELY (hq->empty()) {
            small_slabs_headquarter_.pop_front();
        }

        void* res = slab->allocate();
        CIEL_ASSERT(ciel::is_aligned(res, cielmalloc::sizeclass_to_alignment(small_sizeclass)));

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
            // TODO
        }

        if CIEL_UNLIKELY (slab->deallocate(p)) {
            list<meta_slab>& slabs = small_slabs_[slab->sizeclass()];
            slabs.push_front(slab);

            // TODO: Back to one piece?
        }
    }

    CIEL_NODISCARD void* medium_alloc(size_t size) noexcept {
        const uint8_t sizeclass        = cielmalloc::size_to_sizeclass(size);
        const uint8_t medium_sizeclass = sizeclass - NumSmallClasses;
        CIEL_ASSERT_M(medium_sizeclass < medium_slabs_.size(), "{} >= {}", medium_sizeclass, medium_slabs_.size());

        list<medium_slab>& slabs = medium_slabs_[medium_sizeclass];
        // Check if we have some available slabs in list.
        if CIEL_LIKELY (!slabs.empty()) {
            medium_slab* slab = slabs.front();
            CIEL_ASSERT(!slab->empty());

            void* res = slab->allocate();

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

        void* res = slab->allocate();

        if CIEL_LIKELY (!slab->empty()) {
            slabs.push_front(slab);
        }

        return res;
    }

    void medium_dealloc(void* p) noexcept {
        CIEL_ASSERT(global_pagemap.load(p) == slab_kind::Medium);

        medium_slab* slab = medium_slab::get_slab(p);
        if (slab->remote_alloc() != remote_alloc()) {
            // TODO
        }

        if CIEL_UNLIKELY (slab->deallocate(p)) {
            const uint8_t sizeclass        = slab->sizeclass();
            const uint8_t medium_sizeclass = sizeclass - NumSmallClasses;
            CIEL_ASSERT_M(medium_sizeclass < medium_slabs_.size(), "{} >= {}", medium_sizeclass, medium_slabs_.size());

            list<medium_slab>& slabs = medium_slabs_[medium_sizeclass];
            slabs.push_front(slab);
        }
    }

    // Large allocations are rare and need to be reused as soon as possible,
    // so we manage them directly using a global lock-free stack.
    CIEL_NODISCARD void* large_alloc(size_t size) noexcept {
        return global_alloc.allocate(size);
    }

    void large_dealloc(void* p, slab_kind kind) noexcept {
        global_alloc.deallocate(p, kind);
    }

public:
    CIEL_NODISCARD void* allocate(const size_t size) noexcept {
        handle_message_queue();

        const uint8_t sizeclass = cielmalloc::size_to_sizeclass(size);

        if CIEL_LIKELY (sizeclass < NumSmallClasses) {
            return small_alloc(size);
        }

        if (sizeclass < NumSizeclasses) {
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

#endif // CIELMALLOC_INCLUDE_CIELMALLOC_THREAD_ALLOCATOR_HPP_
