#pragma once

#include <array>
#include <memory>
#include <mutex>
#include <string>

#include "nova_renderer/constants.hpp"
#include "nova_renderer/filesystem/virtual_filesystem.hpp"
#include "nova_renderer/frontend/procedural_mesh.hpp"
#include "nova_renderer/frontend/rendergraph.hpp"
#include "nova_renderer/frontend/resource_loader.hpp"
#include "nova_renderer/nova_settings.hpp"
#include "nova_renderer/renderables.hpp"
#include "nova_renderer/renderdoc_app.h"
#include "nova_renderer/rhi/device_memory_resource.hpp"
#include "nova_renderer/rhi/forward_decls.hpp"
#include "nova_renderer/rhi/render_engine.hpp"
#include "nova_renderer/util/container_accessor.hpp"

#include "frontend/pipeline_storage.hpp"

namespace spirv_cross {
    class CompilerGLSL;
    struct Resource;
} // namespace spirv_cross

namespace nova::renderer {
    class UiRenderpass;

    namespace rhi {
        class Swapchain;
    }

#pragma region Runtime optimized data
    struct Mesh {
        rhi::Buffer* vertex_buffer = nullptr;
        rhi::Buffer* index_buffer = nullptr;

        uint32_t num_indices = 0;
    };
#pragma endregion

    using ProceduralMeshAccessor = MapAccessor<MeshId, ProceduralMesh>;

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
         * \brief Gives Nova a function to use to render UI
         *
         * This function will be executed inside the builtin UI renderpass. This renderpass takes the output of the 3D renderer, adds the UI
         * on top of it, and writes that all to the backbuffer
         *
         * The first parameter to the function is the command list it must record UI rendering into, and the second parameter is the
         * rendering context for the current frame
         *
         * Before calling the UI render function, Nova records commands to begin a renderpass with one RGBA8 color attachment and one D24S8
         * depth/stencil attachment. After calling this function, Nova records commands to end that same renderpass. This allows the host
         * application to only care about rendering the UI, instead of worrying about any pass scheduling concerns
         *
         * \param ui_renderpass The renderpass to use for UI
         */
        void set_ui_renderpass(const std::shared_ptr<Renderpass>& ui_renderpass);

        std::optional<shaderpack::RenderPassCreateInfo> get_renderpass_metadata(const std::string& renderpass_name);

        /*!
         * \brief Executes a single frame
         */
        void execute_frame();

        NovaSettingsAccessManager& get_settings();

        mem::AllocatorHandle<>* get_global_allocator() const;

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
        [[nodiscard]] ProceduralMeshAccessor create_procedural_mesh(uint64_t vertex_size, uint64_t index_size);

        /*!
         * \brief Destroys the mesh with the provided ID, freeing up whatever VRAM it was using
         *
         * In debug builds, this method checks that no renderables are using the mesh
         *
         * \param mesh_to_destroy The handle of the mesh you want to destroy
         */
        void destroy_mesh(MeshId mesh_to_destroy);
#pragma endregion

// TODO: make a resource manager of some sort and name it something that doesn't make graphite mad
#pragma region Resources
        [[nodiscard]] rhi::Buffer* get_builtin_buffer(const std::string& buffer_name) const;

        [[nodiscard]] rhi::Sampler* get_point_sampler() const;
#pragma endregion

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

        [[nodiscard]] RenderableId add_renderable_for_material(const FullMaterialPassName& material_name,
                                                               const StaticMeshRenderableData& renderable);

        [[nodiscard]] rhi::RenderEngine* get_engine() const;

        [[nodiscard]] NovaWindow* get_window() const;

        [[nodiscard]] DeviceResources* get_resource_manager() const;

        [[nodiscard]] PipelineStorage* get_pipeline_storage();

        static NovaRenderer* initialize(const NovaSettings& settings);

        static NovaRenderer* get_instance();

        static void deinitialize();

    private:
        NovaSettingsAccessManager render_settings;

        std::unique_ptr<rhi::RenderEngine> rhi;
        std::shared_ptr<NovaWindow> window;
        rhi::Swapchain* swapchain;

        RENDERDOC_API_1_3_0* render_doc;
        std::pmr::vector<mem::AllocatorHandle<>> frame_allocators;

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
        std::unique_ptr<mem::AllocatorHandle<>> global_allocator;

