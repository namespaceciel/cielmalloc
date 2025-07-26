#ifndef CIELMALLOC_STATS_RANGE_HPP_
#define CIELMALLOC_STATS_RANGE_HPP_

#include <ciel/core/config.hpp>
#include <ciel/core/message.hpp>
#include <cielmalloc/config.hpp>

#include <atomic>
#include <cstddef>

namespace cielmalloc {

struct stats_range {
    template<class ParentRange>
    class type : public ParentRange {
    private:
        inline static std::atomic<size_t> current_usage;
        inline static std::atomic<size_t> peak_usage;

    public:
        using Base = ParentRange;

        CIEL_NODISCARD void* alloc_range(size_t size) noexcept {
            void* ptr = Base::alloc_range(size);

            if (ptr) {
                const size_t cur_usage = current_usage.fetch_add(size, std::memory_order_relaxed) + size;

                size_t pu = peak_usage.load(std::memory_order_relaxed);
                while (pu < cur_usage) {
                    if (peak_usage.compare_exchange_weak(pu, cur_usage, std::memory_order_relaxed)) {
                        break;
                    }
                }
            }

            return ptr;
        }

        void dealloc_range(void* ptr, size_t size) noexcept {
            CIEL_UNUSED(current_usage.fetch_sub(size, std::memory_order_relaxed));

            Base::dealloc_range(ptr, size);
        }

        CIEL_NODISCARD static size_t get_current_memory_usage() noexcept {
            return current_usage.load(std::memory_order_relaxed);
        }

        CIEL_NODISCARD static size_t get_peak_memory_usage() noexcept {
            return peak_usage.load(std::memory_order_relaxed);
        }

    }; // class type

}; // struct stats_range

} // namespace cielmalloc

#endif // CIELMALLOC_STATS_RANGE_HPP_
