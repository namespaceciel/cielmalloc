#ifndef CIELMALLOC_LIST_HPP_
#define CIELMALLOC_LIST_HPP_

#include <ciel/core/config.hpp>
#include <ciel/core/message.hpp>

namespace cielmalloc {

struct list_node_base {
    list_node_base* next{nullptr};

}; // struct list_node_base

template<class T>
class list {
    static_assert(std::is_convertible_v<T, list_node_base>);

private:
    list_node_base dummy_;

public:
    void push_front(T* t) noexcept {
        CIEL_ASSERT(t != nullptr);

        t->next     = dummy_.next;
        dummy_.next = t;
    }

    T* pop_front() noexcept {
        CIEL_ASSERT(!empty());

        T* res      = front();
        dummy_.next = res->next;

        return res;
    }

    CIEL_NODISCARD T* front() noexcept {
        CIEL_ASSERT(!empty());

        return static_cast<T*>(dummy_.next);
    }

    CIEL_NODISCARD bool empty() const noexcept {
        return dummy_.next == nullptr;
    }

}; // class list

} // namespace cielmalloc

#endif // CIELMALLOC_LIST_HPP_
