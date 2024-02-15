#ifndef CIELMALLOC_INCLUDE_CIELMALLOC_PAL_HPP_
#define CIELMALLOC_INCLUDE_CIELMALLOC_PAL_HPP_

#define CIELMALLOC_PAL_DEBUGGING true // TODO

#if CIELMALLOC_PAL_DEBUGGING
#  include <cielmalloc/pal/debug.hpp>
#elif defined(__APPLE__)
#  include <cielmalloc/pal/apple.hpp>
#else
#  error "Unsupported platform"
#endif

#endif // CIELMALLOC_INCLUDE_CIELMALLOC_PAL_HPP_
