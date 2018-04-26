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
#include "objects/resources/uniform_buffer_definitions.h"
#include "objects/resources/uniform_buffer_store.h"
#include "../input/InputHandler.h"
#include "objects/renderpasses/renderpass_builder.h"
#include "vulkan/render_context.h"
#include "objects/resources/shader_resource_manager.h"
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

        meshes = std::make_shared<mesh_store>(context, shader_resources);
        inputs = std::make_shared<input_handler>();

		render_settings->register_change_listener(&shader_resources->get_uniform_buffers());
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

        vk::ResultValue<uint32_t> result = context->device.acquireNextImageKHR(context->swapchain,
                                                                               std::numeric_limits<uint64_t>::max(),
                                                                               swapchain_image_acquire_semaphore,
                                                                               vk::Fence());
        if (result.result != vk::Result::eSuccess) {
            LOG(ERROR) << "Could not acquire swapchain image! vkResult: " << result.result;
        }
        cur_swapchain_image_index = result.value;

        context->device.resetFences({render_done_fence});

        auto main_command_buffer = context->command_buffer_pool->get_command_buffer(0);
        main_command_buffer.buffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        LOG(TRACE) << "Rendering with command buffer " << (VkCommandBuffer) main_command_buffer.buffer;

        vk::CommandBufferBeginInfo cmd_buf_begin_info = {};
        main_command_buffer.buffer.begin(cmd_buf_begin_info);

        player_camera.recalculate_frustum();

        update_nova_ubos();

        vk::ImageMemoryBarrier to_color_attachment_barrier = {};
        to_color_attachment_barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
        to_color_attachment_barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        to_color_attachment_barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        to_color_attachment_barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        to_color_attachment_barrier.subresourceRange.baseMipLevel = 0;
        to_color_attachment_barrier.subresourceRange.levelCount = 1;
        to_color_attachment_barrier.subresourceRange.baseArrayLayer = 0;
        to_color_attachment_barrier.subresourceRange.layerCount = 1;

        // This block seems weirdly hardcoded and not scalable but idk
        to_color_attachment_barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        to_color_attachment_barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        to_color_attachment_barrier.image = context->swapchain_images[cur_swapchain_image_index];
        to_color_attachment_barrier.oldLayout = context->swapchain_layout;
        context->swapchain_layout = vk::ImageLayout::eColorAttachmentOptimal;

        main_command_buffer.buffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::DependencyFlags(),
                0, nullptr,
                0, nullptr,
                1, &to_color_attachment_barrier);

        LOG(DEBUG) << "We have " << passes_list.size() << " passes to render";
        for (const auto &pass : passes_list) {
            execute_pass(pass, main_command_buffer.buffer);
        }

        vk::ImageMemoryBarrier barrier = {};
        barrier.newLayout = vk::ImageLayout::ePresentSrcKHR;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        // This block seems weirdly hardcoded and not scalable but idk
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        barrier.image = context->swapchain_images[cur_swapchain_image_index];
        barrier.oldLayout = context->swapchain_layout;
        context->swapchain_layout = vk::ImageLayout::ePresentSrcKHR;

        main_command_buffer.buffer.pipelineBarrier(
                vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::DependencyFlags(),
                0, nullptr,
                0, nullptr,
                1, &barrier);

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
        if (fence_wait_result == vk::Result::eSuccess) {
            // Process geometry updates
            meshes->remove_old_geometry();
            meshes->upload_new_geometry();

            context->command_buffer_pool->free(main_command_buffer);

        } else {
            LOG(WARNING) << "Could not wait for gui done fence, " << vk::to_string(fence_wait_result);
        }

        LOG(INFO) << "Frame done";
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

        LOG(INFO) << "Beginning pass " << pass.name;

        const auto& renderpass_for_pass = renderpasses_by_pass.at(pass.name);

        for(const auto& write_resource : renderpass_for_pass.texture_outputs) {
            // Transition all written to resources to shader write optimal

            vk::ImageMemoryBarrier barrier = {};
            barrier.newLayout = vk::ImageLayout::eColorAttachmentOptimal;
            barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            barrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            barrier.subresourceRange.baseMipLevel = 0;
            barrier.subresourceRange.levelCount = 1;
            barrier.subresourceRange.baseArrayLayer = 0;
            barrier.subresourceRange.layerCount = 1;

            // This block seems weirdly hardcoded and not scalable but idk
            vk::PipelineStageFlags source_stage = vk::PipelineStageFlagBits::eTopOfPipe;
            vk::PipelineStageFlags destination_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            barrier.srcAccessMask = vk::AccessFlags();
            barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

            if (write_resource == "Backbuffer") {
                barrier.image = context->swapchain_images[cur_swapchain_image_index];
                barrier.oldLayout = context->swapchain_layout;
                context->swapchain_layout = vk::ImageLayout::eColorAttachmentOptimal;

                continue;
            }

            try {
                // heavy handed but I don't have any good debugging tools right now so suck it nerds
                auto& tex = shader_resources->get_texture_manager().get_texture(write_resource);

                barrier.image = tex.get_vk_image();
                barrier.oldLayout = tex.get_layout();

                tex.set_layout(vk::ImageLayout::eColorAttachmentOptimal);

            } catch (std::domain_error &) {
                // Couldn't get the texture? All well. There'll be an error printed out already
                continue;
            }

            buffer.pipelineBarrier(
                    source_stage, destination_stage,
                    vk::DependencyFlags(),
                    0, nullptr,
                    0, nullptr,
                    1, &barrier);
        }

        vk::RenderPassBeginInfo begin_final_pass = vk::RenderPassBeginInfo()
                .setRenderPass(renderpass_for_pass.renderpass)
                .setFramebuffer(renderpass_for_pass.frameBuffer)
                .setRenderArea({{0, 0}, renderpass_for_pass.framebuffer_size});

        buffer.beginRenderPass(&begin_final_pass, vk::SubpassContents::eInline);

        auto& pipelines_for_pass = pipelines_by_renderpass.at(pass.name);
        LOG(INFO) << "Processing data in " << pipelines_for_pass.size() << " pipelines";

        for(auto& nova_pipeline : pipelines_for_pass) {
            render_pipeline(nova_pipeline, buffer);
        }

        buffer.endRenderPass();
    }

    void nova_renderer::render_pipeline(pipeline_object &pipeline_data, vk::CommandBuffer& buffer) {
        if(material_passes_by_pipeline.find(pipeline_data.name) == material_passes_by_pipeline.end()) {
            LOG(WARNING) << "No material passes assigned to pipeline " << pipeline_data.name << ". Skipping this pipeline";
            return;
        }
        LOG(INFO) << "Rendering pipeline " << pipeline_data.name;

        buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline_data.pipeline);

        const auto& material_passes = material_passes_by_pipeline.at(pipeline_data.name);
        LOG(INFO) << "There are " << material_passes.size() << " material passes";
        for(const auto& mat : material_passes) {
            render_all_for_material_pass(mat, buffer, pipeline_data);
        }
    }

    void nova_renderer::render_all_for_material_pass(const material_pass& pass, vk::CommandBuffer &buffer, pipeline_object &pipeline_data) {
        const auto& meshes_for_mat = meshes->get_meshes_for_material(pass.material_name);
        if(meshes_for_mat.empty()) {
            LOG(INFO) << "No meshes available for material " << pass.material_name;
            return;
        }

        LOG(INFO) << "Beginning material " << pass.material_name;

        auto& textures = shader_resources->get_texture_manager();
        auto& buffers = shader_resources->get_uniform_buffers();

        auto per_model_buffer_binding = std::string{};

        // Bind the descriptor sets for this material
        for(const auto& binding : pass.bindings) {
            const auto& descriptor_name = binding.first;
            const auto& resource_name = binding.second;

            if(textures.is_texture_known(resource_name)) {
                pipeline_data.bind_resource(descriptor_name, &textures.get_texture(resource_name));

            } else if(buffers.is_buffer_known(resource_name)) {
                // bind as a buffer

                if(resource_name != "NovaPerModelUBO") {
                    // Bind dis
                } else {
                    // Bind dis later
                    per_model_buffer_binding = descriptor_name;
                }

            } else {
                LOG(ERROR) << "Material " << pass.material_name << " wants to use resource " << resource_name << " for pipeline " << pass.pipeline << " but that resource doesn't exist! Check your spelling";
            }
        }

        pipeline_data.commit_bindings(context->device, shader_resources);

        LOG(INFO) << "Rendering " << meshes_for_mat.size() << " things";
        for(const auto& mesh : meshes_for_mat) {
            render_mesh(mesh, buffer, pipeline_data, per_model_buffer_binding);
        }
    }

    void nova_renderer::render_mesh(const render_object &mesh, vk::CommandBuffer &buffer, pipeline_object &pipeline_data, std::string string) {
        LOG(INFO) << "Binding model matrix descriptor " << (VkDescriptorSet)(mesh.model_matrix_descriptor) << " for render object " << mesh.id;
        const auto& descriptor = pipeline_data.resource_bindings[string];
        buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_data.layout, descriptor.set, 1, &mesh.model_matrix_descriptor, 0, nullptr);

        buffer.bindIndexBuffer(mesh.geometry->indices, {0}, vk::IndexType::eUint32);

        buffer.bindVertexBuffers(0, {mesh.geometry->vertex_buffer}, {0});

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

        auto& textures = shader_resources->get_texture_manager();

        LOG(INFO) << "Initializing framebuffer attachments...";
        textures.create_dynamic_textures(shaderpack.dynamic_textures, passes_list);

        LOG(INFO) << "Building renderpasses and framebuffers...";
        renderpasses_by_pass = make_passes(shaderpack, textures, context);

        LOG(INFO) << "Building pipelines and compiling shaders...";
        pipelines_by_renderpass = make_pipelines(shaderpack, renderpasses_by_pass, context);

        LOG(INFO) << "Sorting materials...";
        material_passes_by_pipeline = extract_material_passes(shaderpack.materials);

        shader_resources->create_descriptor_sets(pipelines_by_renderpass);

        LOG(INFO) << "Loading complete";
    }

    void nova_renderer::deinit() {
        instance.release();
    }

    void nova_renderer::update_nova_ubos() {
        // For each renderable, update its model matrix
        // Currently that means GUI things get the GUI scaling matrix and chunks get a translation
        // Eventually entities will have a rotation too but that's not an immediate concern

        // Basic strategy here: We have a large buffer with all the model matrices, and each renderable has a handle to
        // its space in the buffer. The buffer is constantly mapped since it's updated super frequently. We make all our
        // updates, then flush the buffer

        // When the job system is in, updating the UBO matrices will be handled by the job system. For now we'll do it
        // here

        update_gui_model_matrices();

        for(const auto& mat : materials) {
            const auto& renderables_for_material = meshes->get_meshes_for_material(mat.name);

            for(const auto& renderable : renderables_for_material) {
                update_model_matrix(renderable);
            }
        }
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
        LOG(INFO) << "Beginning frame";
    }

    std::shared_ptr<render_context> nova_renderer::get_render_context() {
        return context;
    }

    std::shared_ptr<shader_resource_manager> nova_renderer::get_shader_resources() {
        return shader_resources;
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
                LOG(INFO) << "Material pass for material " << mat.name << " uses pipeline " << mat_pass.pipeline;
                if(ordered_material_passes.find(mat_pass.pipeline) == ordered_material_passes.end()) {
                    ordered_material_passes[mat_pass.pipeline] = std::vector<material_pass>{};
                }

                auto& vec = ordered_material_passes.at(mat_pass.pipeline);
                vec.push_back(mat_pass);
                ordered_material_passes[mat_pass.pipeline] = vec;
            }
        }

        return ordered_material_passes;
    }

    void nova_renderer::update_model_matrix(const render_object &renderable) {
        // This is not an OOP design and the Java gods are mad at me
        // Luckily I'm coding C++
        switch(renderable.type) {
            case geometry_type::gui:    // Updated separately
            case geometry_type::block:  // These don't change at runtime and it's fine
                break;
            default:
                LOG(WARNING) << "Model matrices are not supported for geometry type " << geometry_type::to_string(renderable.type);
        }
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
            const auto& device = context->device;

            std::vector<render_object> &gui_objects = meshes->get_meshes_for_material("gui");
            for(const auto &gui_obj : gui_objects) {
                update_gui_model_matrix(gui_obj, gui_model, device);
            }
        } catch(std::exception& e) {
            LOG(WARNING) << "Load some GUIs you fool";
            LOG(WARNING) << e.what() << std::endl;
        }
    }

    void nova_renderer::update_gui_model_matrix(const render_object& gui_obj, const glm::mat4& model_matrix, const vk::Device& device) {
        // Send the model matrix to the buffer
        // The per-model uniforms buffer is constantly mapped, so we can just grab the mapping from it
        auto& allocation = shader_resources->get_uniform_buffers().get_per_model_buffer()->get_allocation_info();
        memcpy(((uint8_t*)allocation.pMappedData) + gui_obj.per_model_buffer_range.offset, &model_matrix, gui_obj.per_model_buffer_range.range);
        LOG(INFO) << "Copied the GUI data to the buffer" << std::endl;
    }
}

