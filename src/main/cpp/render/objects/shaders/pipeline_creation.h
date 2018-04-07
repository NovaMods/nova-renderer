/*!
 * \brief Defines a shader program ("program" im OpenGL parlace) and a number of exceptions it can throw
 *
 * \author David
 * \date 17-May-16.
 */

#ifndef RENDERER_GL_SHADER_H
#define RENDERER_GL_SHADER_H

#include <istream>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.hpp>
#include "../../../utils/export.h"
#include "../../../data_loading/loaders/shader_source_structs.h"
#include "../renderpasses/materials.h"
#include "shader_resource_manager.h"
#include "../../../data_loading/loaders/shader_loading.h"
#include "../renderpasses/renderpass_builder.h"
#include "shader_resource_manager.h"


namespace nova {
    struct resource_binding : public vk::DescriptorSetLayoutBinding {
        uint32_t set;
    };

    using bindings_list = std::unordered_map<std::string, resource_binding>;

    struct shader_module {
        vk::ShaderModule module;
        bindings_list bindings;
    };

    struct pipeline_info {
        std::string name;
        vk::Pipeline pipeline;
        bindings_list resource_bindings;

        std::unordered_map<uint32_t, std::vector<vk::DescriptorSetLayoutBinding>> layout_bindings;
        std::unordered_map<uint32_t, vk::DescriptorSetLayout> layouts;
    };

    std::unordered_map<std::string, std::vector<pipeline_info>> make_pipelines(const shaderpack_data& shaderpack,
                                                                              std::unordered_map<std::string, pass_vulkan_information> renderpasses_by_pass,
                                                                              std::shared_ptr<render_context> context);

    shader_module create_shader_module(const shader_file& source, const vk::ShaderStageFlags& stages,  const vk::Device& device);

    pipeline_info make_pipeline(const pipeline& pipeline_info, const pass_vulkan_information& renderpass_info, vk::Device device);

    std::vector<uint32_t> glsl_to_spirv(const std::vector<shader_line>& shader_lines, shaderc_shader_kind stages);

    bindings_list get_interface_of_spirv(const std::vector<uint32_t>& spirv_source, const vk::ShaderStageFlags& stages);

    void add_bindings_from_shader(const pipeline_info& pipeline_data, const shader_module &shader_module, std::string shader_stage_name);
}

#endif //RENDERER_GL_SHADER_H
