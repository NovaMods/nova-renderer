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
#include <shaderc/shaderc.hpp>
#include "../../../utils/export.h"
#include "../../../data_loading/loaders/shader_source_structs.h"
#include "../renderpasses/materials.h"
#include "../../../data_loading/loaders/shader_loading.h"
#include "../renderpasses/renderpass_builder.h"
#include "../resources/texture2D.h"


namespace nova {
    class shader_resource_manager;

    struct resource_binding : public vk::DescriptorSetLayoutBinding {
        uint32_t set;

        bool operator==(const resource_binding& other) const;
        bool operator!=(const resource_binding& other) const;

        vk::DescriptorSetLayoutBinding to_vk_binding() const;
    };

    struct shader_module {
        vk::ShaderModule module;
        std::unordered_map<std::string, resource_binding> bindings;
    };

    struct pipeline_object {
        std::string name;
        vk::Pipeline pipeline;
        vk::PipelineLayout layout;
        std::unordered_map<std::string, resource_binding> resource_bindings;

        std::unordered_map<uint32_t, vk::DescriptorSetLayout> layouts;
        std::vector<vertex_field_enum> attributes;

        std::vector<vk::DescriptorSet> descriptors;

        std::unordered_map<std::string, const texture2D*> bound_textures;

        /*!
         * \brief Marks
         * \param descriptor_name
         * \param tex
         */
        void bind_resource(const std::string& descriptor_name, const texture2D& tex);

        void commit_bindings(const vk::Device& device, std::shared_ptr<shader_resource_manager> shader_resources) const;
    };

    std::unordered_map<std::string, std::vector<pipeline_object>> make_pipelines(const shaderpack_data& shaderpack,
                                                                              std::unordered_map<std::string, pass_vulkan_information> renderpasses_by_pass,
                                                                              std::shared_ptr<render_context> context);

    shader_module create_shader_module(const shader_file& source, const shaderc_shader_kind& stages,  const vk::Device& device);

    pipeline_object make_pipeline(const pipeline& pipeline_create_info, const pass_vulkan_information& renderpass_info, vk::Device device);

    void output_compiled_shader(const shader_file &original_shader_file, const std::vector<uint32_t>& spirv);

    std::vector<uint32_t> glsl_to_spirv(const std::vector<shader_line>& shader_lines, shaderc_shader_kind stages);

    std::unordered_map<std::string, resource_binding> get_interface_of_spirv(const std::vector<uint32_t>& spirv_source, const vk::ShaderStageFlags& stages);

    void add_bindings_from_shader(pipeline_object& pipeline_data, const shader_module &shader_module, const std::string& shader_stage_name, std::unordered_map<uint32_t, std::vector<vk::DescriptorSetLayoutBinding>>& all_layouts);
}

#endif //RENDERER_GL_SHADER_H
