#ifndef CIELMALLOC_INCLUDE_CIELMALLOC_PAL_APPLE_HPP_
#define CIELMALLOC_INCLUDE_CIELMALLOC_PAL_APPLE_HPP_

#include <ciel/core/alignment.hpp>
#include <ciel/core/config.hpp>
#include <ciel/core/message.hpp>
#include <cielmalloc/config.hpp>

#include <CommonCrypto/CommonRandom.h>
#include <bit>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <errno.h>
#include <mach/mach_init.h>
#include <mach/mach_vm.h>
#include <mach/vm_statistics.h>
#include <mach/vm_types.h>
#include <sys/mman.h>
#include <unistd.h>

namespace cielmalloc {

// TODO: unmap

struct pal {
    static constexpr int anonymous_memory_fd       = int(VM_MAKE_TAG(uint32_t(241)));
    static constexpr int default_mach_vm_map_flags = int(VM_MAKE_TAG(uint32_t(241)));

    template<bool commit = false>
    CIEL_NODISCARD static void* reserve(size_t size) noexcept {
        CIEL_ASSERT(ciel::is_pow2(size));
        CIEL_ASSERT(size >= OSPageSize);

        mach_vm_offset_t mask = size - 1;

        int flags = VM_FLAGS_ANYWHERE | default_mach_vm_map_flags;

        mach_vm_address_t addr = 0;

        vm_prot_t prot = commit ? VM_PROT_READ | VM_PROT_WRITE : VM_PROT_NONE;

        kern_return_t kr = mach_vm_map(mach_task_self(), &addr, size, mask, flags, MEMORY_OBJECT_NULL, 0, TRUE, prot,
                                       VM_PROT_READ | VM_PROT_WRITE, VM_INHERIT_COPY);

        if CIEL_UNLIKELY (kr != KERN_SUCCESS) {
            return nullptr;
        }

        void* res = reinterpret_cast<void*>(addr);
        // zero(res, size);
        return res;
    }

    static void commit(void* p, size_t size) noexcept {
        CIEL_ASSERT(ciel::is_aligned(p, OSPageSize));
        CIEL_ASSERT(p != nullptr);

        const auto res = mprotect(p, size, PROT_READ | PROT_WRITE);

        CIEL_ASSERT(res == 0);
        CIEL_UNUSED(res);
    }

    static void decommit(void* p, size_t size) noexcept {
        CIEL_ASSERT(ciel::is_aligned(p, OSPageSize));
        CIEL_ASSERT(p != nullptr);

#ifdef CIEL_IS_DEBUGGING
        // std::memset(p, 0b10101010, size); // decommitted size could be larger than initially committed size.
#endif

        const auto res = madvise(p, size, MADV_DONTNEED);

        CIEL_ASSERT(res == 0);
        CIEL_UNUSED(res);
    }

    void zero(void* p, size_t size) noexcept {
        std::memset(p, 0, size);
    }

}; // struct pal

} // namespace cielmalloc

#endif // CIELMALLOC_INCLUDE_CIELMALLOC_PAL_APPLE_HPP_
