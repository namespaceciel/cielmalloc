#ifndef CIELMALLOC_AAL_HPP_
#define CIELMALLOC_AAL_HPP_

#if defined(__aarch64__) || defined(_M_ARM64) || defined(__arm__) || defined(_M_ARM)
#  define CIELMALLOC_AAL_ARM
#  include <cielmalloc/aal/arm.hpp>
#elif defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#  define CIELMALLOC_AAL_X86
#  include <cielmalloc/aal/x86.hpp>
#else
#  error "Unsupported architecture"
#endif

namespace cielmalloc {

using aal =
#if defined(CIELMALLOC_AAL_ARM)
    aal_arm;
#elif defined(CIELMALLOC_AAL_X86)
    aal_x86;
#endif

inline constexpr size_t Bits        = sizeof(size_t) * 8;
inline constexpr size_t AddressBits = (Bits == 64) ? 48 : 32;

} // namespace cielmalloc

#endif // CIELMALLOC_AAL_HPP_
