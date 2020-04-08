#include "visibility_cache.hpp"

namespace nova::renderer {
    VisibilityCache::VisibilityCache(rx::memory::allocator& allocator) : cached_cameras{allocator}, visibility_cache{allocator} {}
} // namespace nova::renderer
