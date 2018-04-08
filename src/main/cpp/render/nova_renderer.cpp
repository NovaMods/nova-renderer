//
// Created by David on 25-Dec-15.
//

#define ELPP_FRESH_LOG_FILE

#include "nova_renderer.h"
#include "../utils/utils.h"
#include "../data_loading/loaders/loaders.h"
#include "objects/render_object.h"
#include "../data_loading/settings.h"
#include "windowing/glfw_vk_window.h"
#include "vulkan/command_pool.h"
#include "objects/meshes/mesh_store.h"
#include "objects/render_object.h"
#include "objects/meshes/vk_mesh.h"
#include "objects/uniform_buffers/uniform_buffer_definitions.h"
#include "objects/uniform_buffers/uniform_buffer_store.h"
#include "../input/InputHandler.h"
#include "objects/renderpasses/renderpass_builder.h"
#include "vulkan/render_context.h"
#include "objects/shaders/shader_resource_manager.h"
#include "render_graph.h"
#include "objects/meshes/vertex_attributes.h"

#include <easylogging++.h>
#include <glm/gtc/matrix_transform.hpp>
#include <minitrace.h>

INITIALIZE_EASYLOGGINGPP

#ifdef max
#undef max
#endif

namespace nova {
    std::unique_ptr<nova_renderer> nova_renderer::instance;
    std::shared_ptr<settings> nova_renderer::render_settings;

    nova_renderer::nova_renderer() {
        context = std::make_shared<render_context>();

        game_window = std::make_shared<glfw_vk_window>();
        LOG(TRACE) << "Window initialized";

        context->create_instance(*game_window);
        LOG(TRACE) << "Instance created";
        context->setup_debug_callback();
        LOG(TRACE) << "Debug callback set up";
        game_window->create_surface(context);
        LOG(TRACE) << "Created surface";
        context->find_device_and_queues();
        LOG(TRACE) << "Found device and queue";
        context->create_semaphores();
        LOG(TRACE) << "Created semaphores";
        context->create_command_pool_and_command_buffers();
        LOG(TRACE) << "Created command pool";
        context->create_swapchain(game_window->get_size());
        LOG(TRACE) << "Created swapchain";
        context->create_pipeline_cache();
        LOG(TRACE) << "Pipeline cache created";

        shader_resources = std::make_shared<shader_resource_manager>(context);

        LOG(INFO) << "Vulkan code initialized";

        ubo_manager = std::make_shared<uniform_buffer_store>();
        textures = std::make_shared<texture_manager>(context);
        meshes = std::make_shared<mesh_store>(context, shader_resources);
        inputs = std::make_shared<input_handler>();

		render_settings->register_change_listener(ubo_manager.get());
		render_settings->register_change_listener(game_window.get());
        render_settings->register_change_listener(this);

        render_settings->update_config_loaded();
		render_settings->update_config_changed();

        LOG(DEBUG) << "Finished sending out initial config";

        vk::SemaphoreCreateInfo create_info = {};
        swapchain_image_acquire_semaphore = context->device.createSemaphore(create_info);
        render_finished_semaphore = context->device.createSemaphore(create_info);
        LOG(TRACE) << "Created semaphores";

        vk::FenceCreateInfo fence_create_info = vk::FenceCreateInfo()
            .setFlags(vk::FenceCreateFlagBits::eSignaled);
        render_done_fence = context->device.createFence(fence_create_info);
    }

    nova_renderer::~nova_renderer() {
        // Ensure everything is done before we exit
        context->graphics_queue.waitIdle();
        LOG(TRACE) << "Waited for the GPU to be done";

        inputs.reset();
        LOG(TRACE) << "Released the inputs";
        meshes.reset();
        LOG(TRACE) << "Reset the meshes";
        textures.reset();
        LOG(TRACE) << "Reset the textures";
        ubo_manager.reset();
        LOG(TRACE) << "Reset the UBOs";

        auto& device = context->device;

        device.destroySemaphore(swapchain_image_acquire_semaphore);
        device.destroySemaphore(render_finished_semaphore);
        LOG(TRACE) << "Destroyed the semaphores";

        game_window.reset();
        LOG(TRACE) << "Reset the game window";

        shader_resources.reset();
        LOG(TRACE) << "Reset the shader resource manager";

        mtr_shutdown();
    }

