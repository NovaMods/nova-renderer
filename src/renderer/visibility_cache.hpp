#pragma once

#include <rx/core/map.h>

#include "nova_renderer/camera.hpp"
#include "nova_renderer/renderables.hpp"

namespace nova::renderer {
    /*!
     * \brief Cache of cache of which objects are visible to which cameras
     *
     * Implements frustum culling. Will eventually use hardware occlusion queries
     *
     * This class caches visibility per camera. When camera parameters change for a camera at a given index, the cache for that camera is
     * invalidated. Future occlusion queries will have to re-calculate themselves
     */
    class VisibilityCache {
    public:
        explicit VisibilityCache(rx::memory::allocator& allocator);

        VisibilityCache(const VisibilityCache& other) = delete;
        VisibilityCache& operator=(const VisibilityCache& other) = delete;

        VisibilityCache(VisibilityCache&& old) noexcept = default;
        VisibilityCache& operator=(VisibilityCache&& old) noexcept = default;

        ~VisibilityCache() = default;

        /*!
         * \brief Sets the visibility of a renderable in the visibility cache
         *
         * \param camera The camera to set visibility for
         * \param renderable The renderable to set visibility for
         * \param visibility Whether or not the renderable is visible
         */
        void set_renderable_visibility(const Camera& camera, RenderableId renderable, bool visibility);

        /*!
         * \brief Checks if a given renderable is visible to a given camera
         *
         * This method first checks if the visibility cache for this camera is up-to-date. If so, it fetches the visibility result for the
         * renderable and returns is directly. However, if the visibility cache is _not_ up-to-date, this method invalidates the cache and
         * recalculates visibility for this renderable
         *
         * If the cache is up-to-date but doesn't have this renderable in it, this method will calculate visibility and save it to the
         * cache, then return the result
         */
        [[nodiscard]] bool is_renderable_visible_to_camera(RenderableId renderable, const Camera& camera);

    private:
        /*!
         * \brief A map of the camera parameters that were most recently seen for a given camera
         *
         * If we see see a camera that has different parameters than what's in this cache, the visibility results for that camera are
         * invalidated
         */
        rx::map<CameraIndex, Camera> cached_cameras;

        /*!
         * \brief Cache of which renderables are visible for a given camera
         */
        rx::map<CameraIndex, rx::map<RenderableId, bool>> visibility_cache;
    };
} // namespace nova::renderer
