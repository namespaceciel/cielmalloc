#ifndef CIELMALLOC_BITS_HPP_
#define CIELMALLOC_BITS_HPP_

#include <ciel/core/config.hpp>
#include <ciel/core/message.hpp>

#include <bit>
#include <cstddef>

namespace cielmalloc {

inline constexpr size_t Bits = sizeof(size_t) * 8;
static_assert(Bits == 64, "Only 64bit system supported");

inline constexpr size_t AddressBits = 48;

template<class T>
CIEL_NODISCARD constexpr T lsb(T x) noexcept {
    return x & -x;
}

template<class T = size_t, class S>
CIEL_NODISCARD constexpr T one_at_bit(S shift) noexcept {
    static_assert(std::is_integral_v<T>);

    CIEL_ASSERT(sizeof(T) * 8 > static_cast<size_t>(shift));

    return static_cast<T>(1) << shift;
}

template<class T = size_t, class S>
CIEL_NODISCARD constexpr T mask_bits(S n) noexcept {
    return one_at_bit<T>(n) - 1;
}

CIEL_NODISCARD inline constexpr size_t next_pow2_bits(size_t x) noexcept {
    CIEL_ASSERT(x != 0);

    return Bits - std::countl_zero(x - 1);
}

CIEL_NODISCARD inline constexpr size_t next_pow2(size_t x) noexcept {
    return cielmalloc::one_at_bit(cielmalloc::next_pow2_bits(x));
}

template<size_t MantissaBits, size_t LowBits = 0>
CIEL_NODISCARD constexpr size_t to_exp_mant(size_t value) noexcept {
    constexpr size_t LeadingBit   = cielmalloc::one_at_bit(MantissaBits + LowBits - 1);
    constexpr size_t MantissaMask = cielmalloc::mask_bits(MantissaBits);

    --value;
    const size_t e   = Bits - MantissaBits - LowBits - std::countl_zero(value | LeadingBit);
    const size_t b   = e == 0 ? 0 : 1;
    const size_t m   = (value >> (LowBits + e - b)) & MantissaMask;
    const size_t res = (e << MantissaBits) + m;

    return res;
}

template<size_t MantissaBits, size_t LowBits = 0>
CIEL_NODISCARD constexpr size_t from_exp_mant(size_t m_e) noexcept {
    if constexpr (MantissaBits == 0) {
        return cielmalloc::one_at_bit(m_e + LowBits);
    }

    constexpr size_t MantissaMask = cielmalloc::mask_bits(MantissaBits);

    ++m_e;
    const size_t m          = m_e & MantissaMask;
    const size_t e          = m_e >> MantissaBits;
    const size_t b          = e == 0 ? 0 : 1;
    const size_t shifted_e  = e - b;
    const size_t extended_m = m + (b << MantissaBits);
    const size_t res        = extended_m << (shifted_e + LowBits);

    return res;
}

} // namespace cielmalloc

#endif // CIELMALLOC_BITS_HPP_
