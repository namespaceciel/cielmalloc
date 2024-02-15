#include <gtest/gtest.h>

#include "tools.hpp"
#include <ciel/core/message.hpp>
#include <ciel/inplace_vector.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/interface.hpp>
#include <cielmalloc/sizeclass.hpp>

#include <cstddef>

TEST(small_alloc, zero) {
    constexpr size_t allocated_size = 0;
    ciel::println("Small alloc: {} bytes...", allocated_size);

    ciel::inplace_vector<void*, 8> iv;
    for (size_t i = 0; i < iv.capacity(); ++i) {
        iv.unchecked_emplace_back(cielmalloc::malloc(allocated_size));
    }

    for (void* p : iv) {
        cielmalloc::free(p);
    }
}

TEST(small_alloc, single_thread) {
    constexpr size_t begin_size = 4;
    constexpr size_t end_size   = cielmalloc::MediumThreshold;

    for (size_t allocated_size = begin_size; allocated_size < end_size; allocated_size *= 1.3) {
        ciel::println("Small alloc: {} bytes... sizeclass: {}", allocated_size,
                      cielmalloc::size_to_sizeclass(allocated_size));

        ciel::inplace_vector<void*, 1024> iv;
        for (size_t i = 0; i < iv.capacity(); ++i) {
            void* p = cielmalloc::malloc(allocated_size);
            iv.unchecked_emplace_back(p);
        }

        for (void* p : iv) {
            mess_with_memory(p, allocated_size);
        }

        for (void* p : iv) {
            cielmalloc::free(p);
        }
    }
}
