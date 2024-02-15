#include <gtest/gtest.h>

#include <cielmalloc/config.hpp>
#include <cielmalloc/pagemap.hpp>
#include <cielmalloc/slab.hpp>

#include <cstdint>

using namespace cielmalloc;

TEST(pagemap, all) {
    ASSERT_EQ(global_pagemap.load(nullptr), slab_kind::Wild);

    const uintptr_t small  = 0xf1f2f3f4;
    const uintptr_t medium = small + LargeThreshold;
    const uintptr_t large  = medium + LargeThreshold;

    void* p1 = reinterpret_cast<void*>(small);
    void* p2 = reinterpret_cast<void*>(medium);
    void* p3 = reinterpret_cast<void*>(large);

    global_pagemap.store(p1, slab_kind::Small);
    global_pagemap.store(p2, slab_kind::Medium);
    global_pagemap.store(p3, slab_kind::Large);

    ASSERT_EQ(global_pagemap.load(p1), slab_kind::Small);
    ASSERT_EQ(global_pagemap.load(p2), slab_kind::Medium);
    ASSERT_EQ(global_pagemap.load(p3), slab_kind::Large);
}
