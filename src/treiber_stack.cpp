#include <ciel/system.h>
#include <ciel/treiber_stack.h>

NAMESPACE_CIEL_BEGIN

void treiber_stack::push(void* new_head) noexcept {
    void* old_head;

    do {
        old_head = top_;
        ptr_next(new_head) = old_head;
    } while (!top_.compare_exchange_weak(old_head, new_head));
}

void* treiber_stack::pop() noexcept {
    void* old_head;
    void* new_head;

    do {
        old_head = top_;

        if (old_head == nullptr) {
            return nullptr;
        }
        new_head = ptr_next(old_head);
    } while (!top_.compare_exchange_weak(old_head, new_head));

    return old_head;
}

NAMESPACE_CIEL_END