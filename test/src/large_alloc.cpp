#include <gtest/gtest.h>

#include "tools.hpp"
#include <ciel/core/message.hpp>
#include <ciel/inplace_vector.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/interface.hpp>

#include <cstddef>
#include <cstring>

TEST(large_alloc, single_thread) {
    constexpr size_t begin_size = cielmalloc::LargeThreshold;
    constexpr size_t end_size   = cielmalloc::one_at_bit(27);

    for (size_t allocated_size = begin_size; allocated_size < end_size; allocated_size *= 1.3) {
        CIELMALLOC_LOG("UnitTest: Large alloc: {} bytes...", allocated_size);

        ciel::inplace_vector<void*, 1> iv;
        for (size_t i = 0; i < iv.capacity(); ++i) {
            iv.unchecked_emplace_back(cielmalloc::malloc(allocated_size));
        }

        for (void* p : iv) {
            mess_with_memory(p, allocated_size);
        }

        for (void* p : iv) {
            cielmalloc::free(p);
        }
    }
}
