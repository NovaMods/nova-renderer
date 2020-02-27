#pragma once

#include "nova_renderer/rendergraph.hpp"
#include "nova_renderer/rhi/forward_decls.hpp"
#include "nova_renderer/util/result.hpp"

namespace spirv_cross {
    struct Resource;
    class CompilerGLSL;
} // namespace spirv_cross

namespace nova::renderer {
    struct ShaderSource;
    struct PipelineStateCreateInfo;

    struct PipelineReturn {
        Pipeline pipeline;
        PipelineMetadata metadata;
    };

    class PipelineStorage {
    public:
        /*!
         * \brief Creates a new pipeline cache which will create its pipeline on the provided render device
         */
        PipelineStorage(NovaRenderer& renderer, rx::memory::allocator& allocator);

        PipelineStorage(const PipelineStorage& other) = delete;
        PipelineStorage& operator=(const PipelineStorage& other) = delete;

        PipelineStorage(PipelineStorage&& old) noexcept = delete;
        PipelineStorage& operator=(PipelineStorage&& old) noexcept = delete;

        ~PipelineStorage() = default;

        [[nodiscard]] rx::optional<Pipeline> get_pipeline(const rx::string& pipeline_name) const;

        [[nodiscard]] bool create_pipeline(const PipelineStateCreateInfo& create_info);

    private:
        NovaRenderer& renderer;

        rhi::RenderDevice& device;

        rx::memory::allocator& allocator;

        rx::map<rx::string, PipelineMetadata> pipeline_metadatas;

        rx::map<FullMaterialPassName, MaterialPassKey> material_pass_keys;

        rx::map<rx::string, Pipeline> pipelines;

        [[nodiscard]] ntl::Result<PipelineReturn> create_graphics_pipeline(rhi::RhiPipelineInterface* pipeline_interface,
                                                                           const PipelineStateCreateInfo& pipeline_create_info) const;

        [[nodiscard]] ntl::Result<rhi::RhiPipelineInterface*> create_pipeline_interface(
            const PipelineStateCreateInfo& pipeline_create_info) const;

        static void get_shader_module_descriptors(const rx::vector<uint32_t>& spirv,
                                                  rhi::ShaderStage shader_stage,
                                                  rx::map<rx::string, rhi::RhiResourceBindingDescription>& bindings);

        static void add_resource_to_bindings(rx::map<rx::string, rhi::RhiResourceBindingDescription>& bindings,
                                             rhi::ShaderStage shader_stage,
                                             const spirv_cross::CompilerGLSL& shader_compiler,
                                             const spirv_cross::Resource& resource,
                                             rhi::DescriptorType type);
    };
} // namespace nova::renderer
