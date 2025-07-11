#include <gtest/gtest.h>

#include <ciel/core/message.hpp>
#include <cielmalloc/sizeclass.hpp>

#include <cstddef>

using namespace cielmalloc;

TEST(sizeclass, print) {
    for (size_t i = sizeclass_metadata.small_slab_slots.size_begin();
         i < sizeclass_metadata.small_slab_slots.size_end(); ++i) {
        CIELMALLOC_LOG("small_sizeclass: {}, size: {}, alignment: {}, slots: {}", i, sizeclass_metadata.size[i],
                       sizeclass_metadata.alignment[i], sizeclass_metadata.small_slab_slots[i]);
    }

    for (size_t i = sizeclass_metadata.medium_slab_slots.size_begin();
         i < sizeclass_metadata.medium_slab_slots.size_end(); ++i) {
        CIELMALLOC_LOG("medium_sizeclass: {}, size: {}, alignment: {}, slots: {}", i, sizeclass_metadata.size[i],
                       sizeclass_metadata.alignment[i], sizeclass_metadata.medium_slab_slots[i]);
    }
}