        /*!
         * \brief Holds all the object loaded by the current rendergraph
         */
        std::unique_ptr<mem::AllocatorHandle<>> renderpack_allocator;

        std::unique_ptr<DeviceResources> resource_storage;

        std::unique_ptr<DeviceMemoryResource> mesh_memory;

        std::unique_ptr<DeviceMemoryResource> ubo_memory;
        std::unique_ptr<DeviceMemoryResource> staging_buffer_memory;
        void* staging_buffer_memory_ptr;

#pragma region Initialization
        void create_global_allocators();

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

        void create_resource_storage();

        void create_builtin_textures();

        void create_uniform_buffers();

        void create_builtin_renderpasses();
#pragma endregion

#pragma region Renderpack
        using PipelineReturn = std::tuple<Pipeline, PipelineMetadata>;

        bool shaderpack_loaded = false;

        std::mutex shaderpack_loading_mutex;

        std::optional<shaderpack::ShaderpackData> loaded_renderpack;
#pragma endregion

#pragma region Rendergraph
        std::unordered_map<std::string, rhi::Image*> builtin_images;
        std::unordered_map<std::string, std::shared_ptr<Renderpass>> builtin_renderpasses;

        /*!
         * \brief The renderpasses in the shaderpack, in submission order
         *
         * Each renderpass contains all the pipelines that use it. Each pipeline has all the material passes that use
         * it, and each material pass has all the meshes that are drawn with it, and each mesh has all the renderables
         * that use it
         *
         * Basically this vector contains all the data you need to render a frame
         */
        std::pmr::vector<std::shared_ptr<Renderpass>> renderpasses;

        std::unordered_map<std::string, shaderpack::TextureCreateInfo> dynamic_texture_infos;

        void create_dynamic_textures(const std::pmr::vector<shaderpack::TextureCreateInfo>& texture_create_infos);

        void create_render_passes(const std::pmr::vector<shaderpack::RenderPassCreateInfo>& pass_create_infos,
                                  const std::pmr::vector<shaderpack::PipelineCreateInfo>& pipelines,
                                  const std::pmr::vector<shaderpack::MaterialData>& materials);

        /*!
         * \brief Creates a single renderpass
         *
         * \param pipelines Create infos for all the pipelines that might use
         * \param materials All the materials that are relevant to the renderer
         * \param descriptor_pool Descriptor pool to use to allocate this renderpass's descriptors
         * \param create_info Information about how to make the renderpass
         * \param renderpass A pointer to a valid Renderpass object for this method to fill out
         *
         * \note The renderpass shared pointer comes from outside because the builtin passes need to use their own classes, but user passes
         * need to use the default class. This isn't great but I like it a lot better than making this method templated
         *
         * TODO: Don't create pipelines or materials in this method. Split that out, so we can take pipelines and materials out of the
         * renderpass struct
         */
        void add_render_pass(const shaderpack::RenderPassCreateInfo& create_info,
                             const std::pmr::vector<shaderpack::PipelineCreateInfo>& pipelines,
                             const std::pmr::vector<shaderpack::MaterialData>& materials,
                             rhi::DescriptorPool* descriptor_pool,
                             const std::shared_ptr<Renderpass>& renderpass);

        void destroy_dynamic_resources();

        void destroy_renderpasses();
#pragma endregion

#pragma region Rendering pipelines
        std::unique_ptr<PipelineStorage> pipeline_storage;

        std::unordered_map<std::string, std::vector<MaterialPass>> passes_by_pipeline;

        void create_pipelines(const std::pmr::vector<shaderpack::PipelineCreateInfo>& pipeline_create_infos);

        void create_materials_for_pipeline(
            Pipeline& pipeline,
            std::unordered_map<FullMaterialPassName, MaterialPassMetadata, FullMaterialPassNameHasher>& material_metadatas,
            const std::pmr::vector<shaderpack::MaterialData>& materials,
            const std::string& pipeline_name,
            const rhi::PipelineInterface* pipeline_interface,
            rhi::DescriptorPool* descriptor_pool,
            const MaterialPassKey& template_key);
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

        std::unordered_map<std::string, RenderpassMetadata> renderpass_metadatas;
        std::unordered_map<FullMaterialPassName, MaterialPassKey, FullMaterialPassNameHasher> material_pass_keys;

        std::mutex ui_function_mutex;
#pragma endregion
    };
} // namespace nova::renderer
