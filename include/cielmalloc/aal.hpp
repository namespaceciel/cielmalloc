#ifndef CIELMALLOC_AAL_HPP_
#define CIELMALLOC_AAL_HPP_

#if defined(__aarch64__) || defined(_M_ARM64) || defined(__arm__) || defined(_M_ARM)
#  include <cielmalloc/aal/arm.hpp>
#elif defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#  include <cielmalloc/aal/x86.hpp>
#else
#  error "Unsupported architecture"
#endif

namespace cielmalloc {

using aal = aal_impl;

} // namespace cielmalloc

#endif // CIELMALLOC_AAL_HPP_
