#ifndef CIELMALLOC_PAL_POSIX_HPP_
#define CIELMALLOC_PAL_POSIX_HPP_

#include <ciel/core/alignment.hpp>
#include <ciel/core/config.hpp>
#include <ciel/core/message.hpp>
#include <cielmalloc/aal.hpp>
#include <cielmalloc/bits.hpp>
#include <cielmalloc/pal/flag.hpp>

#include <sys/mman.h>
#include <sys/uio.h>
#include <unistd.h>

namespace cielmalloc {

template<class OS>
struct pal_posix {
    static constexpr int DefaultMMAPFlags() noexcept {
        if constexpr (requires { OS::default_mmap_flags; }) {
            return OS::default_mmap_flags;
        }

        return 0;
    }

    static constexpr int AnonFD() noexcept {
        if constexpr (requires { OS::anonymous_memory_fd; }) {
            return OS::anonymous_memory_fd;
        }

        return -1;
    }

    static int extra_mmap_flags(bool) noexcept {
        return 0;
    }

    static constexpr uint64_t pal_features = LazyCommit;

    static constexpr size_t page_size =
#if defined(PAGESIZE)
        ciel::max(aal::smallest_page_size, static_cast<size_t>(PAGESIZE));
#else
        aal::smallest_page_size;
#endif

    CIEL_NODISCARD static void* reserve(size_t size) noexcept {
        auto prot = PROT_NONE;

        void* p = mmap(nullptr, size, prot,
                       MAP_PRIVATE | MAP_ANONYMOUS | DefaultMMAPFlags() | OS::extra_mmap_flags(false), AnonFD(), 0);

        if CIEL_LIKELY (p != MAP_FAILED) {
            return p;
        }

        return nullptr;
    }

    CIEL_NODISCARD static void* reserve_aligned(size_t size) noexcept {
        CIEL_ASSERT_M(ciel::is_pow2(size), "pal::reserve failed, size {} is not pow2", size);

        const size_t alloc_size = size * 2 - OS::page_size;
        void* p                 = OS::reserve(alloc_size);

        const uintptr_t addr         = reinterpret_cast<uintptr_t>(p);
        const uintptr_t aligned_addr = (addr + size - 1) & ~(size - 1);
        const size_t head            = aligned_addr - addr;
        const size_t tail            = alloc_size - head - size;

        if CIEL_LIKELY (head > 0) {
            munmap(p, head);
        }

        if CIEL_LIKELY (tail > 0) {
            munmap(reinterpret_cast<void*>(aligned_addr + size), tail);
        }

        return reinterpret_cast<void*>(aligned_addr);
    }

    template<ZeroMem NeedZeroMem>
    CIEL_NODISCARD static bool commit(void* p, size_t size) noexcept {
        CIEL_ASSERT(p != nullptr);
        CIEL_ASSERT(ciel::is_aligned(p, OS::page_size));
        CIEL_ASSERT(ciel::is_aligned(size, OS::page_size));

        mprotect(p, size, PROT_READ | PROT_WRITE);

        if constexpr (NeedZeroMem == YesZero) {
            zero<true>(p, size);
        }

        return true;
    }

    CIEL_NODISCARD static bool commit_readonly(void* p, size_t size) noexcept {
        CIEL_ASSERT(p != nullptr);
        CIEL_ASSERT(ciel::is_aligned(p, OS::page_size));
        CIEL_ASSERT(ciel::is_aligned(size, OS::page_size));

        mprotect(p, size, PROT_READ);

        return true;
    }

    static void decommit(void* p, size_t size) noexcept {
        CIEL_ASSERT(p != nullptr);
        CIEL_ASSERT(ciel::is_aligned(p, OS::page_size));
        CIEL_ASSERT(ciel::is_aligned(size, OS::page_size));

#ifdef CIEL_IS_DEBUGGING
        // We need to make sure decommitted size is no less than initially committed size.
        std::memset(p, 0b10101010, size);
#endif

        mprotect(p, size, PROT_NONE);
    }

    template<bool AssumePageAligned>
    static void zero(void* p, size_t size) noexcept {
        CIEL_ASSERT(p != nullptr);

        if CIEL_LIKELY (AssumePageAligned
                        || (ciel::is_aligned(p, OS::page_size) && ciel::is_aligned(size, OS::page_size))) {
            CIEL_ASSERT(ciel::is_aligned(p, OS::page_size));
            CIEL_ASSERT(ciel::is_aligned(size, OS::page_size));

            const ciel::keep_errno ke;

            void* r = mmap(p, size, PROT_READ | PROT_WRITE,
                           MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED | DefaultMMAPFlags(), AnonFD(), 0);

            if (r != MAP_FAILED) {
                return;
            }
        }

        bzero(p, size);
    }

}; // struct pal_posix

} // namespace cielmalloc

#endif // CIELMALLOC_PAL_POSIX_HPP_
