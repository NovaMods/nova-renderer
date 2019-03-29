#include <glm/gtc/matrix_transform.hpp>
#include "../../render_objects/renderables.hpp"
#include "../../render_objects/uniform_structs.hpp"
#include "fmt/format.h"
#include "vulkan_render_engine.hpp"

namespace nova::renderer {
    void vulkan_render_engine::create_builtin_uniform_buffers() {
        {
            // Future Work: Get this from a per-scene configuration
            const uint32_t total_object_estimate = 10000;
            VkBufferCreateInfo model_matrix_create_info = {};
            model_matrix_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            model_matrix_create_info.size = total_object_estimate * sizeof(glm::mat4);
            model_matrix_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            model_matrix_create_info.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;

            VmaAllocationCreateInfo alloc_create_info = {};
            alloc_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
            alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

            vmaCreateBuffer(vma_allocator,
                            &model_matrix_create_info,
                            &alloc_create_info,
                            &model_matrix_buffer.buffer,
                            &model_matrix_buffer.allocation,
                            &model_matrix_buffer.alloc_info);
        }

        {
            VkBufferCreateInfo per_frame_data_create_info = {};
            per_frame_data_create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
            per_frame_data_create_info.size = sizeof(per_frame_uniforms);
            per_frame_data_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            per_frame_data_create_info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

            VmaAllocationCreateInfo alloc_create_info = {};
            alloc_create_info.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
            alloc_create_info.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

            vmaCreateBuffer(vma_allocator,
                &per_frame_data_create_info,
                &alloc_create_info,
                &per_frame_data_buffer.buffer,
                &per_frame_data_buffer.allocation,
                &per_frame_data_buffer.alloc_info);
        }
    }

    result<renderable_id_t> vulkan_render_engine::add_renderable(const static_mesh_renderable_data& data) {
        return get_material_passes_for_renderable(data).flatMap([&](const std::vector<const material_pass*>& passes) {
            return get_mesh_for_renderable(data).flatMap(
                std::bind(&vulkan_render_engine::register_renderable, this, data, std::placeholders::_1, passes));
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
        } else {
            return result<std::vector<const material_pass*>>(std::move(passes));
        }
    }

    result<const vk_mesh*> vulkan_render_engine::get_mesh_for_renderable(const static_mesh_renderable_data& data) {
        if(meshes.find(data.mesh) == meshes.end()) {
            return result<const vk_mesh*>(nova_error(fmt::format(fmt("Could not find mesh with id {:d}"), data.mesh)));
        }

        return result<const vk_mesh*>(&meshes.at(data.mesh));
    }

    result<renderable_id_t> vulkan_render_engine::register_renderable(const static_mesh_renderable_data& data,
                                                                      const vk_mesh* mesh,
                                                                      const std::vector<const material_pass*>& passes) {
        renderable_metadata meta = {};
        meta.passes.reserve(passes.size());
        for(const material_pass* m : passes) {
            meta.passes.push_back(m->name);
        }

        // TODO: UBO things!
        // If the renderable is static, allocate its model matrix ubo slot from the static objects UBO
        // If the renderable is dynamic, allocate its model matrix UBO from the dynamic objects ubo

        // Set all the renderable's data
        vk_static_mesh_renderable renderable = {};

        // Generate the renderable ID and store the renderable
        renderable_id_t id = next_renderable_id.fetch_add(1);
        renderable.id = id;
        metadata_for_renderables[id] = meta;

        // Set up model matrix
        renderable.model_matrix = glm::translate(renderable.model_matrix, data.initial_position);
        renderable.model_matrix = glm::rotate(renderable.model_matrix, data.initial_rotation.x, {1, 0, 0});
        renderable.model_matrix = glm::rotate(renderable.model_matrix, data.initial_rotation.y, {0, 1, 0});
        renderable.model_matrix = glm::rotate(renderable.model_matrix, data.initial_rotation.x, {0, 0, 1});
        renderable.model_matrix = glm::scale(renderable.model_matrix, data.initial_scale);

        // Find the materials basses that this renderable belongs to, put it in the appropriate maps
        for(const material_pass* pass : passes) {
            renderables_by_material[pass->name].static_meshes[mesh->id].push_back(renderable);
        }

        return result<renderable_id_t>(std::move(id));
    }

    void vulkan_render_engine::set_renderable_visibility(const renderable_id_t id, const bool is_visible) {
        if(metadata_for_renderables.find(id) != metadata_for_renderables.end()) {
            const renderable_metadata& meta = metadata_for_renderables.at(id);
            for(const std::string& pass_name : meta.passes) {
                if(renderables_by_material.find(pass_name) != renderables_by_material.end()) {
                    std::unordered_map<mesh_id_t, std::vector<vk_static_mesh_renderable>>&
                        renderables_for_mesh = renderables_by_material[pass_name].static_meshes;
                    for(auto& [mesh_id, renderables] : renderables_for_mesh) {
                        (void) mesh_id;
                        for(vk_static_mesh_renderable& renderable : renderables) {
                            if(renderable.id == meta.id) {
                                renderable.is_visible = is_visible;
                            }
                        }
                    }
                }
            }
        }

        // TODO: Try other types of renderables
    }

    void vulkan_render_engine::delete_renderable(renderable_id_t id) { static_cast<void>(id); }
} // namespace nova::renderer