    void nova_renderer::render_frame() {
        begin_frame();

        context->device.resetFences({render_done_fence});

        auto main_command_buffer = context->command_buffer_pool->get_command_buffer(0);
        main_command_buffer.buffer.reset(vk::CommandBufferResetFlagBits());

        vk::CommandBufferBeginInfo cmd_buf_begin_info = {};
        main_command_buffer.buffer.begin(cmd_buf_begin_info);

        player_camera.recalculate_frustum();

        // upload UBO things
        update_gbuffer_ubos();

        for(const auto& pass : passes_list) {
            execute_pass(pass, main_command_buffer.buffer);
        }

        main_command_buffer.buffer.end();

        vk::Semaphore wait_semaphores[] = {swapchain_image_acquire_semaphore};
        vk::PipelineStageFlags wait_stages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        vk::Semaphore signal_semaphores[] = {render_finished_semaphore};

        // TODO: Use the semiphores in render_context
        vk::SubmitInfo submit_info = vk::SubmitInfo()
                .setCommandBufferCount(1)
                .setPCommandBuffers(&main_command_buffer.buffer)
                .setWaitSemaphoreCount(1)
                .setPWaitSemaphores(wait_semaphores)
                .setPWaitDstStageMask(wait_stages)
                .setSignalSemaphoreCount(1)
                .setPSignalSemaphores(signal_semaphores);

        context->graphics_queue.submit(1, &submit_info, render_done_fence);

        vk::ResultValue<uint32_t> result = context->device.acquireNextImageKHR(context->swapchain, std::numeric_limits<uint64_t>::max(),
                                                                               swapchain_image_acquire_semaphore, vk::Fence());
        if(result.result != vk::Result::eSuccess) {
            LOG(ERROR) << "Could not acquire swapchain image! vkResult: " << result.result;
        }
        cur_swapchain_image_index = result.value;

        vk::Result swapchain_result = {};

        vk::PresentInfoKHR present_info = {};
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &render_finished_semaphore;
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &context->swapchain;
        present_info.pImageIndices = &cur_swapchain_image_index;
        present_info.pResults = &swapchain_result;

        context->present_queue.presentKHR(present_info);

        game_window->end_frame();

        auto fence_wait_result = context->device.waitForFences({render_done_fence}, true, std::numeric_limits<uint64_t>::max());
        if(fence_wait_result == vk::Result::eSuccess) {
            // Process geometry updates
            meshes->remove_old_geometry();
            meshes->upload_new_geometry();

        } else {
            LOG(WARNING) << "Could not wait for gui done fence, " << fence_wait_result;
        }
    }

    void nova_renderer::execute_pass(const render_pass &pass, vk::CommandBuffer& buffer) {
        if(renderpasses_by_pass.find(pass.name) == renderpasses_by_pass.end()) {
            LOG(ERROR) << "No renderpass defined for pass " << pass.name << ". Skipping this pass";
            return;
        }

        if(pipelines_by_pass.find(pass.name) == pipelines_by_pass.end()) {
            LOG(WARNING) << "No pipelines attached to pass " << pass.name << ". Skipping this pass";
            return;
        }

        const auto& renderpass_for_pass = renderpasses_by_pass.at(pass.name);

        vk::RenderPassBeginInfo begin_final_pass = vk::RenderPassBeginInfo()
                .setRenderPass(renderpass_for_pass.renderpass)
                .setFramebuffer(renderpass_for_pass.frameBuffer)
                .setRenderArea({{0, 0}, renderpass_for_pass.framebuffer_size});

        buffer.beginRenderPass(&begin_final_pass, vk::SubpassContents::eInline);

        const auto& pipeline_for_pass = pipelines_by_renderpass.at(pass.name);

        for(const auto& nova_pipeline : pipeline_for_pass) {
            render_pipeline(nova_pipeline, buffer);
        }

        buffer.endRenderPass();
    }

    void nova_renderer::render_pipeline(const pipeline_info &pipeline_data, vk::CommandBuffer& buffer) {
        if(material_passes_by_pipeline.find(pipeline_data.name) != material_passes_by_pipeline.end()) {
            LOG(WARNING) << "No material passes assigned to pipeline " << pipeline_data.name << ". Skipping this pipeline";
            return;
        }

        buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_data.pipeline);

