#include <ciel/message_queue.h>
#include <ciel/remote_request.h>
#include <ciel/span.h>
#include <ciel/thread_alloc.h>

NAMESPACE_CIEL_BEGIN

remote_request_list::~remote_request_list() {
    CIEL_PRECONDITION(num_ == 0);
}

void remote_request_list::push_before_head(void* ptr) noexcept {
    const size_t hn = hops_num(ptr);
    CIEL_PRECONDITION(hn < 16);

    ptr_next(ptr) = head_;
    head_ = ptr;

    if (num_++ == 0) {
        tail_ = head_;
    }

    if (num_ > RequestsSendingThreshold) {
        // requests num beyond threshold, sending...
        send_to_message_queue(get_span(ptr)->belong_to_->message_queue_);
    }
}

void remote_request_list::send_to_message_queue(message_queue& mq) noexcept {
    CIEL_PRECONDITION(head_ != nullptr);
    CIEL_PRECONDITION(tail_ != nullptr);

    mq.enqueue(head_, tail_);

    head_ = nullptr;
    tail_ = nullptr;
    num_ = 0;
}

CIEL_NODISCARD size_t& hops_num(void* ptr) noexcept {
    return *(static_cast<size_t*>(ptr) + 1);
}

NAMESPACE_CIEL_END