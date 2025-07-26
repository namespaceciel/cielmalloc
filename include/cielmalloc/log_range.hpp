#ifndef CIELMALLOC_LOG_RANGE_HPP_
#define CIELMALLOC_LOG_RANGE_HPP_

#include <ciel/core/config.hpp>
#include <cielmalloc/config.hpp>

#include <cstddef>

#if __has_include(<cxxabi.h>)
#  define CIELMALLOC_HAS_CXXABI_H
#  include <cxxabi.h>
#endif

namespace cielmalloc {

#ifdef CIELMALLOC_HAS_CXXABI_H
CIEL_NODISCARD inline const char* demangle(const char* name, char* buffer, size_t size) noexcept {
    int status = 0;
    char* res  = abi::__cxa_demangle(name, buffer, &size, &status);

    if CIEL_UNLIKELY (status != 0) {
        return name;
    }

    return res;
}
#else
CIEL_NODISCARD inline const char* demangle(const char* name, char*, size_t) noexcept {
    return name;
}
#endif

struct log_range {
    template<class ParentRange>
    class type : public ParentRange {
    private:
        inline static char buffer[256];
        inline static const char* name =
#ifdef CIEL_HAS_RTTI
            demangle(typeid(ParentRange).name(), buffer, sizeof(buffer));
#else
            "Unknown";
#endif

    public:
        using Base = ParentRange;

        CIEL_NODISCARD void* alloc_range(size_t size) noexcept {
            CIELMALLOC_LOG("In {}::alloc_range,\nsize: {}", name, size);

            void* ptr = Base::alloc_range(size);

            CIELMALLOC_LOG("In {}::alloc_range,\nreturned ptr: {}", name, ptr);

            return ptr;
        }

        void dealloc_range(void* ptr, size_t size) noexcept {
            CIELMALLOC_LOG("In {}::dealloc_range,\nptr: {}, size: {}", name, ptr, size);

            Base::dealloc_range(ptr, size);
        }

    }; // class type

}; // struct log_range

} // namespace cielmalloc

#endif // CIELMALLOC_LOG_RANGE_HPP_