        const auto& material_passes = material_passes_by_pipeline.at(pipeline_data.name);
        for(const auto mat : material_passes) {
            render_all_for_material_pass(mat, buffer, pipeline_data);
        }
    }

    void nova_renderer::render_all_for_material_pass(const material_pass pass, vk::CommandBuffer &buffer, const pipeline_info &pipeline_data) {
        const auto& meshes_for_mat = meshes->get_meshes_for_shader(pass.material_name);
        if(meshes_for_mat.empty()) {
            LOG(TRACE) << "No meshes available for material " << pass.material_name;
            return;
        }

        // Bind the descriptor sets for this material
        // TODO

        for(const auto& mesh : meshes_for_mat) {
            render_mesh(mesh, buffer, pipeline_data);
        }
    }

    void nova_renderer::render_mesh(const render_object &mesh, vk::CommandBuffer &buffer, const pipeline_info &pipeline_data) {
        buffer.bindIndexBuffer(mesh.geometry->indices, {0}, vk::IndexType::eUint32);

        for(uint32_t i = 0; i < pipeline_data.attributes.size(); i++) {
            auto attribute = pipeline_data.attributes[i];
            auto offset = get_all_vertex_attributes()[attribute.to_string()].offset;
            buffer.bindVertexBuffers(i, {mesh.geometry->vertex_buffer}, {offset});
        }

        buffer.drawIndexed(mesh.geometry->num_indices, 1, 0, 0, 0);
    }

    bool nova_renderer::should_end() {
        // If the window wants to close, the user probably clicked on the "X" button
        return game_window->should_close();
    }

    void nova_renderer::init() {
        mtr_init("nova_profile.json");
        MTR_META_PROCESS_NAME("Nova Renderer")
        MTR_META_THREAD_NAME("Main Nova Thread")

        MTR_SCOPE("INIT", "MainInit")
        render_settings = std::make_shared<settings>("config/config.json");

        try {
            instance = std::make_unique<nova_renderer>();
        } catch(std::exception& e) {
            LOG(ERROR) << "Could not initialize Nova cause " << e.what();
        }
    }

    void nova_renderer::on_config_change(nlohmann::json &new_config) {
		auto& shaderpack_name = new_config["loadedShaderpack"];
		LOG(INFO) << "Shaderpack name: " << shaderpack_name;

        bool shaderpack_in_settings_is_new = shaderpack_name != loaded_shaderpack_name;
        if(shaderpack_in_settings_is_new) {
            load_new_shaderpack(shaderpack_name);
        }
    }

    void nova_renderer::on_config_loaded(nlohmann::json &config) {
        // TODO: Probably want to do some setup here, don't need to do that now
    }

    settings &nova_renderer::get_render_settings() {
        return *render_settings;
    }

    texture_manager &nova_renderer::get_texture_manager() {
        return *textures;
    }

	glfw_vk_window &nova_renderer::get_game_window() {
		return *game_window;
	}

	input_handler &nova_renderer::get_input_handler() {
		return *inputs;
	}

    mesh_store &nova_renderer::get_mesh_store() {
        return *meshes;
    }

    void nova_renderer::load_new_shaderpack(const std::string &new_shaderpack_name) {
		LOG(INFO) << "Loading shaderpack " << new_shaderpack_name;

        auto shaderpack = load_shaderpack(new_shaderpack_name);

        pipelines_by_pass = shaderpack.pipelines_by_pass;
        materials = shaderpack.materials;

        LOG(INFO) << "Flattening frame graph...";
        try {
            passes_list = compile_into_list(shaderpack.passes);
        } catch(std::runtime_error& e) {
            LOG(ERROR) << "Could not load shaderpack " << new_shaderpack_name << ": " << e.what();

            // TODO: Find a good way to propagate the error
            return;
        }

        LOG(INFO) << "Initializing framebuffer attachments...";
        textures->create_dynamic_textures(shaderpack.dynamic_textures, passes_list);

        LOG(INFO) << "Building renderpasses and framebuffers...";
        renderpasses_by_pass = make_passes(shaderpack, textures, context);

        LOG(INFO) << "Building pipelines and compiling shaders...";
        pipelines_by_renderpass = make_pipelines(shaderpack, renderpasses_by_pass, context);

        LOG(INFO) << "Sorting materials...";
        material_passes_by_pipeline = extract_material_passes(shaderpack.materials);

        LOG(INFO) << "Loading complete";
    }

    void nova_renderer::deinit() {
        instance.release();
    }

    void nova_renderer::upload_gui_model_matrix(const render_object& gui_obj, const glm::mat4& model_matrix) {
        // Send the model matrix to the buffer
        // The per-model uniforms buffer is constantly mapped, so we can just grab the mapping from it
        auto& allocation = shader_resources->get_per_model_buffer()->get_allocation_info();
        memcpy(((uint8_t*)allocation.pMappedData) + gui_obj.per_model_buffer_range.offset, &model_matrix, gui_obj.per_model_buffer_range.range);

        // Copy the memory to the descriptor set
        auto write_ds = vk::WriteDescriptorSet()
            .setDstSet(gui_obj.per_model_set)
            .setDstBinding(0)
            .setDstArrayElement(0)
            .setDescriptorCount(1)
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setPBufferInfo((&gui_obj.per_model_buffer_range));

        context->device.updateDescriptorSets(1, &write_ds, 0, nullptr);
    }

    void nova_renderer::update_gbuffer_ubos() {
        // Big thing here is to update the camera's matrices

        auto& per_frame_ubo = ubo_manager->get_per_frame_uniforms();

        auto per_frame_uniform_data = per_frame_uniforms{};
        per_frame_uniform_data.gbufferProjection = player_camera.get_projection_matrix();
        per_frame_uniform_data.gbufferModelView = player_camera.get_view_matrix();

        per_frame_ubo.send_data(per_frame_uniform_data);
    }

    camera &nova_renderer::get_player_camera() {
        return player_camera;
    }

    std::vector<material>& nova_renderer::get_materials() {
        return materials;
    }

    void nova_renderer::end_frame() {

    }

    void nova_renderer::begin_frame() {
        LOG(TRACE) << "Beginning frame";
    }

    std::shared_ptr<render_context> nova_renderer::get_render_context() {
        return context;
    }

    std::shared_ptr<shader_resource_manager> nova_renderer::get_shader_resources() {
        return shader_resources;
    }

    void nova_renderer::update_gui_model_matrices() {
        auto& config = render_settings->get_options()["settings"];
        float view_width = config["viewWidth"];
        float view_height = config["viewHeight"];
        float scalefactor = config["scalefactor"];
        // The GUI matrix is super simple, just a viewport transformation
        gui_model = glm::mat4(1.0f);
        gui_model = glm::translate(gui_model, glm::vec3(-1.0f, -1.0f, 0.0f));
        gui_model = glm::scale(gui_model, glm::vec3(scalefactor, scalefactor, 1.0f));
        gui_model = glm::scale(gui_model, glm::vec3(1.0 / view_width, 1.0 / view_height, 1.0));

        try {
            if(!meshes) {
                LOG(ERROR) << "oh no the mesh store is not initialized";
                return;
            }

            std::vector<render_object> &gui_objects = meshes->get_meshes_for_shader("gui");
            for(const auto &gui_obj : gui_objects) {
                upload_gui_model_matrix(gui_obj, gui_model);
            }
        } catch(std::exception& e) {
            LOG(WARNING) << "Load some GUIs you fool";
            LOG(WARNING) << e.what() << std::endl;
        }
    }

    std::vector<render_pass> compile_into_list(std::unordered_map<std::string, render_pass> passes) {
        auto passes_dependency_order = order_passes(passes);
        auto ordered_passes = std::vector<render_pass>{};

        for(const auto& pass_name : passes_dependency_order) {
            ordered_passes.push_back(passes[pass_name]);
        }

        return ordered_passes;
    }

    std::unordered_map<std::string, std::vector<material_pass>> nova_renderer::extract_material_passes(const std::vector<material>& materials) {
        auto ordered_material_passes = std::unordered_map<std::string, std::vector<material_pass>>{};

        for(const auto& mat : materials) {
            for(const auto& mat_pass : mat.passes) {
                ordered_material_passes[mat_pass.pipeline].push_back(mat_pass);
            }
        }

        return ordered_material_passes;
    }
}

