//
// Created by jannis on 29.08.18.
//

#ifndef NOVA_RENDERER_RENDER_ENGINE_HPP
#define NOVA_RENDERER_RENDER_ENGINE_HPP

#include <memory>
#include "../../platform.hpp"
#include "../loading/shaderpack/shaderpack_data.hpp"
#include "../settings/nova_settings.hpp"
#include "../tasks/task_scheduler.hpp"
#include "../util/result.hpp"
#include "../util/utils.hpp"
#include "window.hpp"

namespace nova::renderer {
    NOVA_EXCEPTION(render_engine_initialization_exception);
    NOVA_EXCEPTION(render_engine_rendering_exception);

    struct full_vertex {
        glm::vec3 position;          // 12 bytes
        glm::vec3 normal;            // 12 bytes
        glm::vec3 tangent;           // 12 bytes
        glm::u16vec2 main_uv;        // 4 bytes
        glm::u8vec2 secondary_uv;    // 2 bytes
        uint32_t virtual_texture_id; // 4 bytes
        glm::vec4 additional_stuff;  // 16 bytes
    };

    static_assert(sizeof(full_vertex) % 16 == 0, "full_vertex struct is not aligned to 16 bytes!");

    /*!
     * \brief All the data needed to make a single mesh
     *
     * Meshes all have the same data. Chunks need all the mesh data, and they're most of the world. Entities, GUI,
     * particles, etc will probably have FAR fewer vertices than chunks, meaning that there's not a huge savings by
     * making them use special vertex formats
     */
    struct mesh_data {
        std::vector<full_vertex> vertex_data;
        std::vector<uint32_t> indices;
    };

    using mesh_id_t = uint32_t;

    struct static_mesh_renderer_update_data {
        std::string material_name;

        mesh_id_t mesh;
    };

    struct static_mesh_renderer_data : static_mesh_renderer_update_data {
        glm::vec3 initial_position;
        glm::vec3 initial_rotation;
        glm::vec3 initial_scale;
    };

    using renderable_id_t = uint32_t;

    /*!
     * \brief Abstract class for render backends
     *
     * The constructor should not make any initialization
     * All functions must be called after init(nova::settings) has been called except
     *   explicitly marked in the documentation
     */
    class render_engine {
    public:
        render_engine(render_engine&& other) = delete;
        render_engine& operator=(render_engine&& other) noexcept = delete;

        render_engine(const render_engine& other) = delete;
        render_engine& operator=(const render_engine& other) = delete;

        /*!
         * \brief Needed to make destructor of subclasses called
         */
        virtual ~render_engine() = default;

        [[nodiscard]] virtual std::shared_ptr<iwindow> get_window() const = 0;

        /*!
         * \brief Loads the specified shaderpack, building API-specific data structures
         *
         * \param data The shaderpack to load
         */
        virtual void set_shaderpack(const shaderpack_data& data) = 0;

        /*!
         * \brief Adds a new static mesh renderable to this render engine
         *
         * A static mesh renderable tells Nova to render a specific mesh with a specific material and a specific
         * transform. Static mesh renderables cannot be updated after creation, letting Nova bake them together if
         * doing so would help performance
         *
         * \param data The initial data for this static mesh renderable. Includes things like initial transform, mesh,
         * and material
         *
         * \return The ID of the newly created renderable
         */
        virtual result<renderable_id_t> add_renderable(const static_mesh_renderer_data& data) = 0;

        /*!
         * \brief Sets the visibility of the renderable with the provided ID
         *
         * This method allows the host application to perform its own culling on renderables. If the host application
         * marks a renderable as invisible, that renderable will _always_ be invisible. If the host application marks
         * a renderable as visible, however, Nova will perform its own culling, which may cause Nova to not render your
         * renderable anyways
         *
         * \param id The ID of the renderable to set the visibility of
         * \param is_visible If false, the specified renderable will not be rendered. If true, Nova will render the
         * renderable if the renderable would be visible
         */
        virtual void set_renderable_visibility(renderable_id_t id, bool is_visible) = 0;

        /*!
         * \brief Deletes a renderable from Nova
         *
         * \param id The ID of the renderable to delete
         */
        virtual void delete_renderable(renderable_id_t id) = 0;

        /*!
         * \brief Adds a mesh to this render engine
         *
         * The provided mesh data is uploaded to the GPU. The mesh's identifier is returned to you. This is all you
         * need for the operations that a Nova render engine supports
         *
         * \param mesh The mesh data to send to the GPU
         * \return The ID of the mesh that was just created
         */
        virtual mesh_id_t add_mesh(const mesh_data& mesh) = 0;

        /*!
         * \brief Deletes the mesh with the provided ID from the GPU
         *
         * \param mesh_id The ID of the mesh to delete
         */
        virtual void delete_mesh(uint32_t mesh_id) = 0;

        /*!
         * \brief Renders a frame like so well, you guys
         */
        virtual void render_frame() = 0;

    protected:
        /*!
         * \brief Initializes the engine, does **NOT** open any window
         * \param settings The settings passed to nova
         * \param scheduler The task scheduler that this render engine should use
         *
         * Intentionally does nothing. This constructor serves mostly to ensure that concrete render engines have a
         * constructor that takes in some settings
         *
         * \attention Called by nova
         */
        explicit render_engine(const nova_settings& /* settings */, nova::ttl::task_scheduler* scheduler) : scheduler(scheduler){};

        /*!
         * \brief Initializes the window with the given size, and creates the swapchain for that window
         * \param width The width, in pixels, of the desired window
         * \param height The height, in pixels of the desired window
         */
        virtual void open_window(uint32_t width, uint32_t height) = 0;

        nova::ttl::task_scheduler* scheduler;
    };
} // namespace nova::renderer

#endif // NOVA_RENDERER_RENDER_ENGINE_HPP
