#pragma once
#include <rx/core/string.h>

#include "resource_loader.hpp"

namespace nova::renderer {
    class NovaRenderer;

    using CameraIndex = uint32_t;

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
        float field_of_view = 90.0f;

        /*!
         * \brief Aspect ratio of the camera
         */
        float aspect_ratio = 16.0f / 9.0f;

        /*!
         * \brief Near plane of the camera. Corresponds to a value of 1 in the depth buffer
         */
        float near_plane = 0.001f;

        /*!
         * \brief Far plane of the camera. Corresponds to a value of 0 in the depth buffer
         */
        float far_plane = 1000.0f;
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
        bool is_active = false;

        float aspect_ratio;

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

        glm::vec3 position{};

        glm::vec3 rotation{};

        /*!
         * \brief Index of this camera in the camera array
         */
        CameraIndex index{};

        explicit Camera(const CameraCreateInfo& create_info);

        Camera(const Camera& other) = default;
        Camera& operator=(const Camera& other) = default;

        Camera(Camera&& old) noexcept = default;
        Camera& operator=(Camera&& old) noexcept = default;

        ~Camera() = default;

        [[nodiscard]] const rx::string& get_name() const;

    private:
        rx::string name;
    };

    using CameraAccessor = VectorAccessor<Camera>;
} // namespace nova::renderer
