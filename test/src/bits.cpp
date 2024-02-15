#include <gtest/gtest.h>

#include <cielmalloc/bits.hpp>

TEST(bits, next_pow2_bits) {
    static_assert(cielmalloc::next_pow2_bits(1) == 0);
    static_assert(cielmalloc::next_pow2_bits(7) == 3);
    static_assert(cielmalloc::next_pow2_bits(8) == 3);

    ASSERT_EQ(cielmalloc::next_pow2_bits(1), 0);
    ASSERT_EQ(cielmalloc::next_pow2_bits(7), 3);
    ASSERT_EQ(cielmalloc::next_pow2_bits(8), 3);
}

TEST(bits, next_pow2) {
    static_assert(cielmalloc::next_pow2(1) == 1);
    static_assert(cielmalloc::next_pow2(2) == 2);
    static_assert(cielmalloc::next_pow2(3) == 4);

    ASSERT_EQ(cielmalloc::next_pow2(1), 1);
    ASSERT_EQ(cielmalloc::next_pow2(2), 2);
    ASSERT_EQ(cielmalloc::next_pow2(3), 4);
}
