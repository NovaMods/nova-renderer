#pragma once

#include <array>
#include <memory>
#include <mutex>
#include <string>

#include "nova_renderer/device_memory_resource.hpp"
#include "nova_renderer/nova_settings.hpp"
#include "nova_renderer/polyalloc.hpp"
#include "nova_renderer/render_engine.hpp"
#include "nova_renderer/renderdoc_app.h"

#include "constants.hpp"
#include "renderables.hpp"
#include "util/container_accessor.hpp"
#include "frontend/procedural_mesh.hpp"

namespace spirv_cross {
    class CompilerGLSL;
    struct Resource;
} // namespace spirv_cross

namespace nova::renderer {
    namespace rhi {
        class Swapchain;
    }

#pragma region Runtime optimized data
    template <typename RenderCommandType>
    struct MeshBatch {
        rhi::Buffer* vertex_buffer = nullptr;
        rhi::Buffer* index_buffer = nullptr;

        /*!
         * \brief A buffer to hold all the per-draw data
         *
         * For example, a non-animated mesh just needs a mat4 for its model matrix
         *
         * This buffer gets re-written to every frame, since the number of renderables in this mesh batch might have changed. If there's
         * more renderables than the buffer can hold, it gets reallocated from the RHI
         */
        rhi::Buffer* per_renderable_data = nullptr;

        std::vector<RenderCommandType> commands;
    };

    template<typename RenderCommandType>
    struct ProceduralMeshBatch {
        MapAccessor<MeshId, ProceduralMesh> mesh;
        
        /*!
         * \brief A buffer to hold all the per-draw data
         *
         * For example, a non-animated mesh just needs a mat4 for its model matrix
         *
         * This buffer gets re-written to every frame, since the number of renderables in this mesh batch might have changed. If there's
         * more renderables than the buffer can hold, it gets reallocated from the RHI
         */
        rhi::Buffer* per_renderable_data = nullptr;

        std::vector<RenderCommandType> commands;

        ProceduralMeshBatch(std::unordered_map<MeshId, ProceduralMesh>* meshes, const MeshId key) : mesh(meshes, key) {}
    };

    struct MaterialPass {
        // Descriptors for the material pass

        std::vector<MeshBatch<StaticMeshRenderCommand>> static_mesh_draws;
        std::vector<ProceduralMeshBatch<StaticMeshRenderCommand>> static_procedural_mesh_draws;

        std::vector<rhi::DescriptorSet*> descriptor_sets;
        const rhi::PipelineInterface* pipeline_interface = nullptr;
    };

    struct Pipeline {
        rhi::Pipeline* pipeline = nullptr;

        std::vector<MaterialPass> passes;
    };

    struct Renderpass {
        uint32_t id = 0;

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

        bool operator==(const FullMaterialPassName& other) const;
    };

    struct FullMaterialPassNameHasher {
        std::size_t operator()(const FullMaterialPassName& name) const;
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

        std::unordered_map<FullMaterialPassName, MaterialPassMetadata, FullMaterialPassNameHasher> material_metadatas{};
    };

    struct RenderpassMetadata {
        shaderpack::RenderPassCreateInfo data;

        std::unordered_map<std::string, PipelineMetadata> pipeline_metadata{};
    };
#pragma endregion

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
        explicit NovaRenderer(const NovaSettings& settings);

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
         * Loading a shaderpack will cause a stall in the GPU. Nova will have to wait for all in-flight frames to finish, then replace the
         * current shaderpack with the new one, then start rendering. Replacing the shaderpack might also require reloading all chunks, if
         * the new shaderpack has different geometry filters then the current one
         *
         * \param shaderpack_name The name of the shaderpack to load
         */
        void load_shaderpack(const std::string& shaderpack_name);

        /*!
         * \brief Executes a single frame
         */
        void execute_frame();

        NovaSettingsAccessManager& get_settings();

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
         * \brief Creates a procedural mesh, returning both its mesh id and 
         */
        [[nodiscard]] MapAccessor<MeshId, ProceduralMesh> create_procedural_mesh(uint64_t vertex_size, uint64_t index_size);

        /*!
         * \brief Destroys the mesh with the provided ID, freeing up whatever VRAM it was using
         *
         * In debug builds, this method checks that no renderables are using the mesh
         *
         * \param mesh_to_destroy The handle of the mesh you want to destroy
         */
        void destroy_mesh(MeshId mesh_to_destroy);
#pragma endregion

