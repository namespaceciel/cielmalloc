#ifndef CIELMALLOC_AAL_X86_HPP_
#define CIELMALLOC_AAL_X86_HPP_

#if !defined(__x86_64__) && !defined(_M_X64) && !defined(__i386__) && !defined(_M_IX86)
#  error "Unmatched architecture"
#endif

#include <cielmalloc/aal/flag.hpp>

#include <cstddef>

namespace cielmalloc {

struct aal {
    static constexpr enum AalName aal_name = X86;

    static constexpr size_t smallest_page_size = 0x1000;

}; // struct aal

} // namespace cielmalloc

#endif // CIELMALLOC_AAL_X86_HPP_
