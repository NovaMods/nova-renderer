//
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
#include <nova/profiler.h>

INITIALIZE_EASYLOGGINGPP

// Thank you Visual Studio, this was a totally good thing you did yup
#ifdef max
#undef max
#endif

namespace nova {
    std::unique_ptr<nova_renderer> nova_renderer::instance;
    std::shared_ptr<settings> nova_renderer::render_settings;

    nova_renderer::nova_renderer() {
        NOVA_PROFILER_SCOPE;
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
        swapchain = std::make_shared<swapchain_manager>(3, context, game_window->get_size());
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
    }

    void nova_renderer::render_frame() {
        NOVA_PROFILER_SCOPE;
        begin_frame();

        player_camera.recalculate_frustum();

        swapchain->aqcuire_next_swapchain_image(swapchain_image_acquire_semaphore);

        context->device.resetFences({render_done_fence});

        auto main_command_buffer = context->command_buffer_pool->get_command_buffer(0);
        main_command_buffer.buffer.reset(vk::CommandBufferResetFlagBits::eReleaseResources);
        LOG(TRACE) << "Rendering with command buffer " << (VkCommandBuffer) main_command_buffer.buffer;

        vk::CommandBufferBeginInfo cmd_buf_begin_info = {};
        main_command_buffer.buffer.begin(cmd_buf_begin_info);

        player_camera.recalculate_frustum();

        update_nova_ubos();

        LOG(DEBUG) << "We have " << passes_list.size() << " passes to render";
        for (const auto &pass : passes_list) {
            execute_pass(pass, main_command_buffer.buffer);
        }

        main_command_buffer.buffer.end();

        // Submit the command buffer
        vk::Semaphore wait_semaphores[] = {swapchain_image_acquire_semaphore};
        vk::PipelineStageFlags wait_stages[] = {vk::PipelineStageFlagBits::eColorAttachmentOutput};
        std::vector<vk::Semaphore> signal_semaphores = {render_finished_semaphore};

        vk::SubmitInfo submit_info = vk::SubmitInfo()
                .setCommandBufferCount(1)
                .setPCommandBuffers(&main_command_buffer.buffer)
                .setWaitSemaphoreCount(1)
                .setPWaitSemaphores(wait_semaphores)
                .setPWaitDstStageMask(wait_stages)
                .setSignalSemaphoreCount(static_cast<uint32_t>(signal_semaphores.size()))
                .setPSignalSemaphores(signal_semaphores.data());

        context->graphics_queue.submit(1, &submit_info, render_done_fence);

        swapchain->present_current_image(signal_semaphores);

        game_window->end_frame();

        auto fence_wait_result = context->device.waitForFences({render_done_fence}, true, std::numeric_limits<uint64_t>::max());
        if (fence_wait_result == vk::Result::eSuccess) {
            // Process geometry updates
            meshes->remove_gui_render_objects();
            meshes->remove_old_geometry();
            meshes->upload_new_geometry();

            LOG(TRACE) << "About to reset the main command buffer";
            context->command_buffer_pool->free(main_command_buffer);
            LOG(TRACE) << "Freed it";

        } else {
            LOG(WARNING) << "Could not wait for render done fence, " << vk::to_string(fence_wait_result);
        }

        end_frame();
    }

