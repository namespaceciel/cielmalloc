#ifndef CIELMALLOC_INCLUDE_CIEL_CONFIG_H_
#define CIELMALLOC_INCLUDE_CIEL_CONFIG_H_

#if !defined(__cplusplus) || (__cplusplus < 201103L)
#error "Please use C++ with standard of at least 11"
#endif

#include <cassert>

// standard_version
#if __cplusplus <= 201103L
#define CIEL_STD_VER 11
#elif __cplusplus <= 201402L
#define CIEL_STD_VER 14
#elif __cplusplus <= 201703L
#define CIEL_STD_VER 17
#elif __cplusplus <= 202002L
#define CIEL_STD_VER 20
#elif __cplusplus <= 202302L
#define CIEL_STD_VER 23
#else
#define CIEL_STD_VER 26
#endif

// namespace ciel
#define NAMESPACE_CIEL_BEGIN namespace ciel {

#define NAMESPACE_CIEL_END   }  // namespace ciel

// assert
#define CIEL_PRECONDITION(cond)  assert(cond)
#define CIEL_POSTCONDITION(cond) assert(cond)

// likely, unlikely
#if CIEL_STD_VER >= 20 || (defined(_MSVC_LANG) && _MSVC_LANG >= 202002L)
#define CIEL_LIKELY(x)   (x) [[likely]]
#define CIEL_UNLIKELY(x) (x) [[unlikely]]

#elif defined(__GNUC__) || defined(__clang__)
#define CIEL_LIKELY(x)   (__builtin_expect(!!(x), true))
#define CIEL_UNLIKELY(x) (__builtin_expect(!!(x), false))

#else
#define CIEL_LIKELY(x)   (x)
#define CIEL_UNLIKELY(x) (x)
#endif

// __has_builtin
#ifndef __has_builtin
#define __has_builtin(x) false
#endif

// nodiscard
#if CIEL_STD_VER >= 17
#define CIEL_NODISCARD [[nodiscard]]

#elif (defined(__GNUC__) && (__GNUC__ >= 4)) || defined(__clang__)  // clang, icc, clang-cl
#define CIEL_NODISCARD __attribute__((warn_unused_result))

#elif defined(_HAS_NODISCARD)
#define CIEL_NODISCARD _NODISCARD

#elif _MSC_VER >= 1700
#define CIEL_NODISCARD _Check_return_

#else
#define CIEL_NODISCARD
#endif

// unused
#define CIEL_UNUSED(x) static_cast<void>(x)

NAMESPACE_CIEL_BEGIN

[[noreturn]] inline void unreachable() noexcept {
#if defined(_MSC_VER) && !defined(__clang__)    // MSVC
    __assume(false);

#else    // GCC, Clang
    __builtin_unreachable();
#endif
}

NAMESPACE_CIEL_END

#endif // CIELMALLOC_INCLUDE_CIEL_CONFIG_H_