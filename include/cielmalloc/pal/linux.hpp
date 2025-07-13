#ifndef CIELMALLOC_PAL_LINUX_HPP_
#define CIELMALLOC_PAL_LINUX_HPP_

#if defined(__linux__)

#  include <ciel/core/alignment.hpp>
#  include <ciel/core/config.hpp>
#  include <ciel/core/message.hpp>
#  include <cielmalloc/aal.hpp>
#  include <cielmalloc/bits.hpp>
#  include <cielmalloc/pal/flag.hpp>
#  include <cielmalloc/pal/posix.hpp>

#  include <sys/mman.h>
#  include <sys/prctl.h>
#  include <syscall.h>

namespace cielmalloc {

struct pal_linux : pal_posix<pal_linux> {
    static constexpr int default_mmap_flags = MAP_NORESERVE;

    static constexpr int madvise_free_flags =
#  if defined(MADV_FREE)
        MADV_FREE
#  else
        MADV_DONTNEED
#  endif
        ;

    static constexpr uint64_t pal_features = pal_posix::pal_features | Entropy;

    static constexpr size_t page_size_bits = cielmalloc::next_pow2_bits(page_size);

    static void decommit(void* p, size_t size) noexcept {
        CIEL_ASSERT(p != nullptr);
        CIEL_ASSERT(ciel::is_aligned(p, page_size));
        CIEL_ASSERT(ciel::is_aligned(size, page_size));

        const ciel::keep_errno ke;

#  ifdef CIEL_IS_DEBUGGING
        // We need to make sure decommitted size is no less than initially committed size.
        std::memset(p, 0b10101010, size);
#  endif

        madvise(p, size, madvise_free_flags);

        mprotect(p, size, PROT_NONE);
    }

    template<bool AssumePageAligned>
    static void zero(void* p, size_t size) noexcept {
        if ((AssumePageAligned || (ciel::is_aligned(p, page_size) && ciel::is_aligned(size, page_size)))
            && size > 16 * page_size) {
            CIEL_ASSERT(ciel::is_aligned(p, page_size));
            CIEL_ASSERT(ciel::is_aligned(size, page_size));

            madvise(p, size, MADV_DONTNEED);

        } else {
            bzero(p, size);
        }
    }

}; // struct pal_linux

} // namespace cielmalloc

#endif

#endif // CIELMALLOC_PAL_LINUX_HPP_
