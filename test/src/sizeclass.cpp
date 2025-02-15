#include <gtest/gtest.h>

#include <ciel/core/message.hpp>
#include <cielmalloc/sizeclass.hpp>

#include <cstddef>

using namespace cielmalloc;

TEST(sizeclass, print) {
    for (size_t i = 0; i < sizeclass_metadata.small_slab_slots.size(); ++i) {
        CIELMALLOC_LOG("small_sizeclass: {}, size: {}, alignment: {}, slots: {}", i, sizeclass_metadata.size[i],
                       sizeclass_metadata.alignment[i], sizeclass_metadata.small_slab_slots[i]);
    }

    for (size_t i = 0; i < sizeclass_metadata.medium_slab_slots.size(); ++i) {
        CIELMALLOC_LOG("medium_sizeclass: {}, size: {}, alignment: {}, slots: {}", i + NumSmallClasses,
                       sizeclass_metadata.size[i + NumSmallClasses], sizeclass_metadata.alignment[i],
                       sizeclass_metadata.medium_slab_slots[i]);
    }
}