    void nova_renderer::execute_pass(const render_pass &pass, vk::CommandBuffer& buffer) {
        NOVA_PROFILER_SCOPE;
        if(renderpasses_by_pass.find(pass.name) == renderpasses_by_pass.end()) {
            LOG(ERROR) << "No renderpass defined for pass " << pass.name << ". Skipping this pass";
            return;
        }

        if(pipelines_by_pass.find(pass.name) == pipelines_by_pass.end()) {
            LOG(WARNING) << "No pipelines attached to pass " << pass.name << ". Skipping this pass";
            return;
        }

        LOG(INFO) << "Beginning pass " << pass.name;

        if(pass.texture_inputs) {
            // Transition anything that's not in shader read optimal to shader read optimal
            const std::vector<std::string>& shader_textures = pass.texture_inputs.value().bound_textures;
            for(const auto& tex_name : shader_textures) {
                auto& tex = shader_resources->get_texture_manager().get_texture(tex_name);
                if(tex.get_layout() != vk::ImageLayout::eShaderReadOnlyOptimal) {
                    transfer_image_format(buffer, tex.get_vk_image(), tex.get_layout(), vk::ImageLayout::eShaderReadOnlyOptimal);
                    tex.set_layout(vk::ImageLayout::eShaderReadOnlyOptimal);
                }
            }
        }

        const auto& renderpass_for_pass = renderpasses_by_pass.at(pass.name);

        vk::RenderPassBeginInfo begin_pass = vk::RenderPassBeginInfo()
                .setRenderPass(renderpass_for_pass.renderpass)
                .setRenderArea({{0, 0}, renderpass_for_pass.framebuffer_size});

        vk::Framebuffer framebuffer;
        if(renderpass_for_pass.texture_outputs[0].name == "Backbuffer") {
            framebuffer = swapchain->get_current_framebuffer();

        } else {
            framebuffer = renderpass_for_pass.frameBuffer;
        }

        uint32_t depth_idx = 0xffffffff;
        std::vector<texture_attachment> attachments = pass.texture_outputs.value_or(std::vector<texture_attachment>());
        if(pass.depth_texture) {
            depth_idx = static_cast<uint32_t>(attachments.size());
            attachments.push_back(pass.depth_texture.value());
        }

        vk::ClearColorValue clear_color;
        clear_color.setFloat32({0.689f, 0.82f, 0.922f, 0.0f});

        std::vector<vk::ClearValue> clear_values;
        clear_values.resize(attachments.size());
        // Clear any textures we need to clear
        for(size_t i = 0; i < attachments.size(); i++) {
            if(i == depth_idx) {
                clear_values[i] = vk::ClearValue().setDepthStencil({1, 0xFFFFFFFF});

            } else {
                clear_values[i] = vk::ClearValue().setColor(clear_color);
            }
        }

        begin_pass.setClearValueCount(static_cast<uint32_t>(clear_values.size()))
                .setPClearValues(clear_values.data());

        begin_pass.setFramebuffer(framebuffer);
        LOG(TRACE) << "Using framebuffer " << (VkFramebuffer)framebuffer;

        buffer.beginRenderPass(&begin_pass, vk::SubpassContents::eInline);

        auto& pipelines_for_pass = pipelines_by_renderpass.at(pass.name);
        LOG(DEBUG) << "Processing data in " << pipelines_for_pass.size() << " pipelines";

        for(auto& nova_pipeline : pipelines_for_pass) {
            render_pipeline(nova_pipeline, buffer);
        }

        buffer.endRenderPass();

        // Set the layouts on all the textures to the layouts tha the renderpass will use them in
        if(pass.texture_outputs) {
            const auto& texture_outputs = pass.texture_outputs.value();
            const auto& textures = shader_resources->get_texture_manager();
            for(const auto& attachment : texture_outputs) {
                if(textures.is_texture_known(attachment.name)) {
                    auto &tex = shader_resources->get_texture_manager().get_texture(attachment.name);
                    tex.set_layout(vk::ImageLayout::eColorAttachmentOptimal);
                }
            }
        }
    }

