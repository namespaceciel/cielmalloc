#ifndef CIELMALLOC_PAL_LINUX_HPP_
#define CIELMALLOC_PAL_LINUX_HPP_

#if defined(__linux__)

#  include <ciel/core/alignment.hpp>
#  include <ciel/core/config.hpp>
#  include <ciel/core/message.hpp>
#  include <cielmalloc/aal.hpp>
#  include <cielmalloc/pal/flag.hpp>

#  include <sys/mman.h>
#  include <sys/prctl.h>
#  include <syscall.h>

namespace cielmalloc {

struct pal_impl {
    static constexpr uint64_t pal_features = AlignedAllocation | LazyCommit | Entropy;

    static constexpr size_t page_size =
#  if defined(PAGESIZE)
        ciel::max(aal::smallest_page_size, static_cast<size_t>(PAGESIZE));
#  else
        aal::smallest_page_size;
#  endif

    static constexpr int madvise_free_flags =
#  if defined(MADV_FREE)
        MADV_FREE
#  else
        MADV_DONTNEED
#  endif
        ;

    CIEL_NODISCARD static void* reserve(size_t size) noexcept {
        CIEL_ASSERT(ciel::is_pow2(size));

        auto prot = PROT_NONE;

        void* p = mmap(nullptr, size, prot, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);

        if (p != MAP_FAILED) {
            return p;
        }

        return nullptr;
    }

    template<ZeroMem NeedZeroMem>
    CIEL_NODISCARD static bool commit(void* p, size_t size) noexcept {
        CIEL_ASSERT(p != nullptr);
        CIEL_ASSERT(ciel::is_aligned(p, page_size));
        CIEL_ASSERT(ciel::is_aligned(p, size));

        mprotect(p, size, PROT_READ | PROT_WRITE);

        if constexpr (NeedZeroMem == YesZero) {
            zero(p, size);
        }

        return true;
    }

    static void decommit(void* p, size_t size) noexcept {
        CIEL_ASSERT(p != nullptr);
        CIEL_ASSERT(ciel::is_aligned(p, page_size));
        CIEL_ASSERT(ciel::is_aligned(p, size));

        const ciel::keep_errno ke;

#  ifdef CIEL_IS_DEBUGGING
        // We need to make sure decommitted size is no less than initially committed size.
        std::memset(p, 0b10101010, size);
#  endif

        madvise(p, size, madvise_free_flags);

        mprotect(p, size, PROT_NONE);
    }

    static void zero(void* p, size_t size) noexcept {
        std::memset(p, 0, size);
    }

}; // struct pal_impl

} // namespace cielmalloc

#endif

#endif // CIELMALLOC_PAL_LINUX_HPP_
