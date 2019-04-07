/*!
 * \brief Main class for Nova. This class exists as a singleton so it's always available
 *
 * \author ddubois
 * \date 14-Aug-18.
 */

#ifndef NOVA_RENDERER_NOVA_RENDERER_H
#define NOVA_RENDERER_NOVA_RENDERER_H

#include <memory>
#include <string>

#include "nova_settings.hpp"
#include "render_engine_t.hpp"
#include "renderdoc_app.h"

namespace nova::renderer {
    NOVA_EXCEPTION(already_initialized_exception);
    NOVA_EXCEPTION(uninitialized_exception);

#pragma region Runtime - optimized data
    struct material_pass_t {};

    struct pipeline_t {
        rhi::pipeline_t* pipeline;

        std::vector<material_pass_t> passes;
    };

    struct renderpass_t {
        rhi::renderpass_t* renderpass;
        rhi::framebuffer_t& framebuffer;

        std::vector<pipeline_t> pipelines;
    };
#pragma endregion

    /*!
     * \brief Main class for Nova. Owns all of Nova's resources and provides a way to access them
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
        std::unique_ptr<rhi::render_engine_t> engine;

        RENDERDOC_API_1_3_0* render_doc;
        static std::unique_ptr<nova_renderer> instance;

#pragma region Shaderpack
        bool shaderpack_loaded = false;

        /*!
         * \brief The renderpasses in the shaderpack, in submission order
         */
        std::vector<renderpass_t*> renderpasses;

        void destroy_render_passes();
#pragma endregion
    };
} // namespace nova::renderer

#endif // NOVA_RENDERER_NOVA_RENDERER_H
