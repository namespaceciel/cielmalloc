#ifndef CIELMALLOC_INCLUDE_CIEL_REMOTE_REQUEST_H_
#define CIELMALLOC_INCLUDE_CIEL_REMOTE_REQUEST_H_

#include <ciel/config.h>

NAMESPACE_CIEL_BEGIN

class message_queue;

// Since our pointers allocated are always of size at least 16B,
// we used next 8B to indicate the hops num.
//
class remote_request_list {
public:
    remote_request_list() noexcept = default;

    remote_request_list(const remote_request_list&) = delete;
    remote_request_list& operator=(const remote_request_list&) = delete;

    ~remote_request_list();

    // Make sure next 8B is clean before passed in.
    void push_before_head(void*) noexcept;

    void send_to_message_queue(message_queue&) noexcept;

private:
    static constexpr size_t RequestsSendingThreshold = 32;

    void* head_{nullptr};
    void* tail_{nullptr};
    size_t num_{0};

};  // class remote_request_list

CIEL_NODISCARD size_t& hops_num(void*) noexcept;

NAMESPACE_CIEL_END

#endif // CIELMALLOC_INCLUDE_CIEL_REMOTE_REQUEST_H_