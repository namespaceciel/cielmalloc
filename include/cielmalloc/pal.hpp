#ifndef CIELMALLOC_PAL_HPP_
#define CIELMALLOC_PAL_HPP_

#if defined(__APPLE__)
#  include <cielmalloc/pal/apple.hpp>
#elif defined(__linux__)
#  include <cielmalloc/pal/linux.hpp>
#else
#  error "Unsupported platform"
#endif

namespace cielmalloc {

using pal = pal_impl;

} // namespace cielmalloc

#endif // CIELMALLOC_PAL_HPP_
