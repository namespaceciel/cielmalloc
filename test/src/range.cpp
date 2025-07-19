#include <gtest/gtest.h>

#include <ciel/core/config.hpp>
#include <ciel/core/pipe.hpp>
#include <ciel/vector.hpp>
#include <cielmalloc/commit_range.hpp>
#include <cielmalloc/lock_range.hpp>
#include <cielmalloc/pal.hpp>
#include <cielmalloc/reserve_range.hpp>

#include <cstddef>
#include <thread>

#include "tools.hpp"

using namespace cielmalloc;

TEST(range, lock_range) {
    struct Increment {
        size_t a = 0;
        size_t b = 0;

        void* alloc_range(size_t) noexcept {
            ++a;

            return nullptr;
        }

        void dealloc_range(void*, size_t) noexcept {
            ++b;
        }
    };

    using Range = ciel::pipe<Increment, lock_range>;

    Range range;
    ciel::vector<std::thread> threads(ciel::reserve_capacity, 1000);
    for (size_t i = 0; i < threads.capacity(); ++i) {
        threads.emplace_back([&]() noexcept {
            CIEL_UNUSED(range.alloc_range(1));
            range.dealloc_range(nullptr, 1);
        });
    }

    for (auto& t : threads) {
        t.join();
    }

    ASSERT_EQ(range.a, 1000);
    ASSERT_EQ(range.b, 1000);
}

TEST(range, reserve_and_commit) {
    using Range = ciel::pipe<reserve_range, commit_range>;

    Range range;

    constexpr size_t size = pal::page_size;
    void* p               = range.alloc_range(size);

    mess_with_memory(p, size);

    range.dealloc_range(p, size);
}
