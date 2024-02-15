#ifndef CIELMALLOC_TEST_SRC_TOOLS_HPP_
#define CIELMALLOC_TEST_SRC_TOOLS_HPP_

#include <cstddef>
#include <cstring>

inline void mess_with_memory(void* p, size_t size) noexcept {
    std::memset(p, 0b11110000, size);
}

#endif // CIELMALLOC_TEST_SRC_TOOLS_HPP_
