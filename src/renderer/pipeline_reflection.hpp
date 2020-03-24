#pragma once
#include <rx/core/map.h>
#include <rx/core/string.h>

#include "nova_renderer/rhi/pipeline_create_info.hpp"
#include "nova_renderer/rhi/rhi_enums.hpp"
#include "nova_renderer/rhi/rhi_types.hpp"

namespace spirv_cross {
    struct Resource;
    class Compiler;
} // namespace spirv_cross

namespace nova::renderer {
    rx::map<rx::string, rhi::RhiResourceBindingDescription> get_all_descriptors(const RhiGraphicsPipelineState& pipeline_state);

    void get_shader_module_descriptors(const rx::vector<uint32_t>& spirv,
                                       rhi::ShaderStage shader_stage,
                                       rx::map<rx::string, rhi::RhiResourceBindingDescription>& bindings);

    void add_resource_to_bindings(rx::map<rx::string, rhi::RhiResourceBindingDescription>& bindings,
                                  rhi::ShaderStage shader_stage,
                                  const spirv_cross::Compiler& shader_compiler,
                                  const spirv_cross::Resource& resource,
                                  rhi::DescriptorType type);
} // namespace nova::renderer
