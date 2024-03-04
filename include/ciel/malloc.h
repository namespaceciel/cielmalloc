#ifndef CIELMALLOC_INCLUDE_CIEL_MALLOC_H_
#define CIELMALLOC_INCLUDE_CIEL_MALLOC_H_

#include <ciel/config.h>

NAMESPACE_CIEL_BEGIN

void* malloc(const size_t size) noexcept;

void free(void* ptr) noexcept;

NAMESPACE_CIEL_END

#endif // CIELMALLOC_INCLUDE_CIEL_MALLOC_H_