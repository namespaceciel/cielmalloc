#include <ciel/headquarter_alloc.h>
#include <ciel/thread_alloc.h>

NAMESPACE_CIEL_BEGIN

// span_list
void span_list::push_before_head(span* ptr) noexcept {
    CIEL_PRECONDITION(ptr != nullptr);

    ptr->prev_ = nullptr;

    if (head_ == nullptr) {
        CIEL_PRECONDITION(tail_ == nullptr);

        head_ = ptr;
        tail_ = ptr;
        head_->next_ = nullptr;

    } else {
        CIEL_PRECONDITION(tail_ != nullptr);

        ptr->next_ = head_;
        head_->prev_ = ptr;
        head_ = ptr;
    }
}

void span_list::push_after_tail(span* ptr) noexcept {
    CIEL_PRECONDITION(ptr != nullptr);

    ptr->next_ = nullptr;

    if (tail_ == nullptr) {
        CIEL_PRECONDITION(head_ == nullptr);

        head_ = ptr;
        tail_ = ptr;
        tail_->prev_ = nullptr;

    } else {
        CIEL_PRECONDITION(head_ != nullptr);

        ptr->prev_ = tail_;
        tail_->next_ = ptr;
        tail_ = ptr;
    }
}

CIEL_NODISCARD span* span_list::eject(span* ptr) noexcept {
    CIEL_PRECONDITION(ptr != nullptr);
    CIEL_PRECONDITION(head_ != nullptr);
    CIEL_PRECONDITION(tail_ != nullptr);

    // Only one node.
    if (ptr == head_ && ptr == tail_) {
        head_ = nullptr;
        tail_ = nullptr;
        return ptr;
    }

    // ptr is head_
    if (ptr == head_) {
        head_ = ptr->next_;
        head_->prev_ = nullptr;
        return ptr;
    }

    // ptr is tail_
    if (ptr == tail_) {
        tail_ = ptr->prev_;
        tail_->next_ = nullptr;
        return ptr;
    }

    // ptr at mid
    ptr->prev_->next_ = ptr->next_;
    ptr->next_->prev_ = ptr->prev_;
    return ptr;
}

void span_list::eject_head_after_tail() noexcept {
    CIEL_PRECONDITION(head_ != nullptr);
    CIEL_PRECONDITION(tail_ != nullptr);

    if (head_ == tail_) {
        return;
    }

    tail_->next_ = head_;
    head_->prev_ = tail_;

    tail_ = head_;

    head_->next_->prev_ = nullptr;
    head_ = head_->next_;

    tail_->next_ = nullptr;
}

CIEL_NODISCARD bool span_list::empty() const noexcept {
    return head_ == nullptr;
}

// thread_allocator
CIEL_NODISCARD void* thread_allocator::allocate(const size_t size) noexcept {
    process_message_queue();

    const size_t sizeclass = size_to_sizeclass(size);

    span_list& freelist_at_sizeclass = freelist_[sizeclass];

    // Is there any valid block?
    if (freelist_at_sizeclass.empty() || freelist_at_sizeclass.head_->empty()) {
        span* new_span = headquarter_allocator::get_instance().allocate();
        new_span->init(sizeclass_to_size(sizeclass), bound_core_);
        freelist_at_sizeclass.push_before_head(new_span);
    }

    CIEL_POSTCONDITION(!freelist_at_sizeclass.empty() && !freelist_at_sizeclass.head_->empty());

    void* res = freelist_at_sizeclass.head_->allocate();

    CIEL_POSTCONDITION(res != nullptr);

    if (freelist_at_sizeclass.head_->empty()) {
        freelist_at_sizeclass.eject_head_after_tail();
    }

    return res;
}

void thread_allocator::deallocate(void* ptr) noexcept {
    process_message_queue();

    span* s = get_span(ptr);

    CIEL_PRECONDITION(s->magic_number_ == MagicNumber);

    const size_t sizeclass = size_to_sizeclass(s->obj_size_);
    span_list& freelist_at_sizeclass = freelist_[sizeclass];

    if (s->belong_to_ == bound_core_) {
        switch (s->deallocate(ptr)) {
            case span::DeallocatedResult::BackFromZero : {
                freelist_at_sizeclass.push_before_head(freelist_at_sizeclass.eject(s));
                break;
            }
            case span::DeallocatedResult::BackToOnePiece : {
                CIEL_UNUSED(freelist_at_sizeclass.eject(s));
                headquarter_allocator::get_instance().deallocate(s);
                break;
            }
            case span::DeallocatedResult::None :
                break;
        }

        return;
    }

    // Belongs to another allocator
    const size_t remote_requests_index = (uintptr_t)s->belong_to_ % radix_buckets;

    CIEL_PRECONDITION(remote_requests_index < radix_buckets);

    hops_num(ptr) = 0;
    remote_requests_[remote_requests_index].push_before_head(ptr);
}

CIEL_NODISCARD thread_allocator& thread_allocator::get_instance() noexcept {
    static thread_local thread_allocator alloc;
    return alloc;
}

thread_allocator::~thread_allocator() {
    for (auto& rr : remote_requests_) {
        if (rr.head_) {
            rr.send_to_message_queue(get_span(rr.head_)->belong_to_->message_queue_);
        }
    }

    headquarter_allocator::get_instance().release_core(bound_core_);

    if (--threads_num_ == 0) {
        headquarter_allocator::get_instance().~headquarter_allocator();
    }
}

void thread_allocator::process_message_queue() noexcept {
    void* request;

    while ((request = bound_core_->message_queue_.dequeue()) != nullptr) {
        // We can just call deallocate(request), but if request need more jumps,
        // we can separate them using higher 6 bits hash.

        span* s = get_span(request);

        CIEL_PRECONDITION(s->magic_number_ == MagicNumber);

        const size_t sizeclass = size_to_sizeclass(s->obj_size_);
        span_list& freelist_at_sizeclass = freelist_[sizeclass];

        if (s->belong_to_ == bound_core_) {
            switch (s->deallocate(request)) {
                case span::DeallocatedResult::BackFromZero : {
                    freelist_at_sizeclass.push_before_head(freelist_at_sizeclass.eject(s));
                    break;
                }
                case span::DeallocatedResult::BackToOnePiece : {
                    CIEL_UNUSED(freelist_at_sizeclass.eject(s));
                    headquarter_allocator::get_instance().deallocate(s);
                    break;
                }
                case span::DeallocatedResult::None :
                    break;
            }

            return;
        }

        // Belongs to another allocator
        size_t& hn = hops_num(request);
        const size_t remote_requests_index = (uintptr_t)s->belong_to_ % ((radix_buckets & (0b111111 << (radix_bits << hn))) >> (radix_bits << hn));

        ++hn;

        CIEL_PRECONDITION(remote_requests_index < radix_buckets);

        remote_requests_[remote_requests_index].push_before_head(request);
    }
}

std::atomic<size_t> thread_allocator::threads_num_{0};

NAMESPACE_CIEL_END