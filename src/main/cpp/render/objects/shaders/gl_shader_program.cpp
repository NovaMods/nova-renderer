/*!
 * \author David
 * \date 17-May-16.
 */

#include <cstdlib>
#include <algorithm>

#include <easylogging++.h>
#include "gl_shader_program.h"
#include "../../vulkan/render_context.h"

namespace nova {
    gl_shader_program::gl_shader_program(const shader_definition &source, vk::RenderPass renderpass) : name(source.name) {
        device = render_context::instance.device;
        LOG(TRACE) << "Creating shader with filter expression " << source.filter_expression;
        filter = source.filter_expression;
        LOG(TRACE) << "Created filter expression " << filter;
        create_shader(source.vertex_source, vk::ShaderStageFlagBits::eVertex);
        LOG(TRACE) << "Creatd vertex shader";
        create_shader(source.fragment_source, vk::ShaderStageFlagBits::eFragment);
        LOG(TRACE) << "Created fragment shader";

        link(renderpass);
    }

    gl_shader_program::gl_shader_program(gl_shader_program &&other) noexcept :
            name(std::move(other.name)), filter(std::move(other.filter)) {
    }

    void gl_shader_program::link(vk::RenderPass pass) {
        // Creates a pipeline out of compiled shaders

        std::vector<vk::PipelineShaderStageCreateInfo> stage_create_infos;

        vk::PipelineShaderStageCreateInfo vertex_create_info = {};
        vertex_create_info.stage = vk::ShaderStageFlagBits::eVertex;
        vertex_create_info.module = vertex_module;
        vertex_create_info.pName = "main";
        stage_create_infos.push_back(vertex_create_info);

        vk::PipelineShaderStageCreateInfo fragment_create_info = {};
        fragment_create_info.stage = vk::ShaderStageFlagBits::eFragment;
        fragment_create_info.module = fragment_module;
        fragment_create_info.pName = "main";
        stage_create_infos.push_back(fragment_create_info);


    }

    void gl_shader_program::bind() noexcept {
        //glUseProgram(gl_name);
    }

    gl_shader_program::~gl_shader_program() {
        device.destroyShaderModule(vertex_module);
        device.destroyShaderModule(fragment_module);

        if(geometry_module) {
            device.destroyShaderModule(*geometry_module);
        }
        if(tessellation_evaluation_module) {
            device.destroyShaderModule(*tessellation_evaluation_module);
        }
        if(tessellation_control_module) {
            device.destroyShaderModule(*tessellation_control_module);
        }
    }

    void gl_shader_program::create_shader(const std::vector<uint32_t>& shader_source, const vk::ShaderStageFlags flags) {
        vk::ShaderModuleCreateInfo create_info = {};
        create_info.codeSize = shader_source.size() * sizeof(uint32_t);
        create_info.pCode = shader_source.data();

        auto module = device.createShaderModule(create_info);

        if(flags == vk::ShaderStageFlagBits::eVertex) {
            vertex_module = module;
        } else if(flags == vk::ShaderStageFlagBits::eFragment) {
            fragment_module = module;
        }
    }

    std::string & gl_shader_program::get_filter() noexcept {
        return filter;
    }

    std::string &gl_shader_program::get_name() noexcept {
        return name;
    }

    wrong_shader_version::wrong_shader_version(const std::string &version_line) :
            std::runtime_error(
                    "Invalid version line: '" + version_line + "'. Please only use GLSL version 450 (NOT compatibility profile)"
            ) {}

    compilation_error::compilation_error(const std::string &error_message,
                                         const std::vector<shader_line> source_lines) :
            std::runtime_error(error_message + get_original_line_message(error_message, source_lines)) {}

    std::string compilation_error::get_original_line_message(const std::string &error_message,
                                                             const std::vector<shader_line> source_lines) {
        return "This logic isn't implemented yet";
    }

}
