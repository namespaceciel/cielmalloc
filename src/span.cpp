#include <ciel/headquarter_alloc.h>
#include <ciel/sizeclass.h>
#include <ciel/span.h>

NAMESPACE_CIEL_BEGIN

void span::init(const size_t obj_size, thread_allocator_core* tac) noexcept {
    CIEL_PRECONDITION(is_aligned(this, 1 << PAGE_SHIFT));
    CIEL_PRECONDITION(obj_size % size_to_alignment(obj_size) == 0);

    magic_number_ = MagicNumber;

    belong_to_ = tac;

    allocated_num_ = 0;
    obj_size_ = obj_size;

    free_ = (void*)align_up((uintptr_t)this + sizeof(span), size_to_alignment(obj_size_));

    // Set next to null so that we know it's the last node.
    ptr_next(free_) = nullptr;
}

CIEL_NODISCARD void* span::allocate() noexcept {
    if (ptr_next(free_) == nullptr) {    // Is this the high-water mark?
        // Is free_ crossed over this span (run out of capacity)?
        if (align_down((uintptr_t)free_ + obj_size_, 1 << PAGE_SHIFT) != (uintptr_t)this) {
            return nullptr;
        }

        void* res = free_;
        // TODO: alignment?
        free_ = (void*)((uintptr_t)free_ + obj_size_);

        CIEL_POSTCONDITION(is_aligned(free_, size_to_alignment(obj_size_)));

        // Set next to null so that we know it's the last node.
        ptr_next(free_) = nullptr;

        ++allocated_num_;
        return res;
    }

    // Otherwise free_ has some recycled nodes.
    void* res = free_;
    free_ = ptr_next(free_);

    ++allocated_num_;
    return res;
}

CIEL_NODISCARD bool span::empty() const noexcept {
    return ptr_next(free_) == nullptr &&
        align_down((uintptr_t)free_ + obj_size_, 1 << PAGE_SHIFT) != (uintptr_t)this;
}

CIEL_NODISCARD span::DeallocatedResult span::deallocate(void* ptr) noexcept {
    CIEL_PRECONDITION(ptr != nullptr);
    // Is ptr belongs to this span?
    CIEL_PRECONDITION(align_down((uintptr_t)ptr, 1 << PAGE_SHIFT) == (uintptr_t)this);

    // If this was run out of capacity, and has one new freed node now,
    // we insert it back to head.
    const bool back_from_zero = empty();

    // insert before head
    ptr_next(ptr) = free_;
    free_ = ptr;

    if (--allocated_num_ == 0) {
        return DeallocatedResult::BackToOnePiece;
    }

    return back_from_zero ? DeallocatedResult::BackFromZero : DeallocatedResult::None;
}

void span::deallocate_after_thread_died() noexcept {
    if (--allocated_num_ == 0) {
        headquarter_allocator::get_instance().deallocate(this);
    }
}

CIEL_NODISCARD span* get_span(void* ptr) noexcept {
    auto res = (span*)align_down((uintptr_t)ptr, 1 << PAGE_SHIFT);
    return res;
}

NAMESPACE_CIEL_END