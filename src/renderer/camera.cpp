#include "nova_renderer/camera.hpp"

namespace nova::renderer {
    Camera::Camera(Camera&& old) noexcept
        : field_of_view(old.field_of_view),
          near_plane(old.near_plane),
          far_plane(old.far_plane),
          name(rx::utility::move(old.name)),
          matrices_buffer(rx::utility::move(old.matrices_buffer)) {}

    Camera& Camera::operator=(Camera&& old) noexcept {
        field_of_view = old.field_of_view;
        near_plane = old.near_plane;
        far_plane = old.far_plane;
        name = rx::utility::move(old.name);
        matrices_buffer = rx::utility::move(old.matrices_buffer);

        return *this;
    }

    const rx::string& Camera::get_name() const { return name; }

    Camera::Camera(const CameraCreateInfo& create_info, const BufferResourceAccessor& buffer_accessor)
        : field_of_view(create_info.field_of_view),
          near_plane(create_info.near_plane),
          far_plane(create_info.far_plane),
          name(create_info.name),
          matrices_buffer(buffer_accessor) {}
} // namespace nova::renderer
