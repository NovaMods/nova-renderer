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

#include <foundational/allocation/block_allocator.hpp>
#include <foundational/allocation/bump_point_allocator.hpp>

#include "nova_renderer/device_memory_resource.hpp"
#include "nova_renderer/nova_settings.hpp"
#include "nova_renderer/render_engine.hpp"
#include "nova_renderer/renderdoc_app.h"

#include "../../src/render_engine/configuration.hpp"
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

        std::vector<rhi::ResourceBarrier> read_texture_barriers;
        std::vector<rhi::ResourceBarrier> write_texture_barriers;
    };

    struct Mesh {
        rhi::Buffer* vertex_buffer = nullptr;
        rhi::Buffer* index_buffer = nullptr;

        uint32_t num_indices = 0;
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

        std::unordered_map<FullMaterialPassName, MaterialPassMetadata> material_metadatas{};
    };

    struct RenderpassMetadata {
        shaderpack::RenderPassCreateInfo data;

        std::unordered_map<std::string, PipelineMetadata> pipeline_metadata{};
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
        void execute_frame();

        NovaSettings& get_settings();

#pragma region Meshes
        /*!
         * \brief Tells Nova how many meshes you expect to have in your scene
         *
         * Allows the Nova Renderer to preallocate space for your meshes
         *
         * \param num_meshes The number of meshes you expect to have
         */
        void set_num_meshes(uint32_t num_meshes);

        /*!
         * \brief Creates a new mesh and uploads its data to the GPU, returning the ID of the newly created mesh
         *
         * \param mesh_data The mesh's initial data
         */
        [[nodiscard]] MeshId create_mesh(const MeshData& mesh_data);

        /*!
         * \brief Destroys the mesh with the provided ID, freeing up whatever VRAM it was using
         *
         * In debug builds, this method checks that no renderables are using the mesh
         *
         * \param mesh_to_destroy The handle of the mesh you want to destroy
         */
        void destroy_mesh(MeshId mesh_to_destroy);
#pragma endregion

        RenderableId add_renderable_for_material(const FullMaterialPassName& material_name, const StaticMeshRenderableData& renderable);

        [[nodiscard]] rhi::RenderEngine* get_engine() const;

        static NovaRenderer* initialize(const NovaSettings& settings);

        static NovaRenderer* get_instance();

        static void deinitialize();

    private:
        NovaSettings render_settings;
        std::unique_ptr<rhi::RenderEngine> rhi;

        RENDERDOC_API_1_3_0* render_doc;
        static std::unique_ptr<NovaRenderer> instance;

        rhi::Sampler* point_sampler;

        std::unique_ptr<DeviceMemoryResource> mesh_memory;

        std::unique_ptr<DeviceMemoryResource> ubo_memory;
        std::unique_ptr<DeviceMemoryResource> staging_buffer_memory;
        void* staging_buffer_memory_ptr;

#pragma region Initialization
        /*!
         * \brief Creates global GPU memory pools
         *
         * Creates pools for mesh data and for uniform buffers. The size of the mesh memory pool is a guess that might be true for some
         * games, I'll get more accurate guesses when I have actual data. The size of the uniform buffer pool is the size of the builtin
         * uniform buffers plus memory for the estimated number of renderables, which again will just be a guess and probably not a
         * super good one
         */
        void create_global_gpu_pools();

        void create_global_sync_objects();
#pragma endregion

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

#pragma region Meshes
        MeshId next_mesh_id = 0;

        std::unordered_map<MeshId, Mesh> meshes;
#pragma endregion

#pragma region Rendering
        uint64_t frame_count = 0;
        uint8_t cur_frame_idx = 0;

        std::array<rhi::Fence*, NUM_IN_FLIGHT_FRAMES> frame_fences;

        std::unordered_map<std::string, RenderpassMetadata> renderpass_metadatas;
        std::unordered_map<FullMaterialPassName, MaterialPassKey> material_pass_keys;
        
        void record_renderpass(const Renderpass& renderpass, rhi::CommandList* cmds);
        
        void record_pipeline(const Pipeline& pipeline, rhi::CommandList* cmds);

        void record_material_pass(const MaterialPass& pass, rhi::CommandList* cmds);
#endif
    };
} // namespace nova::renderer

#endif // NOVA_RENDERER_NOVA_RENDERER_H
