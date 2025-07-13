#include <gtest/gtest.h>

#include <ciel/core/message.hpp>
#include <cielmalloc/bits.hpp>
#include <cielmalloc/buddy.hpp>
#include <cielmalloc/pal.hpp>
#include <cielmalloc/pal/flag.hpp>

#include <cstddef>
#include <utility>

#include "tools.hpp"

using namespace cielmalloc;

TEST(buddy, all) {
    static constexpr size_t BitBegin     = 5;
    static constexpr size_t BitEnd       = pal::page_size_bits;
    static constexpr size_t MaxBlockSize = cielmalloc::one_at_bit(BitEnd);

    buddy<RBNodeInplace, BitBegin, BitEnd> buddy;

    auto refill = []() {
        void* ptr = pal::reserve(MaxBlockSize);
        if (ptr) {
            CIEL_ASSERT(pal::commit<NoZero>(ptr, MaxBlockSize));
        }

        return std::make_pair(ptr, MaxBlockSize);
    };

    static constexpr size_t size1 = cielmalloc::one_at_bit(8);
    static constexpr size_t size2 = cielmalloc::one_at_bit(5);

    auto p1 = buddy.allocate_block(size1, refill);
    auto p2 = buddy.allocate_block(size2, refill);

    mess_with_memory(p1, size1);
    mess_with_memory(p2, size2);

    buddy.deallocate_block(p1, size1);
    buddy.deallocate_block(p2, size2);
}
