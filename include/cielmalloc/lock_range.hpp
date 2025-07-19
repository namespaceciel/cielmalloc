#ifndef CIELMALLOC_RANGE_LOCK_RANGE_HPP_
#define CIELMALLOC_RANGE_LOCK_RANGE_HPP_

#include <ciel/core/combining_lock.hpp>
#include <ciel/core/config.hpp>

#include <cstddef>

namespace cielmalloc {

struct lock_range {
    template<class ParentRange>
    class type : public ParentRange {
    private:
        ciel::combining_lock lock_;

    public:
        using Base = ParentRange;

        CIEL_NODISCARD void* alloc_range(size_t bit) noexcept {
            void* res = nullptr;

            ciel::with(lock_, [&]() noexcept {
                res = Base::alloc_range(bit);
            });

            return res;
        }

        void dealloc_range(void* ptr, size_t bit) noexcept {
            ciel::with(lock_, [&]() noexcept {
                Base::dealloc_range(ptr, bit);
            });
        }

    }; // class type

}; // struct lock_range

} // namespace cielmalloc

#endif // CIELMALLOC_RANGE_LOCK_RANGE_HPP_
