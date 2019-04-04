/*!
 * \author ddubois
 * \date 31-Mar-19.
 */

#pragma once
#include <nova_renderer/render_engine.hpp>
#include "glad.h"

namespace nova::renderer {
    struct gl_resource_t : resource_t {
        GLuint id;
    };

	struct gl_renderpass_t : renderpass_t {

	};

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

        std::shared_ptr<window> get_window() const override;

        // Inherited via render_engine
        virtual result<renderpass_t*> create_renderpass(const render_pass_create_info_t& data) override;
        virtual framebuffer_t* create_framebuffer(const std::vector<resource_t*>& attachments) override;
        virtual pipeline_t* create_pipeline(const pipeline_create_info_t& data) override;
        virtual resource_t* create_buffer(const buffer_create_info_t& info) override;
        virtual resource_t* create_texture(const texture2d_create_info_t& info) override;
        virtual semaphore_t* create_semaphore() override;
        virtual std::vector<semaphore_t*> create_semaphores(uint32_t num_semaphores) override;
        virtual fence_t* create_fence(bool signaled = false) override;
        virtual std::vector<fence_t*> create_fences(uint32_t num_fences, bool signaled = false) override;
        virtual void destroy_renderpass(renderpass_t* pass) override;
        virtual void destroy_pipeline(pipeline_t* pipeline) override;
        virtual void destroy_resource(resource_t* resource) override;
        virtual void destroy_semaphores(const std::vector<semaphore_t*>& semaphores) override;
        virtual void destroy_fences(const std::vector<fence_t*>& fences) override;
        command_list_t* allocate_command_list(uint32_t thread_idx,
                                              queue_type needed_queue_type,
                                              command_list_t::level command_list_type) override;
        virtual void submit_command_list(command_list_t* cmds,
                                         queue_type queue,
                                         fence_t* fence_to_signal = nullptr,
                                         const std::vector<semaphore_t*>& wait_semaphores = {},
                                         const std::vector<semaphore_t*>& signal_semaphores = {}) override;

    protected:
        static void set_initial_state();

        std::unique_ptr<window> window;

        void open_window(uint32_t width, uint32_t height) override;

        std::unordered_map<std::string, sampler_create_info_t> samplers;
    };
} // namespace nova::renderer
