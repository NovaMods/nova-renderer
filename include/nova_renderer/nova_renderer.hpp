#pragma once

#include <memory>
#include <string>

#include "nova_renderer/nova_settings.hpp"
#include "nova_renderer/render_engine_t.hpp"
#include "nova_renderer/renderdoc_app.h"

namespace nova::renderer {
    NOVA_EXCEPTION(already_initialized_exception);
    NOVA_EXCEPTION(uninitialized_exception);

#pragma region Runtime optimized data
    template <typename RenderableType>
    struct mesh_batch_t {
        rhi::resource_t* vertex_buffer;
        rhi::resource_t* index_buffer;

        std::vector<RenderableType> renderables;
    };

    struct material_pass_t {
        // Descriptors for the material pass

        std::vector<mesh_batch_t<static_mesh_renderable_t>> static_mesh_draws;
    };

    struct pipeline_t {
        rhi::pipeline_t* pipeline;

        std::vector<material_pass_t> passes;
    };

    struct renderpass_t {
        rhi::renderpass_t* renderpass;
        rhi::framebuffer_t* framebuffer;

        std::vector<pipeline_t> pipelines;

        bool writes_to_backbuffer;
    };
#pragma endregion

    /*!
     * \brief Main class for Nova. Owns all of Nova's resources and provides a way to access them
     * This class exists as a singleton so it's always available
     */
    class nova_renderer {
    public:
        /*!
         * \brief Initializes the Nova Renderer
         */
        explicit nova_renderer(nova_settings settings);

        nova_renderer(nova_renderer&& other) noexcept = delete;
        nova_renderer& operator=(nova_renderer&& other) noexcept = delete;

        nova_renderer(const nova_renderer& other) = delete;
        nova_renderer& operator=(const nova_renderer& other) = delete;

        ~nova_renderer();

        /*!
         * \brief Loads the shaderpack with the given name
         *
         * This method will first try to load from the `shaderpacks/` folder (mimicking Optifine shaders). If the
         * shaderpack isn't found there, it'll try to load it from the `resourcepacks/` directory (mimicking Bedrock
         * shaders). If the shader can't be found at either place, a `nova::resource_not_found` exception will be thrown
         *
         * \param shaderpack_name The name of the shaderpack to load
         */
        void load_shaderpack(const std::string& shaderpack_name);

        /*!
         * \brief Executes a single frame
         */
        void execute_frame() const;

        nova_settings& get_settings();

        [[nodiscard]] rhi::render_engine_t* get_engine() const;

        static nova_renderer* initialize(const nova_settings& settings);

        static nova_renderer* get_instance();

        static void deinitialize();

    private:
        nova_settings render_settings;
        std::unique_ptr<rhi::render_engine_t> rhi;

        RENDERDOC_API_1_3_0* render_doc;
        static std::unique_ptr<nova_renderer> instance;

#pragma region Shaderpack
        bool shaderpack_loaded = false;

        /*!
         * \brief The renderpasses in the shaderpack, in submission order
         */
        std::vector<renderpass_t> renderpasses;

        std::unordered_map<std::string, rhi::image_t*> dynamic_textures;
        std::unordered_map<std::string, shaderpack::texture_create_info_t> dynamic_texture_infos;

        void create_dynamic_textures(const std::vector<shaderpack::texture_create_info_t>& texture_create_infos);

        void create_render_passes(const std::vector<shaderpack::render_pass_create_info_t>& pass_create_infos);

        void destroy_render_passes();

        void destroy_dynamic_resources();
#pragma endregion
    };
} // namespace nova::renderer
