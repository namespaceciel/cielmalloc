#include <gtest/gtest.h>

#include <ciel/core/config.hpp>
#include <ciel/inplace_vector.hpp>
#include <cielmalloc/config.hpp>
#include <cielmalloc/medium_slab.hpp>
#include <cielmalloc/pal.hpp>
#include <cielmalloc/remote_allocator.hpp>
#include <cielmalloc/sizeclass.hpp>

#include <cstdint>

using namespace cielmalloc;

TEST(medium_slab, all) {
    auto chunk = static_cast<medium_slab*>(pal::reserve(LargeThreshold));
    pal::commit(chunk, LargeThreshold);

    for (uint8_t i = NumMediumClassesRange.first; i < NumMediumClassesRange.second; ++i) {
        const auto slots = medium_slab_slots(i);

        ciel::inplace_vector<void*, 255> iv;
        chunk->init(reinterpret_cast<remote_allocator*>(static_cast<uintptr_t>(1)), i);
        while (!chunk->empty()) {
            iv.unchecked_emplace_back(chunk->allocate());
        }
        ASSERT_EQ(iv.size(), slots);

        for (auto p : iv) {
            CIEL_UNUSED(chunk->deallocate(p));
        }
        ASSERT_TRUE(chunk->full());
    }

    pal::decommit(chunk, LargeThreshold);
}
