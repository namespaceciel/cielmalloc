#ifndef CIELMALLOC_INCLUDE_CIEL_SIZECLASS_H_
#define CIELMALLOC_INCLUDE_CIEL_SIZECLASS_H_

#include <ciel/config.h>
#include <ciel/system.h>

#include <cstddef>

NAMESPACE_CIEL_BEGIN

// 16B - 1KB  --- 16B  aligned --- 64
// 1KB - 8KB  --- 128B aligned --- 56
// 8KB - 63KB --- 1KB  aligned --- 55
// 175 sizeclass in total

static constexpr size_t FreelistNum = 176;    // except for index 0

size_t size_to_sizeclass(size_t) noexcept;

size_t sizeclass_to_size(const size_t) noexcept;

size_t size_to_alignment(const size_t) noexcept;

NAMESPACE_CIEL_END

#endif // CIELMALLOC_INCLUDE_CIEL_SIZECLASS_H_