# Nova Template Library

The Nova Template Library (NTL) is Nova's version of the C++ Standard Template Library (STL). While they _do_ have much of the same functionality, the NTL implements it without relying so much on iterators. Instead, the NTL lets you convert any container into a _Stream_, which then supports the regular functional operations like `map`, `reduce`, etc

## Motivations

### Speed

Nova needs to run as fast as possible, which means it needs to take full advantage of the CPU cache. The STL doens't do that - classes like `std::unordered_map` thrach the CPU data cache

### Missing functionality

The STL is missing a number of things that Nova needs or that I'd like to use, such as:

- Custom allocator support (limited in C++11 and C++17, finally usable in C++20, but I don't like their interface anyways)
- Fixed-size array where the size is known at runtime
- Functional programming with streams

Additionally, I despise its iterator-heavy design
