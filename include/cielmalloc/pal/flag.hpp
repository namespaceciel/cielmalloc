#ifndef CIELMALLOC_PAL_FLAG_HPP_
#define CIELMALLOC_PAL_FLAG_HPP_

namespace cielmalloc {

enum PalFeatures : uint64_t {
    AlignedAllocation = (1 << 0),
    LazyCommit        = (1 << 1),
    Entropy           = (1 << 2),

}; // enum PalFeatures

enum ZeroMem {
    NoZero,
    YesZero,

}; // enum ZeroMem

} // namespace cielmalloc

#endif // CIELMALLOC_PAL_FLAG_HPP_
