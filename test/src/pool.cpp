#include <gtest/gtest.h>

#include <ciel/inplace_vector.hpp>
#include <ciel/test/simple_latch.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/pool.hpp>

#include <atomic>
#include <cstddef>
#include <thread>

using namespace cielmalloc;

namespace {

struct T {
    std::atomic<T*> next;
    alignas(CachelineSize) int i;

}; // struct T

} // namespace

TEST(pool, all) {
    constexpr size_t threads_num    = 64;
    constexpr size_t operations_num = 10000;

    auto& pl = pool<T>::get();

    ciel::inplace_vector<std::thread, threads_num> threads;
    ciel::SimpleLatch go{threads_num};

    for (size_t i = 0; i < threads_num; ++i) {
        threads.unchecked_emplace_back([&] {
            go.arrive_and_wait();

            ciel::inplace_vector<T*, operations_num> iv;
            for (size_t j = 0; j < operations_num; ++j) {
                iv.unchecked_emplace_back(pl.acquire());
                iv.back()->i = 42;
            }

            for (T* p : iv) {
                ASSERT_EQ(p->i, 42);
                pl.release(p);
            }
        });
    }

    for (auto& t : threads) {
        t.join();
    }
}