    void nova_renderer::render_pipeline(pipeline_object &pipeline_data, vk::CommandBuffer& buffer) {
        NOVA_PROFILER_SCOPE;
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

    void nova_renderer::render_all_for_material_pass(const material_pass& mat, vk::CommandBuffer &buffer, pipeline_object &pipeline) {
        NOVA_PROFILER_SCOPE;
        const auto& meshes_for_mat = meshes->get_meshes_for_material(mat.material_name);
        if(meshes_for_mat.empty()) {
            LOG(INFO) << "No meshes available for material " << mat.material_name;
            return;
        }

        LOG(INFO) << "Beginning material " << mat.material_name;

        auto per_model_buffer_binding = std::string{};

        const auto& pipeline_layout = pipeline.layout;

        // Find the per-model UBO
        for(const auto& binding : mat.bindings) {
            const auto &descriptor_name = binding.first;
            const auto &resource_name = binding.second;
            if(resource_name == "NovaPerModelUBO") {
                // Bind dis later
                per_model_buffer_binding = descriptor_name;
                break;
            }
        }

        for(const auto& resource : pipeline.resource_bindings) {
            std::stringstream ss;
            ss << "Shader descriptor {name=" << resource.first << " set=" << resource.second.set << " binding=" << resource.second.binding << ") ";
            if(mat.bindings.find(resource.first) != mat.bindings.end()) {
                ss << "has resource " << mat.bindings.at(resource.first) << " bound";

            } else {
                ss << " has nothing bound!";
            }

            LOG(TRACE) << ss.str();
        }

        std::stringstream ss;
        ss << "Binding descriptors ";
        for(const auto& desc : mat.descriptor_sets) {
            ss << (VkDescriptorSet)desc << ", ";
        }
        LOG(TRACE) << ss.str();
        buffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline_layout, 0, mat.descriptor_sets, {});

        LOG(INFO) << "Rendering " << meshes_for_mat.size() << " things";
        for(const auto& mesh : meshes_for_mat) {
            render_mesh(mesh, buffer, pipeline, per_model_buffer_binding);
        }
    }

    void nova_renderer::render_mesh(const render_object &mesh, vk::CommandBuffer &buffer, pipeline_object &pipeline_data, std::string per_model_buffer_resource) {
        NOVA_PROFILER_SCOPE;
        const auto& descriptor = pipeline_data.resource_bindings[per_model_buffer_resource];
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
        NOVA_PROFILER_SCOPE;
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
        textures.create_dynamic_textures(shaderpack.dynamic_textures, passes_list, swapchain);

        LOG(INFO) << "Building renderpasses and framebuffe rs...";
        renderpasses_by_pass = make_passes(shaderpack, textures, context, swapchain);

        LOG(INFO) << "Building pipelines and compiling shaders...";
        pipelines_by_renderpass = make_pipelines(shaderpack, renderpasses_by_pass, context);

        LOG(INFO) << "Sorting materials...";
        material_passes_by_pipeline = extract_material_passes(shaderpack.materials);

        shader_resources->create_descriptor_sets(pipelines_by_renderpass, material_passes_by_pipeline);

        // Look for any materials that use a fullscreen pass and insert renderables for them
        insert_special_geometry(material_passes_by_pipeline);

        LOG(INFO) << "Loading complete";
    }

    void nova_renderer::deinit() {
        instance.release();
    }

    void nova_renderer::update_nova_ubos() {
        NOVA_PROFILER_SCOPE;
        // For each renderable, update its model matrix
        // Currently that means GUI things get the GUI scaling matrix and chunks get a translation
        // Eventually entities will have a rotation too but that's not an immediate concern

        // Basic strategy here: We have a large buffer with all the model matrices, and each renderable has a handle to
        // its space in the buffer. The buffer is constantly mapped since it's updated super frequently. We make all our
        // updates, then flush the buffer

        // When the job system is in, updating the UBO matrices will be handled by the job system. For now we'll do it
        // here

        update_per_frame_ubo();
        update_gui_model_matrices();

        for(const auto& mat : materials) {
            const auto& renderables_for_material = meshes->get_meshes_for_material(mat.name);

            for(const auto& renderable : renderables_for_material) {
                update_model_matrix(renderable);
            }
        }
    }

    void nova_renderer::update_per_frame_ubo() {
        NOVA_PROFILER_SCOPE;
        LOG(DEBUG) << "Updating the per-frame UBO";

        LOG(DEBUG) << "Camera position: " << player_camera.position << " rotation: " << player_camera.rotation;

        auto per_frame_data = per_frame_uniforms{};
        per_frame_data.gbufferProjection = player_camera.get_projection_matrix();
        per_frame_data.gbufferModelView = player_camera.get_view_matrix();

        LOG(TRACE) << "View matrix: " << per_frame_data.gbufferModelView;
        LOG(TRACE) << "Projection matrix: " << per_frame_data.gbufferProjection;
        LOG(TRACE) << "Set " << sizeof(per_frame_uniforms) << " bytes of uniform data";

        uniform_buffer& per_frame_ubo = shader_resources->get_uniform_buffers().get_buffer("NovaPerFrameUBO");
        per_frame_ubo.set_data(&per_frame_data, sizeof(per_frame_uniforms));
    }

