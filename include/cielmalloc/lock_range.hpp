#ifndef CIELMALLOC_LOCK_RANGE_HPP_
#define CIELMALLOC_LOCK_RANGE_HPP_

#include <ciel/core/combining_lock.hpp>
#include <ciel/core/config.hpp>
#include <cielmalloc/config.hpp>

#include <cstddef>

namespace cielmalloc {

struct lock_range {
    template<class ParentRange>
    class type : public ParentRange {
    private:
        ciel::combining_lock lock_;

    public:
        using Base = ParentRange;

        CIEL_NODISCARD void* alloc_range(size_t size) noexcept {
            CIELMALLOC_LOG("In lock_range::alloc_range, size: {}", size);

            CIEL_ASSERT(ciel::is_pow2(size));

            void* ptr = nullptr;

            ciel::with(lock_, [&]() noexcept {
                ptr = Base::alloc_range(size);
            });

            CIELMALLOC_LOG("In lock_range::alloc_range, returned ptr: {}", ptr);

            return ptr;
        }

        void dealloc_range(void* ptr, size_t size) noexcept {
            CIELMALLOC_LOG("In lock_range::dealloc_range, ptr: {}, size: {}", ptr, size);

            CIEL_ASSERT(ptr != nullptr);
            CIEL_ASSERT(ciel::is_pow2(size));

            ciel::with(lock_, [&]() noexcept {
                Base::dealloc_range(ptr, size);
            });
        }

    }; // class type

}; // struct lock_range

} // namespace cielmalloc

#endif // CIELMALLOC_LOCK_RANGE_HPP_
