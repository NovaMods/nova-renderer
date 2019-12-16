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

    static std::string ui_pass_name = "UI";

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

        std::vector<RenderCommandType> commands;
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

        std::vector<RenderCommandType> commands;

        ProceduralMeshBatch(std::unordered_map<MeshId, ProceduralMesh>* meshes, const MeshId key) : mesh(meshes, key) {}
    };

    struct MaterialPass {
        std::vector<MeshBatch<StaticMeshRenderCommand>> static_mesh_draws;
        std::vector<ProceduralMeshBatch<StaticMeshRenderCommand>> static_procedural_mesh_draws;

        std::vector<rhi::DescriptorSet*> descriptor_sets;
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

        std::vector<MaterialPass> passes;

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

    /*!
     * \brief Renderpass that's ready to be recorded into a command list
     *
     * Renderpass has two virtual methods: `record` and `record_inside_renderpass`. `record` records the renderpass in its entirety, while
     * `record_inside_renderpass` only records the inside of the renderpass, not the work needed to begin or end it. I expect that most
     * subclasses will only want to override `record_inside_renderpass`
     */
    class Renderpass {
    public:
        virtual ~Renderpass() = default;
        uint32_t id = 0;

        rhi::Renderpass* renderpass = nullptr;
        rhi::Framebuffer* framebuffer = nullptr;

        std::vector<Pipeline> pipelines;

        bool writes_to_backbuffer = false;

        std::vector<rhi::ResourceBarrier> read_texture_barriers;
        std::vector<rhi::ResourceBarrier> write_texture_barriers;

        virtual void render(rhi::CommandList* cmds, FrameContext& ctx);

        void record_pre_renderpass_barriers(rhi::CommandList* cmds, FrameContext& ctx) const;

        virtual void render_pass_contents(rhi::CommandList* cmds, FrameContext& ctx);

        void record_post_renderpass_barriers(rhi::CommandList* cmds, FrameContext& ctx) const;

        /*!
         * \brief Returns the framebuffer that this renderpass should render to
         */
        [[nodiscard]] rhi::Framebuffer* get_framebuffer(const FrameContext& ctx) const;
    };
} // namespace nova::renderer
