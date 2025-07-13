#ifndef CIELMALLOC_AAL_ARM_HPP_
#define CIELMALLOC_AAL_ARM_HPP_

#if defined(__aarch64__) || defined(_M_ARM64) || defined(__arm__) || defined(_M_ARM)

#  include <cielmalloc/aal/flag.hpp>

#  include <cstddef>

namespace cielmalloc {

struct aal_arm {
    static constexpr enum AalName aal_name = ARM;

    static constexpr size_t smallest_page_size = 0x1000;

}; // struct aal_arm

} // namespace cielmalloc

#endif

#endif // CIELMALLOC_AAL_ARM_HPP_
