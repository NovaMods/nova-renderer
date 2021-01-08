#include "nova_renderer/camera.hpp"

namespace nova::renderer {
    const std::string& Camera::get_name() const { return name; }

    Camera::Camera(const CameraCreateInfo& create_info)
        : is_active(true),
          aspect_ratio{create_info.aspect_ratio},
          field_of_view{create_info.field_of_view},
          near_plane{create_info.near_plane},
          far_plane{create_info.far_plane},
          name{create_info.name} {}
} // namespace nova::renderer
