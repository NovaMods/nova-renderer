/*!
 * \author ddubois 
 * \date 31-Mar-19.
 */

#pragma once
#include <nova_renderer/render_engine.hpp>

namespace nova::renderer {
    /*!
     * \brief OpenGL 2.1 render engine because compatibility
     */
    class gl2_render_engine : render_engine {
    public:
        explicit gl2_render_engine(nova_settings& settings);

        gl2_render_engine(gl2_render_engine&& other) = delete;
        gl2_render_engine& operator=(gl2_render_engine&& other) noexcept = delete;

        gl2_render_engine(const gl2_render_engine& other) = delete;
        gl2_render_engine& operator=(const gl2_render_engine& other) = delete;

        ~gl2_render_engine() override;

        std::shared_ptr<iwindow> get_window() const override;

        void set_shaderpack(const shaderpack_data& data) override;

        result<renderable_id_t> add_renderable(const static_mesh_renderable_data& data) override;

        void set_renderable_visibility(renderable_id_t id, bool is_visible) override;

        void delete_renderable(renderable_id_t id) override;

        result<mesh_id_t> add_mesh(const mesh_data& mesh) override;

        void delete_mesh(uint32_t mesh_id) override;

        command_list* allocate_command_list(uint32_t thread_idx, queue_type needed_queue_type, command_list::level command_list_type) override;

        void render_frame() override;

    protected:
        void open_window(uint32_t width, uint32_t height) override;
    };
}
