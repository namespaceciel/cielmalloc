#ifndef CIELMALLOC_BUDDY_HPP_
#define CIELMALLOC_BUDDY_HPP_

#include <ciel/core/alignment.hpp>
#include <ciel/core/array.hpp>
#include <ciel/core/config.hpp>
#include <ciel/core/rb_tree.hpp>
#include <cielmalloc/bits.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/pagemap.hpp>

#include <cstddef>
#include <cstdint>
#include <type_traits>
#include <utility>

namespace cielmalloc {

struct RBNodeInplace : ciel::rb_node_base {
    using value_type = uintptr_t;

    CIEL_NODISCARD value_type value() const noexcept {
        return reinterpret_cast<value_type>(this);
    }

}; // struct RBNodeInplace

template<class NodeType, size_t BitBegin, size_t BitEnd>
class buddy {
    using node_type  = NodeType;
    using value_type = typename node_type::value_type;

    static_assert(sizeof(node_type) <= cielmalloc::one_at_bit(BitBegin),
                  "Block size should be no less than sizeof(rb_node)");

private:
    struct NodeCompare {
        template<class L, class R>
        CIEL_NODISCARD bool operator()(L&& lhs, R&& rhs) const noexcept {
            return lhs < rhs;
        }

    }; // struct NodeCompare

    ciel::array<ciel::rb_tree<node_type, NodeCompare>, BitBegin, BitEnd> trees_;

    CIEL_NODISCARD static size_t to_index(const size_t size) noexcept {
        CIEL_ASSERT(ciel::is_pow2(size));

        return cielmalloc::next_pow2_bits(size);
    }

    CIEL_NODISCARD static node_type* get_node(void* ptr) noexcept {
        if constexpr (std::is_same_v<node_type, RBNodeInplace>) {
            return static_cast<node_type*>(ptr);

        } else {
            return &(Pagemap::get().template get<true>(ptr));
        }
    }

    CIEL_NODISCARD static node_type* get_node(uintptr_t ptr) noexcept {
        return get_node(reinterpret_cast<void*>(ptr));
    }

    CIEL_NODISCARD static void* get_ptr(node_type* node) noexcept {
        if constexpr (std::is_same_v<node_type, RBNodeInplace>) {
            return static_cast<void*>(node);

        } else {
            return (node != nullptr) ? reinterpret_cast<void*>(node->value()) : nullptr;
        }
    }

    CIEL_NODISCARD static void* get_ptr(uintptr_t node) noexcept {
        return get_ptr(reinterpret_cast<node_type*>(node));
    }

    // Split a block of size {1 << from} into size {1 << to} and return,
    // store the rest blocks.
    CIEL_NODISCARD node_type* divide_block(node_type* const block, size_t from, const size_t to) noexcept {
        CIEL_ASSERT(block != nullptr);

        for (--from; from >= to; --from) {
            node_type* other_half = get_node(block->value() + cielmalloc::one_at_bit(from));

            const bool b = trees_[from].insert(other_half);

            CIEL_ASSERT(b);
            CIEL_UNUSED(b);
        }

        return block;
    }

public:
    // RefillCallback shall allocate a block of size {1 << BitEnd} from upper range.
    // Return nullptr is this request cannot be satisfied (refill may fail).
    template<class RefillCallback>
    CIEL_NODISCARD void* allocate_block(const size_t size, RefillCallback&& refill) noexcept {
        node_type* node = [&]() noexcept -> node_type* {
            const auto index = to_index(size);
            // Is this tree has blocks, then return one.
            if (!trees_[index].empty()) {
                return trees_[index].extract_min();
            }

            // Or try to divide the bigger block.
            for (auto i = index + 1; i < BitEnd; ++i) {
                if (!trees_[i].empty()) {
                    auto node = trees_[i].extract_min();
                    return divide_block(node, i, index);
                }
            }

            const auto refill_block = refill(); // std::pair<void*, size_t>: memory and size_bits
            if CIEL_UNLIKELY (refill_block.first == nullptr) {
                return nullptr;
            }

            return divide_block(get_node(refill_block.first), cielmalloc::next_pow2_bits(refill_block.second), index);
        }();

        return get_ptr(node);
    }

    // Return nullptr if p is successfully deallocated,
    // or a block of size {1 << BitEnd}.
    void* deallocate_block(void* ptr, const size_t size) noexcept {
        node_type* node  = get_node(ptr);
        const auto index = to_index(size);

        for (auto i = index; i < BitEnd; ++i) {
            const auto bit_size   = cielmalloc::one_at_bit(i);
            node_type* other_half = trees_[i].find(node->value() ^ bit_size);

            // If this block cannot be consolidated, then insert it to the tree.
            if (other_half == nullptr) {
                const bool b = trees_[i].insert(node);

                CIEL_ASSERT(b);
                CIEL_UNUSED(b);

                return nullptr;
            }

            // Or consolidate the block, try next tree.
            trees_[i].remove(other_half);
            // New block's address will always be the smaller one.
            if (node->value() > other_half->value()) {
                node = other_half;
            }
        }

        // Block too big for this buddy.
        return reinterpret_cast<void*>(node->value());
    }

}; // class buddy

} // namespace cielmalloc

#endif // CIELMALLOC_BUDDY_HPP_
