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
        std::vector<rhi::DescriptorSet*> descriptor_sets;
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

#pragma region metadata
    struct FullMaterialPassName {
        std::string material_name;
        std::string pass_name;
    };

    struct MaterialPassKey {
        uint32_t renderpass_index;
        uint32_t pipeline_index;
        uint32_t material_pass_index;
    };

    struct MaterialPassMetadata {
        shaderpack::MaterialPass data;
    };

    struct PipelineMetadata {
        shaderpack::PipelineCreateInfo data;

        std::unordered_map<FullMaterialPassName, MaterialPassMetadata> material_metadatas;
    };

    struct RenderpassMetadata {
        shaderpack::RenderPassCreateInfo data;

        std::unordered_map<std::string, PipelineMetadata> pipeline_metadata;
    };
#pragma endregion

    struct ResourceBinding {};

    /*!
     * \brief Main class for Nova. Owns all of Nova's resources and provides a way to access them
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

        void add_renderable_for_material(const FullMaterialPassName& material_name, const StaticMeshRenderableData& renderable);

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
        using PipelineReturn = std::tuple<Pipeline, PipelineMetadata>;

        bool shaderpack_loaded = false;

        /*!
         * \brief The renderpasses in the shaderpack, in submission order
         *
         * Each renderpass contains all the pipelines that use it. Each pipeline has all the material passes that use
         * it, and each material pass has all the meshes that are drawn with it, and each mesh has all the renderables
         * that use it
         *
         * Basically this vector contains all the data you need to render a frame
         */
        std::vector<Renderpass> renderpasses;

        std::unordered_map<std::string, rhi::Image*> dynamic_textures;
        std::unordered_map<std::string, shaderpack::TextureCreateInfo> dynamic_texture_infos;

        std::unordered_map<std::string, RenderpassMetadata> renderpass_metadatas;
        std::unordered_map<FullMaterialPassName, MaterialPassKey> material_pass_keys;

        void create_dynamic_textures(const std::vector<shaderpack::TextureCreateInfo>& texture_create_infos);

        void create_render_passes(const std::vector<shaderpack::RenderPassCreateInfo>& pass_create_infos,
                                  const std::vector<shaderpack::PipelineCreateInfo>& pipelines,
                                  const std::vector<shaderpack::MaterialData>& materials);

        void create_materials_for_pipeline(Pipeline& pipeline,
                                           std::unordered_map<FullMaterialPassName, MaterialPassMetadata>& material_metadatas,
                                           const std::vector<shaderpack::MaterialData>& materials,
                                           const std::string& pipeline_name,
                                           const rhi::PipelineInterface* pipeline_interface,
                                           const rhi::DescriptorPool* descriptor_pool,
                                           const MaterialPassKey& template_key);

        /*!
         * \brief Binds the resources for one material to that material's descriptor sets
         * 
         * This method does not perform any validation. It assumes that descriptor_descriptions has a description for 
         * every descriptor that the material wants to bind to, and it assumes that material has all the needed
         * descriptors 
         *
         * \param material The material to bind resources to
         * \param bindings What resources should be bound to what descriptor. The key is the descriptor name and the
         * value is the resource name
         * \param descriptor_descriptions A map from descriptor name to all the information you need to update that
         * descriptor
         */
        void bind_data_to_material_descriptor_sets(
            const MaterialPass& material,
            const std::unordered_map<std::string, std::string>& bindings,
            const std::unordered_map<std::string, rhi::ResourceBindingDescription>& descriptor_descriptions);

        [[nodiscard]] Result<rhi::PipelineInterface*> create_pipeline_interface(
            const shaderpack::PipelineCreateInfo& pipeline_create_info,
            const std::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
            const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture) const;

        [[nodiscard]] Result<PipelineReturn> create_graphics_pipeline(const rhi::PipelineInterface* pipeline_interface,
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

#endif // NOVA_RENDERER_NOVA_RENDERER_H
