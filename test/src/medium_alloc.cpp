#include <gtest/gtest.h>

#include "tools.hpp"
#include <ciel/core/message.hpp>
#include <ciel/inplace_vector.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/interface.hpp>
#include <cielmalloc/sizeclass.hpp>

#include <cstddef>
#include <cstring>

TEST(medium_alloc, single_thread) {
    constexpr size_t begin_size = cielmalloc::MediumThreshold;
    constexpr size_t end_size   = cielmalloc::LargeThreshold;

    for (size_t allocated_size = begin_size; allocated_size < end_size; allocated_size *= 1.3) {
        ciel::println("Medium alloc: {} bytes... sizeclass: {}", allocated_size,
                      cielmalloc::size_to_sizeclass(allocated_size));

        ciel::inplace_vector<void*, 256> iv;
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