    camera &nova_renderer::get_player_camera() {
        return player_camera;
    }

    std::vector<material>& nova_renderer::get_materials() {
        return materials;
    }

    void nova_renderer::end_frame() {
        NOVA_PROFILER_FLUSH_TO_FILE("profiler_data.txt");
        LOG(INFO) << "Frame done";
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
        NOVA_PROFILER_SCOPE;
        auto passes_dependency_order = order_passes(passes);
        auto ordered_passes = std::vector<render_pass>{};

        for(const auto& pass_name : passes_dependency_order) {
            ordered_passes.push_back(passes[pass_name]);
        }

        return ordered_passes;
    }

    std::unordered_map<std::string, std::vector<material_pass>> nova_renderer::extract_material_passes(const std::vector<material>& materials) {
        NOVA_PROFILER_SCOPE;
        auto ordered_material_passes = std::unordered_map<std::string, std::vector<material_pass>>{};

        for(const auto& mat : materials) {
            for(const auto& mat_pass : mat.passes) {
                LOG(TRACE) << "Material pass for material " << mat.name << " uses pipeline " << mat_pass.pipeline;
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
        NOVA_PROFILER_SCOPE;
        // This is not an OOP design and the Java gods are mad at me
        // Luckily I'm coding C++
        switch(renderable.type) {
            case geometry_type::gui:    // Updated separately
            case geometry_type::text:   // Updated separately
            case geometry_type::block:  // These don't change at runtime and it's fine
                break;
            default:
                LOG(WARNING) << "Model matrices are not supported for geometry type " << geometry_type::to_string(renderable.type);
        }
    }

    void nova_renderer::update_gui_model_matrices() {
        NOVA_PROFILER_SCOPE;
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
            std::vector<render_object> &gui_text_objexts = meshes->get_meshes_for_material("gui_text");
            for(const auto &gui_obj : gui_text_objexts) {
                update_gui_model_matrix(gui_obj, gui_model, device);
            }
        } catch(std::exception& e) {
            LOG(WARNING) << "Load some GUIs you fool";
            LOG(WARNING) << e.what() << std::endl;
        }
    }

    void nova_renderer::update_gui_model_matrix(const render_object& gui_obj, const glm::mat4& model_matrix, const vk::Device& device) {
        NOVA_PROFILER_SCOPE;
        // Send the model matrix to the buffer
        // The per-model uniforms buffer is constantly mapped, so we can just grab the mapping from it
        auto& allocation = shader_resources->get_uniform_buffers().get_per_model_buffer()->get_allocation_info();
        memcpy(((uint8_t*)allocation.pMappedData) + gui_obj.per_model_buffer_range.offset, &model_matrix, gui_obj.per_model_buffer_range.range);
        LOG(INFO) << "Copied the GUI data to the buffer" << std::endl;
    }

    void nova_renderer::insert_special_geometry(const std::unordered_map<std::string, std::vector<material_pass>> &material_passes_by_pipeline) {
        NOVA_PROFILER_SCOPE;
        // If the material pass has the fullscreen geometry in its filter, insert it into the mesh store
        for(const auto& passes_for_pipeline : material_passes_by_pipeline) {
            for(const material_pass& pass : passes_for_pipeline.second) {
                const auto& mat = std::find_if(materials.begin(), materials.end(), [&](const material& mat) {return mat.name == pass.material_name;});
                if((*mat).geometry_filter.find("geometry_type::fullscreen_quad") != std::string::npos) {
                    LOG(DEBUG) << "Inserting a fullscreen quad for material " << (*mat).name;
                    meshes->add_fullscreen_quad_for_material((*mat).name);
                }
            }
        }
    }
}

