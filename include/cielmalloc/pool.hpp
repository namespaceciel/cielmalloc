#ifndef CIELMALLOC_POOL_HPP_
#define CIELMALLOC_POOL_HPP_

#include <ciel/core/alignment.hpp>
#include <ciel/core/config.hpp>
#include <ciel/core/treiber_stack.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/pal.hpp>

#include <atomic>

namespace cielmalloc {

template<class T>
class pool {
    static_assert(std::is_same_v<decltype(T::next), std::atomic<T*>>);

private:
    static constexpr size_t slab_size           = LargeThreshold;
    static constexpr size_t allocated_alignment = ciel::max<size_t>(alignof(T), CachelineSize);
    static constexpr size_t allocated_size      = ciel::align_up(sizeof(T), allocated_alignment);

    struct slab_view {
        std::atomic<slab_view*> next;
        uintptr_t bump;

        void init() noexcept {
            bump = reinterpret_cast<uintptr_t>(this) + allocated_size;
        }

        CIEL_NODISCARD T* allocate() noexcept {
            CIEL_ASSERT(!empty());
            CIEL_ASSERT(ciel::is_aligned(bump, allocated_alignment));

            T* res = reinterpret_cast<T*>(bump);
            bump += allocated_size;

            const uintptr_t page_start = ciel::align_down(reinterpret_cast<uintptr_t>(res), OSPageSize);
            const uintptr_t page_end   = ciel::align_up(reinterpret_cast<uintptr_t>(res) + sizeof(T), OSPageSize);
            pal::commit(reinterpret_cast<void*>(page_start), page_end - page_start);

            CIEL_ASSERT(ciel::is_aligned(res, alignof(T)));
            return res;
        }

        CIEL_NODISCARD bool empty() const noexcept {
            return bump + allocated_size > reinterpret_cast<uintptr_t>(this) + slab_size;
        }

    }; // struct slab_view

    static_assert(sizeof(slab_view) <= allocated_size);

    ciel::treiber_stack<slab_view> slabs_;
    ciel::treiber_stack<T> objects_;

    pool() = default;

public:
    pool(const pool&)            = delete;
    pool& operator=(const pool&) = delete;

    CIEL_NODISCARD T* acquire() noexcept {
        if (T* top = objects_.pop(); top != nullptr) {
            return top;
        }

        slab_view* slab = nullptr;
        if (slab = slabs_.pop(); slab == nullptr) {
            slab = reinterpret_cast<slab_view*>(pal::reserve(slab_size));
            pal::commit(slab, sizeof(slab_view));
            slab->init();
        }

        T* res = slab->allocate();

        if CIEL_LIKELY (!slab->empty()) {
            slabs_.push(slab);
        }

        return res;
    }

    void release(T* p) noexcept {
        objects_.push(p);
    }

    inline static pool instance;

    CIEL_NODISCARD static pool& get() noexcept {
        return instance;
    }

}; // class pool

} // namespace cielmalloc

#endif // CIELMALLOC_POOL_HPP_
