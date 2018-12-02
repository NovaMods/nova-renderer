//
// Created by jannis on 29.08.18.
//

#ifndef NOVA_RENDERER_RENDER_ENGINE_HPP
#define NOVA_RENDERER_RENDER_ENGINE_HPP

#include <memory>
#include "../loading/shaderpack/shaderpack_data.hpp"
#include "../settings/nova_settings.hpp"
#include "../util/utils.hpp"
#include "ftl/task_scheduler.h"
#include "window.hpp"

namespace nova {
    NOVA_EXCEPTION(render_engine_initialization_exception);
    NOVA_EXCEPTION(render_engine_rendering_exception);

    struct full_vertex {
        glm::vec3 position;           // 12 bytes
        glm::vec3 normal;             // 12 bytes
        glm::vec3 tangent;            // 12 bytes
        glm::u16vec2 main_uv;         // 4 bytes
        glm::u8vec2 secondary_uv;     // 2 bytes
        uint32_t virtual_texture_id;  // 4 bytes
        glm::vec4 additional_stuff;   // 16 bytes
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

    /*!
     * \brief Abstract class for render backends
     *
     * The constructor should not make any initialization
     * All functions must be called after init(nova::settings) has been called except
     *   explicitly marked in the documentation
     */
    class render_engine {
    public:
        render_engine(const render_engine& other) = delete;
        render_engine(render_engine&& other) = delete;

        render_engine& operator=(const render_engine& other) = delete;
        render_engine& operator=(render_engine&& other) = delete;

        /*!
         * \brief The engine name, for example "vulkan-1.1"
         * \attention Can be called before init(nova::settings)
         *
         * C++ doesn't support static virtual methods, but if you don't provide this method then Nova won't compile so
         * it's documented here
         *
         * \return the engine name
         *
         * static const std::string get_engine_name() const;
         */

        /*!
         * \brief Needed to make destructor of subclasses called
         */
        virtual ~render_engine() = default;

        virtual std::shared_ptr<iwindow> get_window() const = 0;

        /*!
         * \brief Loads the specified shaderpack, building API-specific data structures
         *
         * \param data The shaderpack to load
         */
        virtual void set_shaderpack(const shaderpack_data& data) = 0;

        /*!
         * \brief Adds a mesh to this render engine
         *
         * The provided mesh data is uploaded to the GPU. The mesh's identifier is returned to you. This is all you
         * need for the operations that a Nova render engine supports
         *
         * \param mesh The mesh data to send to the GPU
         * \return The ID of the mesh that was just created
         */
        virtual uint32_t add_mesh(const mesh_data& mesh) = 0;

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
        explicit render_engine(const nova_settings& settings, ftl::TaskScheduler* scheduler) : scheduler(scheduler){};

        /*!
         * \brief Initializes the window with the given size, and creates the swapchain for that window
         * \param width The width, in pixels, of the desired window
         * \param height The height, in pixels of the desired window
         */
        virtual void open_window(uint32_t width, uint32_t height) = 0;

        ftl::TaskScheduler* scheduler;
    };
}  // namespace nova

#endif  // NOVA_RENDERER_RENDER_ENGINE_HPP
