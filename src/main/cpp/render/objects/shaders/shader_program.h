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

    /*!
     * \brief Represents an OpenGL shader program
     *
     * Shader programs can include between two and five shaders. At the bare minimum, a shader program needs a vertex
     * shader and a fragment shader. A shader program can also have a geometry shader, a tessellation control shader,
     * and a tessellation evaluation shader. Note that if a shader program has one of the tessellation shaders, it must
     * also have the other tessellation shader.
     *
     * A gl_shader_program does a couple of things. First, it holds a reference to the OpenGL object. Second, it holds
     * all  the configuration options declared in the shader. Third, it possibly holds the uniforms and attributes
     * defined in this shader. There's a good chance that I won't end up with uniform and attribute information. This
     * class will also hold the map from line in the shader sent to the compiler and the line number and shader file
     * that the line came from on disk
     */
    class vk_shader_program {
    public:
        /*!
         * \brief Constructs a gl_shader_program
         */
        vk_shader_program(const shader_definition &source, const material_state& material, vk::RenderPass renderpass, vk::PipelineCache pipeline_cache, vk::Device device, std::shared_ptr<shader_resource_manager> shader_resources, glm::ivec2& window_size);

        /*!
         * \brief Default copy constructor
         *
         * \param other The thing to copy
         */
		vk_shader_program(const vk_shader_program &other) = default;

        vk_shader_program(vk_shader_program &&other) noexcept;

        vk_shader_program() = default;

        /*!
         * \brief Deletes this shader and all it holds dear
         */
        ~vk_shader_program();

        std::string& get_filter() noexcept;

        std::string& get_name() noexcept;

        vk::Pipeline get_pipeline() noexcept;

    private:
        std::string name;

        vk::Pipeline pipeline;

        vk::ShaderModule vertex_module;
        vk::ShaderModule fragment_module;
        std::experimental::optional<vk::ShaderModule> geometry_module;
        std::experimental::optional<vk::ShaderModule> tessellation_evaluation_module;
        std::experimental::optional<vk::ShaderModule> tessellation_control_module;

        vk::Device device;

        /*!
         * \brief The filter that the renderer should use to get the geometry for this shader
         *
         * Since there's a one-to-one correlation between shaders and filters, I thought it'd be best to put the
         * filter with the shader
         */
        std::string filter;

        void create_shader_module(const std::vector<uint32_t> &shader_source, vk::ShaderStageFlags flags);

        void create_pipeline(vk::RenderPass pass, const material_state &material, vk::PipelineCache cache, glm::ivec2& window_size);

        std::shared_ptr<shader_resource_manager> shader_resources;
    };

    struct pipeline_layout_info {
        vk::PipelineLayout pipeline_layout;
        std::unordered_map<std::string, std::pair<uint32_t, uint32_t>> resource_name_to_set_and_binding;
    };

    struct shader_module {
        vk::ShaderModule module;
    };

    std::unordered_map<std::string, std::vector<vk::Pipeline>> make_pipelines(const shaderpack_data& shaderpack,
                                                                              std::unordered_map<std::string, pass_vulkan_information> renderpasses_by_pass,
                                                                              std::shared_ptr<render_context> context, std::shared_ptr<shader_resource_manager> shader_resources);

    shader_module create_shader_module(const shader_file& source, const vk::ShaderStageFlags& stages,  const vk::Device& device, std::shared_ptr<shader_resource_manager> shader_resources);

    vk::Pipeline make_pipeline(const pipeline& pipeline_info, const pass_vulkan_information& renderpass_info, vk::Device device, std::shared_ptr<shader_resource_manager> shader_resources);

    std::vector<uint32_t> glsl_to_spirv(const std::vector<shader_line>& shader_lines, shaderc_shader_kind stages);

    pipeline_layout_info get_interface_of_spirv(const std::vector<uint32_t>& spirv_source, const vk::ShaderStageFlags& stages, vk::Device device);
}

#endif //RENDERER_GL_SHADER_H
