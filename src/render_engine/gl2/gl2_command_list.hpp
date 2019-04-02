/*!
 * \author ddubois
 * \date 31-Mar-19.
 */

#ifndef NOVA_RENDERER_GL_2_COMMAND_LIST_HPP
#define NOVA_RENDERER_GL_2_COMMAND_LIST_HPP
#include <nova_renderer/command_list.hpp>

#include "glad.h"

namespace nova::renderer {
    enum class gl2_command_type {
        BUFFER_COPY,
        EXECUTE_COMMAND_LISTS,
        BEGIN_RENDERPASS,
        END_RENDERPASS,
        BIND_PIPELINE,
        BIND_MATERIAL,
        BIND_VERTEX_BUFFERS,
        BIND_INDEX_BUFFER,
        DRAW_INDEXED_MESH,
    };

    struct buffer_copy_command {
        GLuint destination_buffer;
        uint64_t destination_offset;

        GLuint source_buffer;
        uint64_t source_offset;

        uint64_t num_bytes;
    };

    struct execute_command_lists_command {
        std::vector<command_list*> lists_to_execute;
    };

    struct bind_renderpass_command {
        GLuint framebuffer;
    };

    struct bind_pipeline_command {};

    struct bind_material_command {};

    struct bind_vertex_buffers_command {};

    struct bind_index_buffer_command {};

    struct draw_indexed_mesh_command {};

    struct gl_command {
        gl2_command_type type;

        union {
            buffer_copy_command buffer_copy;
            execute_command_lists_command execute_command_lists;
            bind_renderpass_command bind_renderpass;
            bind_pipeline_command bind_pipeline;
            bind_material_command bind_material;
            bind_vertex_buffers_command bind_vertex_buffers;
            bind_index_buffer_command bind_index_buffer;
            draw_indexed_mesh_command draw_indexed_mesh;
        };
    };

    /*!
     * \brief Command list implementation for OpenGL 2.1
     *
     * This class is fun because OpenGL has no notion of a command list - it's synchronous af. Thus, this command list
     * is a custom thing that records commands into host memory. When this command list is submitted to a "queue", Nova
     * runs the OpenGL commands. This lets command lists be recorded from multiple threads, but submitting a command
     * list to OpenGL is _way_ more expensive than submitting a DirectX or Vulkan command list
     *
     * OpenGL also has a really fun property where it can't be multithreaded, especially since Nova has to use OpenGL
     * 2.1. The OpenGL render engine will chew through OpenGL commands in a separate thread, and in that way mimic the
     * async nature of modern APIs, but still... it'll be rough
     *
     * On the other hand, OpenGL has no concept of a resource barrier...
     */
    class gl2_command_list : public command_list {
    public:
        gl2_command_list();

        void resource_barriers([[maybe_unused]] pipeline_stage_flags stages_before_barrier,
                              [[maybe_unused]] pipeline_stage_flags stages_after_barrier,
                              [[maybe_unused]] const std::vector<resource_barrier_t>& barriers) override;

        void copy_buffer(resource_t* destination_buffer,
                         uint64_t destination_offset,
                         resource_t* source_buffer,
                         uint64_t source_offset,
                         uint64_t num_bytes) override;

        void execute_command_lists(const std::vector<command_list*>& lists) override;

        void begin_renderpass([[maybe_unused]] renderpass_t* renderpass, framebuffer_t* framebuffer) override;

        void end_renderpass() override;

        void bind_pipeline() override;

        void bind_material() override;

        void bind_vertex_buffers() override;

        void bind_index_buffer() override;

        void draw_indexed_mesh() override;

        ~gl2_command_list() override;

        /*!
         * \brief Provides access to the actual command list, so that the GL2 render engine can process the commands
         */
        std::vector<gl_command> get_commands() const;

    private:
        std::vector<gl_command> commands;
    };
} // namespace nova::renderer

#endif // NOVA_RENDERER_GL_2_COMMAND_LIST_HPP
