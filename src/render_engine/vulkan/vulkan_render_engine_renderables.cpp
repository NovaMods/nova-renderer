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

    result<renderable_id_t> vulkan_render_engine::add_renderable(const static_mesh_renderer_data& data) {
        return [&]() -> result<std::vector<const material_pass*>> {
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
        }()
                .map([&](const std::vector<const material_pass*>& passes) {
                    static_cast<void>(passes);
                    auto mesh_result =
                        [&]() {
                            if(meshes.find(data.mesh) == meshes.end()) {
                                return result<const vk_mesh*>(
                                    nova_error(fmt::format(fmt("Could not find mesh with id {:d}"), data.mesh)));
                            }

                            return result<const vk_mesh*>(&meshes.at(data.mesh));
                        }()
                            .map([](const vk_mesh* mesh) {
                                static_cast<void>(mesh);

                                // vk_static_mesh_renderable renderable = {};
                                // renderable.draw_cmd = &mesh->draw_cmd;

                                return static_cast<renderable_id_t>(3);
                            });

                    return static_cast<renderable_id_t>(3);
                });
    }

    void vulkan_render_engine::set_renderable_visibility(renderable_id_t id, bool is_visible) {
        static_cast<void>(id);
        static_cast<void>(is_visible);
    }

    void vulkan_render_engine::delete_renderable(renderable_id_t id) { static_cast<void>(id); }
} // namespace nova::renderer