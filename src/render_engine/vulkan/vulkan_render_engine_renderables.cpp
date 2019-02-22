#include "../../render_objects/renderables.hpp"
#include "fmt/format.h"
#include "vulkan_render_engine.hpp"

namespace nova::renderer {
    void vulkan_render_engine::create_builtin_uniform_buffers() {
        // TODO: move this to the settings
        const uint32_t static_object_estimate = 5000;
        VkBufferCreateInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        info.size = static_object_estimate * sizeof(glm::mat4);
        info.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        const uint32_t alignment = static_cast<uint32_t>(gpu.props.limits.minUniformBufferOffsetAlignment);

        static_model_matrix_buffer = std::make_unique<auto_buffer>("NovaStaticModelUBO", vma_allocator, info, alignment, false);
    }

    result<renderable_id_t> vulkan_render_engine::add_renderable(const static_mesh_renderable_data& data) {
        return get_material_passes_for_renderable(data).map([&](const std::vector<const material_pass*>& passes) {
            return get_mesh_for_renderable(data).map(
                std::bind(&vulkan_render_engine::register_renderable, this, std::placeholders::_1, passes));
        });
    }

    result<std::vector<const material_pass*>> vulkan_render_engine::get_material_passes_for_renderable(
        const static_mesh_renderable_data& data) {
        std::vector<const material_pass*> passes;

        for(const auto& [pipeline_name, materials] : material_passes_by_pipeline) {
            static_cast<void>(pipeline_name);

            for(const material_pass& pass : materials) {
                if(pass.material_name == data.material_name) {
                    passes.push_back(&pass);
                }
            }
        }

        if(passes.empty()) {
            return result<std::vector<const material_pass*>>(
                nova_error(fmt::format(fmt("Could not find material {:s}"), data.material_name)));
        }
        else {
            return result<std::vector<const material_pass*>>(std::move(passes));
        }
    }

    result<const vk_mesh*> vulkan_render_engine::get_mesh_for_renderable(const static_mesh_renderable_data& data) {
        if(meshes.find(data.mesh) == meshes.end()) {
            return result<const vk_mesh*>(nova_error(fmt::format(fmt("Could not find mesh with id {:d}"), data.mesh)));
        }

        return result<const vk_mesh*>(&meshes.at(data.mesh));
    }

    result<renderable_id_t> vulkan_render_engine::register_renderable(const vk_mesh* mesh,
                                                                      const std::vector<const material_pass*>& passes) {
        renderable_metadata meta = {};
        meta.passes.reserve(meshes.size());
        for(const material_pass* m : passes) {
            meta.passes.push_back(m->name);
        }

        meta.buffer = mesh->memory->block->get_buffer();

        // Set all the renderable's data
        vk_static_mesh_renderable renderable = {};
        renderable.draw_cmd = &mesh->draw_cmd;

        // Generate the renderable ID and store the renderable
        renderable_id_t id = RENDERABLE_ID.fetch_add(1);
        renderable.id = id;
        static_mesh_renderables[id] = renderable;
        metadata_for_renderables[id] = meta;

        // Find the materials basses that this renderable belongs to, put it in the appropriate maps
        for(const material_pass* pass : passes) {
            renderables_by_material[pass->name][mesh->memory->block->get_buffer()].push_back(id);
        }

        return result<renderable_id_t>(std::move(id));
    }

    void vulkan_render_engine::set_renderable_visibility(const renderable_id_t id, const bool is_visible) {
        if(static_mesh_renderables.find(id) != static_mesh_renderables.end()) {
            static_mesh_renderables[id].is_visible = is_visible;
        }

        // TODO: Try other types of renderables
    }

    void vulkan_render_engine::delete_renderable(renderable_id_t id) { static_cast<void>(id); }
} // namespace nova::renderer