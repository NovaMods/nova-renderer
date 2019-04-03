/*!
 * \author ddubois
 * \date 31-Mar-19.
 */

#include "gl2_render_engine.hpp"
#include "gl2_command_list.hpp"

#include "../../util/logger.hpp"

namespace nova::renderer {
    gl2_render_engine::gl2_render_engine(nova_settings& settings) : render_engine(settings) {
        const bool loaded_opengl = gladLoadGL() != 0;
        if(!loaded_opengl) {
            NOVA_LOG(FATAL) << "Could not load OpenGL 2.1 functions, sorry bro";
            return;
        }

        set_initial_state();
    }

    void gl2_render_engine::set_initial_state() {
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }

    void gl2_render_engine::set_shaderpack(const shaderpack_data_t& data) {
        destroy_dynamic_textures();

        create_dynamic_textures(data.resources.textures);
        for(const sampler_create_info_t& sampler_data : data.resources.samplers) {
            samplers.emplace(sampler_data.name, sampler_data);
        }
    }

    command_list_t* gl2_render_engine::allocate_command_list([[maybe_unused]] uint32_t thread_idx,
                                                           [[maybe_unused]] queue_type needed_queue_type,
                                                           [[maybe_unused]] command_list_t::level command_list_type) {
        return new gl2_command_list();
    }

    void gl2_render_engine::create_dynamic_textures(const std::vector<texture_create_into_t>& texture_datas) {
        dynamic_textures.reserve(texture_datas.size());

        std::vector<GLuint> texture_ids(texture_datas.size());
        glGenTextures(texture_ids.size(), texture_ids.data());

        for(const texture_create_into_t& data : texture_datas) {
            gl_texture texture = {};
            texture.id = texture_ids.front();
            texture_ids.pop_back();

            glBindTexture(GL_TEXTURE_2D, texture.id);

            GLenum internal_format = GL_RGBA8;
            GLenum format = GL_RGBA;
            GLenum type = GL_UNSIGNED_BYTE;

            switch(data.format.pixel_format) {
                case pixel_format_enum::RGBA8:
                    internal_format = GL_RGBA8;
                    format = GL_RGBA;
                    type = GL_UNSIGNED_BYTE;
                    break;

                case pixel_format_enum::RGBA32F:
                    NOVA_LOG(WARN)
                        << "You requested a texture with 128 bits per component, but your graphics card only supports 64 bits per component";
                case pixel_format_enum::RGBA16F:
                    internal_format = GL_RGBA16;
                    format = GL_RGBA;
                    type = GL_UNSIGNED_SHORT;
                    break;

                case pixel_format_enum::Depth:
                    internal_format = GL_DEPTH_COMPONENT32;
                    format = GL_RED;
                    type = GL_UNSIGNED_INT;
                    break;

                case pixel_format_enum::DepthStencil:
                    internal_format = GL_DEPTH_COMPONENT24;
                    format = GL_RED;
                    type = GL_UNSIGNED_SHORT;
                    break;
            }

            const glm::uvec2 texture_size = data.format.get_size_in_pixels(window->get_window_size());

            glTexImage2D(GL_TEXTURE_2D, 0, internal_format, texture_size.x, texture_size.y, 0, format, type, nullptr);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            dynamic_textures.emplace(data.name, texture);
        }
    }

    void gl2_render_engine::destroy_dynamic_textures() {
        std::vector<GLuint> textures_to_delete;
        textures_to_delete.reserve(dynamic_textures.size());
        for(const auto& [texture_name, texture] : dynamic_textures) {
            textures_to_delete.push_back(texture.id);
        }

        glDeleteTextures(textures_to_delete.size(), textures_to_delete.data());
        dynamic_textures.clear();
    }
} // namespace nova::renderer
