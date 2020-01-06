#pragma once
#include <optional>
#include <string>

#include "nova_renderer/frontend/rendergraph.hpp"
#include "nova_renderer/rhi/forward_decls.hpp"
#include "nova_renderer/util/result.hpp"

namespace spirv_cross {
    struct Resource;
    class CompilerGLSL;
} // namespace spirv_cross

namespace nova::renderer {
    using PipelineReturn = std::tuple<Pipeline, PipelineMetadata>;

    class PipelineStorage {
    public:
        /*!
         * \brief Creates a new pipeline cache which will create its pipeline on the provided render device
         */
        PipelineStorage(NovaRenderer& renderer, mem::AllocatorHandle<>& allocator);

        PipelineStorage(const PipelineStorage& other) = delete;
        PipelineStorage& operator=(const PipelineStorage& other) = delete;

        PipelineStorage(PipelineStorage& old) noexcept = default;
        PipelineStorage& operator=(PipelineStorage& old) noexcept = default;

        ~PipelineStorage() = default;

        [[nodiscard]] std::optional<const Pipeline&> get_pipeline(const std::string& pipeline_name) const;

        [[nodiscard]] bool add_pipeline_from_shaderpack(const shaderpack::PipelineCreateInfo& create_info);

    private:
        NovaRenderer& renderer;

        rhi::RenderEngine* device;

        mem::AllocatorHandle<>& allocator;

        std::unordered_map<std::string, PipelineMetadata> pipeline_metadatas;

        std::unordered_map<FullMaterialPassName, MaterialPassKey> material_pass_keys;

        std::unordered_map<std::string, Pipeline> pipelines;

        [[nodiscard]] ntl::Result<PipelineReturn> create_graphics_pipeline(
            rhi::PipelineInterface* pipeline_interface, const shaderpack::PipelineCreateInfo& pipeline_create_info) const;

        [[nodiscard]] ntl::Result<rhi::PipelineInterface*> create_pipeline_interface(
            const shaderpack::PipelineCreateInfo& pipeline_create_info,
            const std::pmr::vector<shaderpack::TextureAttachmentInfo>& color_attachments,
            const std::optional<shaderpack::TextureAttachmentInfo>& depth_texture) const;

        static void get_shader_module_descriptors(const std::pmr::vector<uint32_t>& spirv,
                                                  rhi::ShaderStage shader_stage,
                                                  std::unordered_map<std::string, rhi::ResourceBindingDescription>& bindings);

        static void add_resource_to_bindings(std::unordered_map<std::string, rhi::ResourceBindingDescription>& bindings,
                                             rhi::ShaderStage shader_stage,
                                             const spirv_cross::CompilerGLSL& shader_compiler,
                                             const spirv_cross::Resource& resource,
                                             rhi::DescriptorType type);
    };
} // namespace nova::renderer
