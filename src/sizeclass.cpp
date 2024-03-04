#include <ciel/sizeclass.h>

NAMESPACE_CIEL_BEGIN

size_t size_to_sizeclass(size_t size) noexcept {
    CIEL_PRECONDITION(size <= (63 << 10));

    if (size <= 1024) {
        size = align_up(size, 16);
        return size / 16;

    } else if (size <= 8096) {
        size = align_up(size, 128);
        return 64 + (size - 1024) / 128;

    } else {
        size = align_up(size, 1024);
        return 120 + (size - (8 << 10)) / 1024;
    }
}

size_t sizeclass_to_size(const size_t sizeclass) noexcept {
    CIEL_PRECONDITION(sizeclass <= 175);

    if (sizeclass <= 64) {
        return sizeclass * 16;

    } else if (sizeclass <= 120) {
        return 1024 + (sizeclass - 64) * 128;

    } else {
        return (8 << 10) + (sizeclass - 120) * 1024;
    }
}

size_t size_to_alignment(const size_t size) noexcept {
    CIEL_PRECONDITION(size <= (63 << 10));

    if (size <= 1024) {
        return 16;

    } else if (size <= 8096) {
        return 128;

    } else {
        return 1024;
    }
}

NAMESPACE_CIEL_END