#ifndef CIELMALLOC_INCLUDE_CIEL_TREIBER_STACK_H_
#define CIELMALLOC_INCLUDE_CIEL_TREIBER_STACK_H_

#include <ciel/config.h>

#include <atomic>

NAMESPACE_CIEL_BEGIN

// A simple lock-free stack, used in headquarter_allocator.
//
class treiber_stack {
public:
    void push(void*) noexcept;

    void* pop() noexcept;

private:
    std::atomic<void*> top_{nullptr};

};  // class treiber_stack

NAMESPACE_CIEL_END

#endif // CIELMALLOC_INCLUDE_CIEL_TREIBER_STACK_H_