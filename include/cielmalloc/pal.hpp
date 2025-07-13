#ifndef CIELMALLOC_PAL_HPP_
#define CIELMALLOC_PAL_HPP_

#if defined(__APPLE__)
#  include <cielmalloc/pal/apple.hpp>
#  define CIELMALLOC_PAL_APPLE
#elif defined(__linux__)
#  define CIELMALLOC_PAL_LINUX
#  include <cielmalloc/pal/linux.hpp>
#else
#  error "Unsupported platform"
#endif

namespace cielmalloc {

using pal =
#if defined(CIELMALLOC_PAL_APPLE)
    pal_apple;
#elif defined(CIELMALLOC_PAL_LINUX)
    pal_linux;
#endif

template<PalFeatures F, uint64_t U = static_cast<uint64_t>(F)>
inline constexpr bool pal_supports = (pal::pal_features & U) == U;

} // namespace cielmalloc

#endif // CIELMALLOC_PAL_HPP_
