#include <gtest/gtest.h>

#include <ciel/core/pipe.hpp>
#include <ciel/inplace_vector.hpp>
#include <cielmalloc/bits.hpp>
#include <cielmalloc/commit_range.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/large_buddy_range.hpp>
#include <cielmalloc/lock_range.hpp>
#include <cielmalloc/pagemap_register_range.hpp>
#include <cielmalloc/reserve_range.hpp>
#include <cielmalloc/small_buddy_range.hpp>

#include <cstddef>
#include <functional>
#include <thread>

#include "tools.hpp"

using namespace cielmalloc;

TEST(range, all) {
    using Range = ciel::pipe<reserve_range, pagemap_register_range, large_buddy_range, lock_range, commit_range,
                             small_buddy_range>;
    Range range;

    ciel::inplace_vector<std::thread, 32> threads;
    for (size_t i = 0; i < threads.capacity(); ++i) {
        threads.unchecked_emplace_back([&]() noexcept {
            for (size_t size = 32; size <= cielmalloc::one_at_bit(25); size *= 2) {
                void* p = range.alloc_range(size);
                if (p) {
                    mess_with_memory(p, size);
                    range.dealloc_range(p, size);

                } else {
                    static thread_local auto id = std::this_thread::get_id();
                    CIELMALLOC_LOG("Allocate failed, thread_id is {}, size is {}", std::hash<decltype(id)>{}(id), size);
                }
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    // for (size_t size = 32; size <= pal::page_size * 2; size *= 2) {
    //     void* p = range.alloc_range(size);
    //     if (p) {
    //         mess_with_memory(p, size);
    //         range.dealloc_range(p, size);

    // } else {
    //     static thread_local auto id = std::this_thread::get_id();
    //     CIELMALLOC_LOG("Allocate failed, thread_id is {}, size is {}", std::hash<decltype(id)>{}(id), size);
    // }
    // }
}
