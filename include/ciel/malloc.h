#ifndef CIELMALLOC_INCLUDE_CIEL_MALLOC_H_
#define CIELMALLOC_INCLUDE_CIEL_MALLOC_H_

#include <ciel/config.h>

#include <cstddef>

CIEL_NODISCARD void* operator new(size_t);

CIEL_NODISCARD void* operator new[](size_t);

void operator delete(void*) noexcept;

void operator delete[](void*) noexcept;

NAMESPACE_CIEL_BEGIN

CIEL_NODISCARD void* malloc(const size_t) noexcept;

void free(void* ptr) noexcept;

NAMESPACE_CIEL_END

#endif // CIELMALLOC_INCLUDE_CIEL_MALLOC_H_