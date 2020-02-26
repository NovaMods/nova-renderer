#pragma once
#include <rx/core/string.h>

#include "resource_loader.hpp"

namespace nova::renderer {
    class NovaRenderer;

    /*!
     * \brief Create info to initialize a camera
     */
    struct CameraCreateInfo {
        /*!
         * \brief Name of the new camera
         */
        rx::string name;

        /*!
         * \brief Vertical field of view
         */
        float field_of_view = 90;

        /*!
         * \brief Near plane of the camera. Corresponds to a value of 1 in the depth buffer
         */
        float near_plane = 0.001;

        /*!
         * \brief Far plane of the camera. Corresponds to a value of 0 in the depth buffer
         */
        float far_plane = 1000.0;
    };

    /*!
     * \brief Data for a camera's UBO
     */
    struct CameraUboData {
        /*!
         * \brief Current frame's view matrix
         */
        glm::mat4 view;

        /*!
         * \brief current frame's projection matrix
         */
        glm::mat4 projection;

        /*!
         * \brief Previous frame's view matrix
         */
        glm::mat4 previous_view;

        /*!
         * \brief Previous frame's projection matrix
         */
        glm::mat4 previous_projection;
    };

    class Camera {
        friend class NovaRenderer;

    public:
        /*!
         * \brief Vertical field of view
         */
        float field_of_view;

        /*!
         * \brief Near clipping plane
         *
         * Objects at this worldspace distance from the camera will have a value of 1 in the depth buffer
         */
        float near_plane;

        /*!
         * \brief Far clipping plane
         *
         * Objects at this worldspace distance from the camera will have a value of 0 in the depth buffer
         */
        float far_plane;

        /*!
         * \brief Index of this camera in the camera array
         */
        uint32_t index;

        Camera(const Camera& other) = delete;
        Camera& operator=(const Camera& other) = delete;

        Camera(Camera&& old) noexcept = default;
        Camera& operator=(Camera&& old) noexcept = default;

        ~Camera() = default;

        [[nodiscard]] const rx::string& get_name() const;

    private:
        rx::string name;

        explicit Camera(const CameraCreateInfo& create_info, BufferResourceAccessor buffer_accessor);

        BufferResourceAccessor matrices_buffer;
    };
} // namespace nova::renderer
