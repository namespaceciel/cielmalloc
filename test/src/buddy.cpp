#include <gtest/gtest.h>

#include <cielmalloc/bits.hpp>
#include <cielmalloc/buddy.hpp>

#include <cstddef>

TEST(buddy, all) {
    constexpr size_t size = cielmalloc::one_at_bit(10);
    alignas(size) unsigned char buffer[size];

    cielmalloc::buddy<5, 11> buddy;

    buddy.deallocate_block(buffer, size);

    auto p1 = buddy.allocate_block(cielmalloc::one_at_bit(8));
    auto p2 = buddy.allocate_block(cielmalloc::one_at_bit(5));
    buddy.deallocate_block(p1, cielmalloc::one_at_bit(8));
    buddy.deallocate_block(p2, cielmalloc::one_at_bit(5));

    ASSERT_EQ(buddy.allocate_block(size), buffer);
}
