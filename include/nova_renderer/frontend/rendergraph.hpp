#pragma once

#include "nova_renderer/frame_context.hpp"
#include "nova_renderer/frontend/procedural_mesh.hpp"
#include "nova_renderer/renderables.hpp"
#include "nova_renderer/rhi/rhi_types.hpp"
#include "nova_renderer/util/container_accessor.hpp"

namespace nova::renderer {
    namespace shaderpack {
        struct RenderPassCreateInfo;
    };

#pragma region Structs for rendering
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

        std::pmr::vector<RenderCommandType> commands;
    };

    template <typename RenderCommandType>
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

        std::pmr::vector<RenderCommandType> commands;

        ProceduralMeshBatch(std::unordered_map<MeshId, ProceduralMesh>* meshes, const MeshId key) : mesh(meshes, key) {}
    };

    struct MaterialPass {
        std::pmr::vector<MeshBatch<StaticMeshRenderCommand>> static_mesh_draws;
        std::pmr::vector<ProceduralMeshBatch<StaticMeshRenderCommand>> static_procedural_mesh_draws;

        std::pmr::vector<rhi::DescriptorSet*> descriptor_sets;
        const rhi::PipelineInterface* pipeline_interface = nullptr;

        void record(rhi::CommandList* cmds, FrameContext& ctx) const;
        static void record_rendering_static_mesh_batch(const MeshBatch<StaticMeshRenderCommand>& batch,
                                                       rhi::CommandList* cmds,
                                                       FrameContext& ctx);
        static void record_rendering_static_mesh_batch(const ProceduralMeshBatch<StaticMeshRenderCommand>& batch,
                                                       rhi::CommandList* cmds,
                                                       FrameContext& ctx);
    };

    struct Pipeline {
        rhi::Pipeline* pipeline = nullptr;
        rhi::PipelineInterface* pipeline_interface = nullptr;

        void record(rhi::CommandList* cmds, FrameContext& ctx) const;
    };
#pragma endregion

#pragma region Metadata structs
    struct FullMaterialPassName {
        std::string material_name;
        std::string pass_name;

        bool operator==(const FullMaterialPassName& other) const;
    };

    struct FullMaterialPassNameHasher {
        std::size_t operator()(const FullMaterialPassName& name) const;
    };

    struct MaterialPassKey {
        std::string pipeline_name;
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
    };
#pragma endregion

    /*!
     * \brief Renderpass that's ready to be recorded into a command list
     *
     * Renderpass has two virtual methods: `record` and `record_inside_renderpass`. `record` records the renderpass in its entirety, while
     * `record_inside_renderpass` only records the inside of the renderpass, not the work needed to begin or end it. I expect that most
     * subclasses will only want to override `record_inside_renderpass`
     */
    class Renderpass {
    public:
        Renderpass() = default;

        Renderpass(Renderpass&& old) noexcept = default;
        Renderpass& operator=(Renderpass&& old) noexcept = default;

        Renderpass(const Renderpass& other) = default;
        Renderpass& operator=(const Renderpass& other) = default;

        virtual ~Renderpass() = default;

        uint32_t id = 0;

        bool is_builtin = false;

        rhi::Renderpass* renderpass = nullptr;
        rhi::Framebuffer* framebuffer = nullptr;

        /*!
         * \brief Names of all the pipelines which are in this renderpass
         */
        std::pmr::vector<std::string> pipeline_names;

        bool writes_to_backbuffer = false;

        std::pmr::vector<rhi::ResourceBarrier> read_texture_barriers;
        std::pmr::vector<rhi::ResourceBarrier> write_texture_barriers;

        /*!
         * \brief Performs the rendering work of this renderpass
         *
         * Custom renderpasses can override this method to perform custom rendering. However, I recommend that you override
         * `render_renderpass_contents` instead. A typical renderpass will need to issue barriers for the resource it uses, and
         * the default renderpass implementation calls `render_renderpass_contents` after issuing those barriers
         *
         * \param cmds The command list that this renderpass should record all its rendering commands into. You may record secondary command
         * lists in multiple threads and execute them with this command list, if you want
         *
         * \param ctx The context for the current frame. Contains information about the available resources, the current frame, and
         * everything you should need to render. If there's something you need that isn't in the frame context, submit an issue on the Nova
         * GitHub
         */
        virtual void render(rhi::CommandList* cmds, FrameContext& ctx);

        /*!
         * \brief Returns the framebuffer that this renderpass should render to
         */
        [[nodiscard]] rhi::Framebuffer* get_framebuffer(const FrameContext& ctx) const;

    protected:
        /*!
         * \brief Records all the resource barriers that need to take place before this renderpass renders anything
         *
         * By default `render` calls this method before calling `render_renderpass_contents`. If you override `render`, you'll need to call
         * this method yourself before using any of this renderpass's resources
         */
        void record_pre_renderpass_barriers(rhi::CommandList* cmds, FrameContext& ctx) const;

        /*!
         * \brief Renders the contents of this renderpass
         *
         * The default `render` method calls this method after `record_pre_renderpass_barriers` and before
         * `record_post_renderpass_barriers`. Thus, I recommend that you override this method instead of `render` - you'll have fewer things
         * to worry about
         *
         * \param cmds The command list that this renderpass should record all its rendering commands into. You may record secondary command
         * lists in multiple threads and execute them with this command list, if you want
         *
         * \param ctx The context for the current frame. Contains information about the available resources, the current frame, and
         * everything you should need to render. If there's something you need that isn't in the frame context, submit an issue on the Nova
         * GitHub
         */
        virtual void render_renderpass_contents(rhi::CommandList* cmds, FrameContext& ctx);

        /*!
         * \brief Records all the resource barriers that need to take place after this renderpass renders anything
         *
         * By default `render` calls this method after calling `render_renderpass_contents`. If you override `render`, you'll need to call
         * this method yourself near the end of your `render` method
         */
        void record_post_renderpass_barriers(rhi::CommandList* cmds, FrameContext& ctx) const;
    };
} // namespace nova::renderer
