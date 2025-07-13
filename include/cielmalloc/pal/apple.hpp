#ifndef CIELMALLOC_PAL_APPLE_HPP_
#define CIELMALLOC_PAL_APPLE_HPP_

#if defined(__APPLE__)

#  include <ciel/core/alignment.hpp>
#  include <ciel/core/config.hpp>
#  include <ciel/core/message.hpp>
#  include <cielmalloc/aal.hpp>
#  include <cielmalloc/bits.hpp>
#  include <cielmalloc/pal/flag.hpp>
#  include <cielmalloc/pal/posix.hpp>

#  include <CommonCrypto/CommonRandom.h>
#  include <bit>
#  include <cstddef>
#  include <cstdint>
#  include <cstdio>
#  include <cstdlib>
#  include <cstring>
#  include <errno.h>
#  include <mach/mach_init.h>
#  include <mach/mach_vm.h>
#  include <mach/vm_statistics.h>
#  include <mach/vm_types.h>
#  include <sys/mman.h>
#  include <unistd.h>

namespace cielmalloc {

struct pal_apple : pal_posix<pal_apple> {
    static constexpr int PALAnonDefaultID          = 241;
    static constexpr int anonymous_memory_fd       = static_cast<int>(VM_MAKE_TAG(uint32_t(PALAnonDefaultID)));
    static constexpr int default_mach_vm_map_flags = static_cast<int>(VM_MAKE_TAG(uint32_t(PALAnonDefaultID)));

    static constexpr uint64_t pal_features = AlignedAllocation | LazyCommit | Entropy;

    static constexpr size_t page_size      = (aal::aal_name == ARM) ? 0x4000 : 0x1000;
    static constexpr size_t page_size_bits = cielmalloc::next_pow2_bits(page_size);

    static constexpr size_t minimum_alloc_size = page_size;

    CIEL_NODISCARD static void* reserve_aligned(size_t size) noexcept {
        CIEL_ASSERT_M(ciel::is_pow2(size), "pal::reserve failed, size {} is not pow2", size);
        CIEL_ASSERT_M(size >= minimum_alloc_size, "pal::reserve failed, size {} is less than minimum_alloc_size {}",
                      size, minimum_alloc_size);

        const mach_vm_offset_t mask = size - 1;

        const int flags = VM_FLAGS_ANYWHERE | default_mach_vm_map_flags;

        mach_vm_address_t addr = 0;

        const vm_prot_t prot = VM_PROT_NONE;

        const kern_return_t kr = mach_vm_map(mach_task_self(), &addr, size, mask, flags, MEMORY_OBJECT_NULL, 0, TRUE,
                                             prot, VM_PROT_READ | VM_PROT_WRITE, VM_INHERIT_COPY);

        if CIEL_UNLIKELY (kr != KERN_SUCCESS) {
            return nullptr;
        }

        return reinterpret_cast<void*>(addr);
    }

    template<ZeroMem NeedZeroMem>
    CIEL_NODISCARD static bool commit(void* p, size_t size) noexcept {
        CIEL_ASSERT(p != nullptr);
        CIEL_ASSERT(ciel::is_aligned(p, page_size));
        CIEL_ASSERT(ciel::is_aligned(size, page_size));

        const ciel::keep_errno ke;

        if constexpr (NeedZeroMem == YesZero) {
            void* r =
                mmap(p, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED, anonymous_memory_fd, 0);

            if CIEL_LIKELY (r != MAP_FAILED) {
                return true;
            }
        }

        mprotect(p, size, PROT_READ | PROT_WRITE);

        while (madvise(p, size, MADV_FREE_REUSE) == -1 && errno == EAGAIN) {}

        if constexpr (NeedZeroMem == YesZero) {
            zero<true>(p, size);
        }

        return true;
    }

    static void decommit(void* p, size_t size) noexcept {
        CIEL_ASSERT(p != nullptr);
        CIEL_ASSERT(ciel::is_aligned(p, page_size));
        CIEL_ASSERT(ciel::is_aligned(size, page_size));

#  ifdef CIEL_IS_DEBUGGING
        // We need to make sure decommitted size is no less than initially committed size.
        std::memset(p, 0b10101010, size);
#  endif

        while (madvise(p, size, MADV_FREE_REUSABLE) == -1 && errno == EAGAIN) {}

        mprotect(p, size, PROT_NONE);
    }

    CIEL_NODISCARD static uint64_t get_entropy64() noexcept {
        uint64_t result = 0;

        CIEL_ASSERT_M(CCRandomGenerateBytes(reinterpret_cast<void*>(&result), sizeof(result)) == kCCSuccess,
                      "Failed to get system randomness");

        return result;
    }

}; // struct pal_apple

} // namespace cielmalloc

#endif

#endif // CIELMALLOC_PAL_APPLE_HPP_
