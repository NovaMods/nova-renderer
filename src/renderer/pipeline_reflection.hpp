#pragma once
#include <unordered_map>
#include <string>

#include "nova_renderer/rhi/pipeline_create_info.hpp"
#include "nova_renderer/rhi/rhi_enums.hpp"
#include "nova_renderer/rhi/rhi_types.hpp"

namespace spirv_cross {
    struct Resource;
    class Compiler;
} // namespace spirv_cross

namespace nova::renderer {
    std::unordered_map<std::string, rhi::RhiResourceBindingDescription> get_all_descriptors(const RhiGraphicsPipelineState& pipeline_state);

    void get_shader_module_descriptors(const std::vector<uint32_t>& spirv,
                                       rhi::ShaderStage shader_stage,
                                       std::unordered_map<std::string, rhi::RhiResourceBindingDescription>& bindings);

    void add_resource_to_bindings(std::unordered_map<std::string, rhi::RhiResourceBindingDescription>& bindings,
                                  rhi::ShaderStage shader_stage,
                                  const spirv_cross::Compiler& shader_compiler,
                                  const spirv_cross::Resource& resource,
                                  rhi::DescriptorType type);
} // namespace nova::renderer
