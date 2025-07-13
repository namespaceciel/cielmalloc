#ifndef CIELMALLOC_BUDDY_HPP_
#define CIELMALLOC_BUDDY_HPP_

#include <ciel/core/alignment.hpp>
#include <ciel/core/array.hpp>
#include <ciel/core/config.hpp>
#include <ciel/core/rb_tree.hpp>
#include <cielmalloc/bits.hpp>
#include <cielmalloc/config.hpp>

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace cielmalloc {

template<size_t BitBegin, size_t BitEnd>
class buddy {
    using Node = ciel::rb_node<void>;
    static_assert(sizeof(Node) <= cielmalloc::one_at_bit(BitBegin),
                  "Block size should be no less than sizeof(rb_node)");

private:
    struct NodeCompare {
        CIEL_NODISCARD bool operator()(uintptr_t lhs, uintptr_t rhs) const noexcept {
            return lhs < rhs;
        }

    }; // struct NodeCompare

    ciel::array<ciel::rb_tree<void, NodeCompare>, BitBegin, BitEnd> trees_;

    CIEL_NODISCARD static size_t to_index(const size_t size) noexcept {
        CIEL_ASSERT(ciel::is_pow2(size));

        return cielmalloc::next_pow2_bits(size);
    }

    CIEL_NODISCARD void* divide_block(Node* const block, size_t from, const size_t to) noexcept {
        for (--from; from >= to; --from) {
            auto p       = cielmalloc::offset(block, cielmalloc::one_at_bit(from));
            const bool b = trees_[from].insert(p);

            CIEL_ASSERT(b);
            CIEL_UNUSED(b);
        }

        return block;
    }

    // Allocate a block of size 1 << BitEnd from PAL.
    CIEL_NODISCARD Node* refill() noexcept {
        CIEL_ASSERT_M(false, "Not implemented");

        return nullptr;
    }

    // Deallocate a block of size 1 << BitEnd to PAL.
    void vacate(Node*) noexcept {
        CIEL_ASSERT_M(false, "Not implemented");
    }

public:
    CIEL_NODISCARD void* allocate_block(const size_t size) noexcept {
        const auto index = to_index(size);
        if (!trees_[index].empty()) {
            return trees_[index].extract_min();
        }

        for (auto i = index + 1; i < BitEnd; ++i) {
            if (!trees_[i].empty()) {
                auto node = trees_[i].extract_min();
                return divide_block(node, i, index);
            }
        }

        return divide_block(refill(), BitEnd, index);
    }

    void deallocate_block(void* p, const size_t size) noexcept {
        Node* node       = static_cast<Node*>(p);
        const auto index = to_index(size);

        for (auto i = index; i < BitEnd; ++i) {
            const auto bit_size = cielmalloc::one_at_bit(i);
            Node* other_half    = trees_[i].find(node->value() ^ bit_size);

            if (other_half == nullptr) {
                const bool b = trees_[i].insert(node);

                CIEL_ASSERT(b);
                CIEL_UNUSED(b);

                return;
            }

            node = cielmalloc::transform(node, [bit_size](uintptr_t p) {
                return p & (~bit_size);
            });
        }

        vacate(node);
    }

}; // class buddy

} // namespace cielmalloc

#endif // CIELMALLOC_BUDDY_HPP_
