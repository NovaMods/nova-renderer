#pragma once

#include <memory>

#include "nova_settings.hpp"
#include "renderables.hpp"
#include "shaderpack_data.hpp"
#include "util/platform.hpp"
#include "util/result.hpp"
#include "util/utils.hpp"
#include "window.hpp"

namespace nova::renderer {
#pragma region Forward declarations
    struct resource_t {};

    struct renderpass_t {};

    struct framebuffer_t {
        glm::uvec2 size;
    };

    class command_list;
#pragma endregion

    NOVA_EXCEPTION(render_engine_initialization_exception);
    NOVA_EXCEPTION(render_engine_rendering_exception);

#define NUM_THREADS 1

    enum class queue_type {
        GRAPHICS,
        TRANSFER,
        ASYNC_COMPUTE,
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
        render_engine(render_engine&& other) = delete;
        render_engine& operator=(render_engine&& other) noexcept = delete;

        render_engine(const render_engine& other) = delete;
        render_engine& operator=(const render_engine& other) = delete;

        /*!
         * \brief Needed to make destructor of subclasses called
         */
        virtual ~render_engine() = default;

        [[nodiscard]] virtual std::shared_ptr<iwindow> get_window() const = 0;

#pragma region Shaderpack
        /*!
         * \brief Creates a renderpass from the provided data
         * 
         * Renderpasses are created 100% upfront, meaning that the caller can't change anything about a renderpass
         * after it's been created
         * 
         * \param data The data to create a renderpass from
         * 
         * \return The newly created renderpass
         */
        [[nodiscard]] virtual renderpass_t create_renderpass(const render_pass_data& data) = 0;
#pragma endregion 

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
        virtual result<renderable_id_t> add_renderable(const static_mesh_renderable_data& data) = 0;

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
        virtual result<mesh_id_t> add_mesh(const mesh_data& mesh) = 0;

        /*!
         * \brief Deletes the mesh with the provided ID from the GPU
         *
         * \param mesh_id The ID of the mesh to delete
         */
        virtual void delete_mesh(uint32_t mesh_id) = 0;

        /*!
         * \brief Allocates a new command list that can be used from the provided thread and has the desired type
         * 
         * Ownership of the command list is given to the caller. You can record your commands into it, then submit it
         * to a queue. Submitting it gives ownership back to the render engine, and recording commands into a
         * submitted command list is not supported
         * 
         * There is one command list pool per swapchain image per thread. All the pools for one swapchain image are
         * reset at the beginning of a frame that renders to that swapchain image. This means that any command list
         * allocated in one frame will not be valid in the next frame. DO NOT hold on to command lists
         * 
         * Command lists allocated by this method are returned ready to record commands into - the caller doess't need
         * to begin the command list
         */
        virtual command_list* allocate_command_list(uint32_t thread_idx, queue_type needed_queue_type, command_list::level command_list_type) = 0;

        /*!
         * \brief Renders a frame like so well, you guys
         */
        virtual void render_frame() = 0;

    protected:
        /*!
         * \brief Initializes the engine, does **NOT** open any window
         * \param settings The settings passed to nova
         *
         * Intentionally does nothing. This constructor serves mostly to ensure that concrete render engines have a
         * constructor that takes in some settings
         *
         * \attention Called by nova
         */
        explicit render_engine(nova_settings& settings) : settings(settings){};

        /*!
         * \brief Initializes the window with the given size, and creates the swapchain for that window
         * \param width The width, in pixels, of the desired window
         * \param height The height, in pixels of the desired window
         */
        virtual void open_window(uint32_t width, uint32_t height) = 0;

        nova_settings& settings;
    };
} // namespace nova::renderer
