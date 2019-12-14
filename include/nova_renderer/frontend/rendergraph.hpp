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

    class MaterialPass {
    public:
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

    class Pipeline {
    public:
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

    class Renderpass {
    public:
        uint32_t id = 0;

        rhi::Renderpass* renderpass = nullptr;
        rhi::Framebuffer* framebuffer = nullptr;

        std::vector<Pipeline> pipelines;

        bool writes_to_backbuffer = false;

        std::vector<rhi::ResourceBarrier> read_texture_barriers;
        std::vector<rhi::ResourceBarrier> write_texture_barriers;

        /*!
         * \brief Tells this renderpass to use the provided function to record itself into a command list
         *
         * Nova expects that you'll only set this variable once ever. Otherwise, you may run into all kinds of threading issues
         *
         * This member allows a host application to register a custom command list recording function for this renderpass. It paves the way
         * for shaderpacks with render scripts
         *
         * The first parameter to this function is this exact renderpass, the second parameter is the command list to record into
         */
        std::optional<std::function<void(const Renderpass&, rhi::CommandList*, FrameContext&)>> record_func;

        void record(rhi::CommandList* cmds, FrameContext& ctx) const;

        void default_record(rhi::CommandList* cmds, FrameContext& ctx) const;

        void record_pre_renderpass_barriers(rhi::CommandList* cmds, FrameContext& ctx) const;

        void record_post_renderpass_barriers(rhi::CommandList* cmds, FrameContext& ctx) const;

        /*!
         * \brief Returns the framebuffer that this renderpass should render to
         */
        [[nodiscard]] rhi::Framebuffer* get_framebuffer(const FrameContext& ctx) const;
    };

    const shaderpack::RenderPassCreateInfo& get_ui_pass();
} // namespace nova::renderer