        [[nodiscard]] RenderableId add_renderable_for_material(const FullMaterialPassName& material_name,
                                                               const StaticMeshRenderableData& renderable);

        [[nodiscard]] rhi::RenderEngine* get_engine() const;

        static NovaRenderer* initialize(const NovaSettings& settings);

        static NovaRenderer* get_instance();

        static void deinitialize();

    private:
        NovaSettingsAccessManager render_settings;
        std::unique_ptr<rhi::RenderEngine> rhi;
        rhi::Swapchain* swapchain;

        RENDERDOC_API_1_3_0* render_doc;
        static std::unique_ptr<NovaRenderer> instance;

        rhi::Sampler* point_sampler;

        /*!
         * \brief The allocator that all of Nova's memory will be allocated through
         *
         * Local allocators 0.1 uwu
         *
         * Right now I throw this allocator at the GPU memory allocators, because they need some way to allocate memory and I'm not about to
         * try and band-aid aid things together. Future work will have a better way to bootstrap Nova's allocators
         */
        std::shared_ptr<bvestl::polyalloc::allocator_handle> global_allocator;

        std::unique_ptr<DeviceMemoryResource> mesh_memory;

        std::unique_ptr<DeviceMemoryResource> ubo_memory;
        std::unique_ptr<DeviceMemoryResource> staging_buffer_memory;
        void* staging_buffer_memory_ptr;

#pragma region Initialization
        void create_global_allocator();

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

        void create_uniform_buffers();
#pragma endregion

#pragma region Shaderpack
        using PipelineReturn = std::tuple<Pipeline, PipelineMetadata>;

        bool shaderpack_loaded = false;

        std::mutex shaderpack_loading_mutex;

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

        void create_materials_for_pipeline(
            Pipeline& pipeline,
            std::unordered_map<FullMaterialPassName, MaterialPassMetadata, FullMaterialPassNameHasher>& material_metadatas,
            const std::vector<shaderpack::MaterialData>& materials,
            const std::string& pipeline_name,
            const rhi::PipelineInterface* pipeline_interface,
            rhi::DescriptorPool* descriptor_pool,
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

        [[nodiscard]] ntl::Result<rhi::PipelineInterface*> create_pipeline_interface(
            const shaderpack::PipelineCreateInfo& pipeline_create_info,
            const std::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
            const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture) const;

        [[nodiscard]] ntl::Result<PipelineReturn> create_graphics_pipeline(
            rhi::PipelineInterface* pipeline_interface, const shaderpack::PipelineCreateInfo& pipeline_create_info) const;

        static void get_shader_module_descriptors(const std::vector<uint32_t>& spirv,
                                                  rhi::ShaderStageFlags shader_stage,
                                                  std::unordered_map<std::string, rhi::ResourceBindingDescription>& bindings);

        static void add_resource_to_bindings(std::unordered_map<std::string, rhi::ResourceBindingDescription>& bindings,
                                             rhi::ShaderStageFlags shader_stage,
                                             const spirv_cross::CompilerGLSL& shader_compiler,
                                             const spirv_cross::Resource& resource,
                                             rhi::DescriptorType type);

        void destroy_render_passes();

        void destroy_dynamic_resources();
#pragma endregion

#pragma region Meshes
        MeshId next_mesh_id = 0;

        std::unordered_map<MeshId, Mesh> meshes;
        std::unordered_map<MeshId, ProceduralMesh> proc_meshes;
#pragma endregion

#pragma region Rendering
        uint64_t frame_count = 0;
        uint8_t cur_frame_idx = 0;

        std::unordered_map<std::string, rhi::Buffer*> builtin_buffers;
        uint32_t cur_model_matrix_index = 0;

        std::array<rhi::Fence*, NUM_IN_FLIGHT_FRAMES> frame_fences;

        std::vector<RenderpassMetadata> renderpass_metadatas;
        std::unordered_map<FullMaterialPassName, MaterialPassKey, FullMaterialPassNameHasher> material_pass_keys;

        void record_renderpass(Renderpass& renderpass, rhi::CommandList* cmds);

        void record_pipeline(Pipeline& pipeline, rhi::CommandList* cmds);
        void record_material_pass(MaterialPass& pass, rhi::CommandList* cmds);

        void record_rendering_static_mesh_batch(const MeshBatch<StaticMeshRenderCommand>& batch, rhi::CommandList* cmds);
        void record_rendering_static_mesh_batch(const ProceduralMeshBatch<StaticMeshRenderCommand>& batch, rhi::CommandList* cmds);
#pragma endregion
    };
} // namespace nova::renderer
