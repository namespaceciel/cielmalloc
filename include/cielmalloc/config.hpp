#ifndef CIELMALLOC_INCLUDE_CIELMALLOC_CONFIG_HPP_
#define CIELMALLOC_INCLUDE_CIELMALLOC_CONFIG_HPP_

#include <ciel/core/config.hpp>
#include <cielmalloc/bits.hpp>

#include <cstddef>
#include <cstdint>

namespace cielmalloc {

inline constexpr size_t IntermediateBits = 2;

inline constexpr size_t ReserveMultiple = 16;

inline constexpr size_t CachelineSize = ciel::cacheline_size;

inline constexpr size_t OSPageSize = 0x1000; // 64KB
#ifdef PAGE_SIZE
static_assert(PAGE_SIZE == OSPageSize, "Page size from system header does not match cielmalloc config page size");
#endif

inline constexpr size_t MinAllocBits = 4;
inline constexpr size_t MinAllocSize = cielmalloc::one_at_bit(MinAllocBits); // 16

inline constexpr size_t MediumThresholdBits = 16;
inline constexpr size_t MediumThreshold     = cielmalloc::one_at_bit(MediumThresholdBits); // 64KB
inline constexpr uintptr_t MediumMask       = ~cielmalloc::mask_bits(MediumThresholdBits);
static_assert(MediumMask == 0xffff'ffff'ffff'0000);

inline constexpr size_t LargeThresholdBits = 24;
inline constexpr size_t LargeThreshold     = cielmalloc::one_at_bit(LargeThresholdBits); // 16MB
inline constexpr uintptr_t LargeMask       = ~cielmalloc::mask_bits(LargeThresholdBits);
static_assert(LargeMask == 0xffff'ffff'ff00'0000);

} // namespace cielmalloc

#ifdef CIELMALLOC_LOG_ON
#  include <ciel/core/message.hpp>
#  define CIELMALLOC_LOG(...) ciel::println(__VA_ARGS__)
#else
#  define CIELMALLOC_LOG(...)
#endif

#endif // CIELMALLOC_INCLUDE_CIELMALLOC_CONFIG_HPP_
