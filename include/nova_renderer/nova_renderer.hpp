#pragma once

#include <memory>
#include <string>

#include "nova_renderer/nova_settings.hpp"
#include "nova_renderer/render_engine.hpp"
#include "nova_renderer/renderdoc_app.h"
#include "renderables.hpp"

namespace spirv_cross {
    class CompilerGLSL;
    struct Resource;
} // namespace spirv_cross

namespace nova::renderer {
    NOVA_EXCEPTION(already_initialized_exception);
    NOVA_EXCEPTION(uninitialized_exception);

#pragma region Runtime optimized data
    template <typename RenderableType>
    struct MeshBatch {
        rhi::Resource* vertex_buffer = nullptr;
        rhi::Resource* index_buffer = nullptr;

        std::vector<RenderableType> renderables;
    };

    struct MaterialPass {
        // Descriptors for the material pass

        std::vector<MeshBatch<StaticMeshRenderCommand>> static_mesh_draws;
    };

    struct Pipeline {
        rhi::Pipeline* pipeline = nullptr;

        std::vector<MaterialPass> passes;
    };

    struct Renderpass {
        rhi::Renderpass* renderpass = nullptr;
        rhi::Framebuffer* framebuffer = nullptr;

        std::vector<Pipeline> pipelines;

        bool writes_to_backbuffer = false;
    };
#pragma endregion

    struct MaterialPassMetadata {
        shaderpack::MaterialPass data;
    };

    struct PipelineMetadata {
        shaderpack::PipelineCreateInfo data;

        std::unordered_map<std::string, MaterialPassMetadata> material_metadatas;
    };

    struct RenderpassMetadata {
        shaderpack::RenderPassCreateInfo data;

        std::unordered_map<std::string, PipelineMetadata> pipeline_metadata;
    };

    struct ResourceBinding {};

    /*!
     * \brief Main class for Nova. Owns all of Nova's resources and provides a way to access them
     * This class exists as a singleton so it's always available
     */
    class NovaRenderer {
    public:
        /*!
         * \brief Initializes the Nova Renderer
         */
        explicit NovaRenderer(NovaSettings settings);

        NovaRenderer(NovaRenderer&& other) noexcept = delete;
        NovaRenderer& operator=(NovaRenderer&& other) noexcept = delete;

        NovaRenderer(const NovaRenderer& other) = delete;
        NovaRenderer& operator=(const NovaRenderer& other) = delete;

        ~NovaRenderer();

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

        NovaSettings& get_settings();

        [[nodiscard]] rhi::RenderEngine* get_engine() const;

        static NovaRenderer* initialize(const NovaSettings& settings);

        static NovaRenderer* get_instance();

        static void deinitialize();

    private:
        NovaSettings render_settings;
        std::unique_ptr<rhi::RenderEngine> rhi;

        RENDERDOC_API_1_3_0* render_doc;
        static std::unique_ptr<NovaRenderer> instance;

#pragma region Shaderpack
        bool shaderpack_loaded = false;

        /*!
         * \brief The renderpasses in the shaderpack, in submission order
         */
        std::vector<Renderpass> renderpasses;

        std::unordered_map<std::string, rhi::Image*> dynamic_textures;
        std::unordered_map<std::string, shaderpack::TextureCreateInfo> dynamic_texture_infos;

        std::unordered_map<std::string, RenderpassMetadata> renderpass_metadatas;

        void create_dynamic_textures(const std::vector<shaderpack::TextureCreateInfo>& texture_create_infos);

        void create_render_passes(const std::vector<shaderpack::RenderPassCreateInfo>& pass_create_infos,
                                  const std::vector<shaderpack::PipelineCreateInfo>& pipelines,
                                  const std::vector<shaderpack::MaterialData>& materials);

        std::tuple<Pipeline, PipelineMetadata> create_graphics_pipeline(const rhi::Renderpass* renderpass,
                                                                        const std::vector<shaderpack::MaterialData>& materials,
                                                                        const shaderpack::PipelineCreateInfo& pipeline_create_info) const;

        static void get_shader_module_descriptors(const std::vector<uint32_t>& spirv,
                                                  const rhi::ShaderStageFlags shader_stage,
                                                  std::unordered_map<std::string, rhi::ResourceBindingDescription>& bindings);

        static void add_resource_to_bindings(std::unordered_map<std::string, rhi::ResourceBindingDescription>& bindings,
                                             const rhi::ShaderStageFlags shader_stage,
                                             const spirv_cross::CompilerGLSL& shader_compiler,
                                             const spirv_cross::Resource& resource,
                                             const rhi::DescriptorType type);

        void destroy_render_passes();

        void destroy_dynamic_resources();
#pragma endregion
    };
} // namespace nova::renderer
