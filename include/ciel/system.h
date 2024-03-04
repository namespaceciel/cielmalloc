#ifndef CIELMALLOC_INCLUDE_CIEL_SYSTEMALLOC_H_
#define CIELMALLOC_INCLUDE_CIEL_SYSTEMALLOC_H_

#include <ciel/config.h>

#include <cstddef>

#ifndef _WIN32
#error "So far only Windows is supported."
#endif

#include <Windows.h>

NAMESPACE_CIEL_BEGIN

// To indicate if this address is allocated by CielMalloc.
static constexpr size_t MagicNumber = 0x9674182612758621;

void*& ptr_next(void* ptr) noexcept;

// Is a pointer aligned?
bool is_aligned(void* ptr, const size_t alignment) noexcept;

// Align upwards
uintptr_t align_up(uintptr_t sz, const size_t alignment) noexcept;

// Align downwards
uintptr_t align_down(uintptr_t sz, const size_t alignment) noexcept;

// We use 64KB as one page(span).
static constexpr size_t PAGE_SHIFT = 16;

void* SystemAlloc(size_t page_num) noexcept;

void SystemFree(void* ptr) noexcept;

NAMESPACE_CIEL_END

#endif // CIELMALLOC_INCLUDE_CIEL_SYSTEMALLOC_H_