#ifndef CIELMALLOC_INCLUDE_CIELMALLOC_SIZECLASS_HPP_
#define CIELMALLOC_INCLUDE_CIELMALLOC_SIZECLASS_HPP_

#include <ciel/core/message.hpp>
#include <cielmalloc/bits.hpp>
#include <cielmalloc/config.hpp>

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace cielmalloc {

inline constexpr uint8_t size_to_sizeclass(size_t size) noexcept {
    const size_t res = cielmalloc::to_exp_mant<IntermediateBits, MinAllocBits>(size);

    CIEL_ASSERT(res <= std::numeric_limits<uint8_t>::max());

    return static_cast<uint8_t>(res);
}

// Small:
// [ 16B,    32B,   48B,    64B,  80B,  96B,  112B,  128B,
//   160B,   192B,  224B,   256B,
//   320B,   384B,  448B,   512B,
//   640B,   768B,  896B,   1KB
//   1.25KB, 1.5KB, 1.75KB, 2KB,
//   2.5KB,  3KB,   3.5KB,  4KB,
//   5KB,    6KB,   7KB,    8KB,
//   10KB,   12KB,  14KB,   16KB,
//   20KB,   24KB,  28KB,   32KB,
//   40KB,   48KB,  56KB,   64KB )
//
// Medium:
// [ 64KB,   80KB,  96KB,   112KB,  128KB,
//   160KB,  192KB, 224KB,  256KB,
//   320KB,  384KB, 448KB,  512KB,
//   640KB,  768KB, 896KB,  1MB,
//   1.25MB, 1.5MB, 1.75MB, 2MB,
//   2.5MB,  3MB,   3.5MB,  4MB,
//   5MB,    6MB,   7MB,    8MB,
//   10MB,   12MB,  14MB,   16MB )
//
// Large:
// [ 16MB,  32MB,  64MB,  128MB,
//   256MB, 512MB, 1GB,   2GB,
//   4GB,   8GB,   16GB,  32GB,
//   64GB,  128GB, 256GB, 512GB,
//   1TB,   2TB,   4TB,   8TB,
//   16TB,  32TB,  64TB,  128TB, 256TB )

inline constexpr uint8_t NumSizeclasses = cielmalloc::size_to_sizeclass(LargeThreshold);   // 75

inline constexpr uint8_t NumSmallClasses = cielmalloc::size_to_sizeclass(MediumThreshold); // 43

inline constexpr uint8_t NumMediumClasses = NumSizeclasses - NumSmallClasses;              // 32

inline constexpr uint8_t NumLargeClasses = AddressBits - LargeThresholdBits;               // 24

struct sizeclass_table {
    // sizeclass -> size
    std::array<size_t, NumSizeclasses> size{};
    // sizeclass -> alignment
    std::array<size_t, NumSizeclasses> alignment{};
    // how many objects can a 64KB slab allocate for each small sizeclass
    std::array<uint16_t, NumSmallClasses> small_slab_slots{};
    // how many objects can a 16MB slab allocate for each medium sizeclass
    std::array<uint8_t, NumMediumClasses> medium_slab_slots{};

    constexpr sizeclass_table() noexcept {
        for (uint8_t sizeclass = 0; sizeclass < NumSizeclasses; ++sizeclass) {
            size[sizeclass]      = cielmalloc::from_exp_mant<IntermediateBits, MinAllocBits>(sizeclass);
            alignment[sizeclass] = cielmalloc::lsb(size[sizeclass]);
        }

        for (uint8_t i = 0; i < NumSmallClasses; ++i) {
            small_slab_slots[i] = static_cast<uint16_t>(MediumThreshold / size[i]);
        }

        for (uint8_t i = NumSmallClasses; i < NumSizeclasses; ++i) {
            medium_slab_slots[i - NumSmallClasses] = static_cast<uint8_t>((LargeThreshold - OSPageSize) / size[i]);
        }
    }

}; // struct sizeclass_table

inline constexpr sizeclass_table sizeclass_metadata{};

inline constexpr size_t sizeclass_to_size(uint8_t sizeclass) noexcept {
    return sizeclass_metadata.size[sizeclass];
}

static_assert(MinAllocSize == sizeclass_to_size(0));

inline constexpr size_t sizeclass_to_alignment(uint8_t sizeclass) noexcept {
    return sizeclass_metadata.alignment[sizeclass];
}

inline constexpr uint16_t small_slab_slots(uint8_t sizeclass) noexcept {
    return sizeclass_metadata.small_slab_slots[sizeclass];
}

inline constexpr uint8_t medium_slab_slots(uint8_t sizeclass) noexcept {
    return sizeclass_metadata.medium_slab_slots[sizeclass - NumSmallClasses];
}

} // namespace cielmalloc

#endif // CIELMALLOC_INCLUDE_CIELMALLOC_SIZECLASS_HPP_
