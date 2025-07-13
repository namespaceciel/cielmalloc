#ifndef CIELMALLOC_LARGE_SLAB_HPP_
#define CIELMALLOC_LARGE_SLAB_HPP_

#include <ciel/core/alignment.hpp>
#include <ciel/core/config.hpp>
#include <ciel/core/message.hpp>
#include <ciel/core/treiber_stack.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/sizeclass.hpp>
#include <cielmalloc/slab.hpp>

#include <atomic>

namespace cielmalloc {

// size and alignment:
// [ 16MB,  32MB,  64MB,  128MB,
//   256MB, 512MB, 1GB,   2GB,
//   4GB,   8GB,   16GB,  32GB,
//   64GB,  128GB, 256GB, 512GB,
//   1TB,   2TB,   4TB,   8TB,
//   16TB,  32TB,  64TB,  128TB, 256TB )
class large_slab : public slab_base {
public:
    std::atomic<large_slab*> next; // Used by treiber_stack.

    void init() noexcept {
        kind_ = slab_kind::Large;
    }

}; // class large_slab

} // namespace cielmalloc

#endif // CIELMALLOC_LARGE_SLAB_HPP_
