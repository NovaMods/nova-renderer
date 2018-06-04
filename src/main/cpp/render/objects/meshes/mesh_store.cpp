/*!
* \brief
*
* \author ddubois
* \date 27-Sep-16.
*/

#include <algorithm>
#include <easylogging++.h>
#include <regex>
#include <iomanip>
#include <utility>
#include "mesh_store.h"
#include "../../nova_renderer.h"
#include "vk_mesh.h"

namespace nova {
    mesh_store::mesh_store(std::shared_ptr<render_context> context, std::shared_ptr<shader_resource_manager> shader_resources)
            : context(context), shader_resources(shader_resources) {
        LOG(INFO) << "Does the mesh store have shader resources? " << (bool)shader_resources;
    }

    std::vector<render_object>& mesh_store::get_meshes_for_material(std::string material_name) {
        if(renderables_grouped_by_material.find(material_name) != renderables_grouped_by_material.end()) {
            return renderables_grouped_by_material.at(material_name);
        } else {
            return default_vector;
        }
    }

    void mesh_store::add_gui_buffers(const char *geo_type, mc_gui_geometry *command) {
        LOG(DEBUG) << "Adding GUI geometry " << command->texture_name << " for geometry type " << geo_type;
        std::string texture_name(command->texture_name);
        texture_name = std::regex_replace(texture_name, std::regex("^textures/"), "");
        texture_name = std::regex_replace(texture_name, std::regex(".png$"), "");
        texture_name = "minecraft:" + texture_name;
        const texture_manager::texture_location tex_location = shader_resources->get_texture_manager().get_texture_location(texture_name);
        glm::vec2 tex_size = tex_location.max - tex_location.min;

        mesh_definition cur_screen_buffer = {};
        //cur_screen_buffer.vertex_data.resize(28 * static_cast<unsigned long>(command->vertex_buffer_size / 9), 0);
        for (int i = 0; i + 8 < command->vertex_buffer_size; i += 9) {
            // position
            cur_screen_buffer.vertex_data.push_back(*reinterpret_cast<int*>(&command->vertex_buffer[i]));
            cur_screen_buffer.vertex_data.push_back(*reinterpret_cast<int*>(&command->vertex_buffer[i+1]));
            cur_screen_buffer.vertex_data.push_back(*reinterpret_cast<int*>(&command->vertex_buffer[i+2]));

            // UV0
            float u = command->vertex_buffer[i+3] * tex_size.x + tex_location.min.x;
            cur_screen_buffer.vertex_data.push_back(*reinterpret_cast<int*>(&u));
            float v = command->vertex_buffer[i+4] * tex_size.y + tex_location.min.y;
            cur_screen_buffer.vertex_data.push_back(*reinterpret_cast<int*>(&v));

            // MidTexCoord
            cur_screen_buffer.vertex_data.push_back(0);
            cur_screen_buffer.vertex_data.push_back(0);

            // VirtualTextureId
            cur_screen_buffer.vertex_data.push_back(0);

            // Color
            cur_screen_buffer.vertex_data.push_back(*reinterpret_cast<int*>(&command->vertex_buffer[i+5]));
            cur_screen_buffer.vertex_data.push_back(*reinterpret_cast<int*>(&command->vertex_buffer[i+6]));
            cur_screen_buffer.vertex_data.push_back(*reinterpret_cast<int*>(&command->vertex_buffer[i+7]));
            cur_screen_buffer.vertex_data.push_back(*reinterpret_cast<int*>(&command->vertex_buffer[i+8]));

            // UV1
            cur_screen_buffer.vertex_data.push_back(0);
            cur_screen_buffer.vertex_data.push_back(0);

            // Normal
            cur_screen_buffer.vertex_data.push_back(0);
            cur_screen_buffer.vertex_data.push_back(0);
            cur_screen_buffer.vertex_data.push_back(0);

            // Tangent
            cur_screen_buffer.vertex_data.push_back(0);
            cur_screen_buffer.vertex_data.push_back(0);
            cur_screen_buffer.vertex_data.push_back(0);

            // McEntityId
            cur_screen_buffer.vertex_data.push_back(0);
            cur_screen_buffer.vertex_data.push_back(0);
            cur_screen_buffer.vertex_data.push_back(0);
            cur_screen_buffer.vertex_data.push_back(0);
        }
        cur_screen_buffer.indices.resize(static_cast<unsigned long>(command->index_buffer_size), 0);
        for(int i = 0; i < command->index_buffer_size; i++) {
            cur_screen_buffer.indices[i] = (unsigned int)command->index_buffer[i];
        }

        cur_screen_buffer.vertex_format = format::POS_UV_COLOR;

        render_object gui;
        gui.model_matrix_descriptor = shader_resources->create_model_matrix_descriptor();
        gui.per_model_buffer_range = shader_resources->get_uniform_buffers().get_per_model_buffer()->allocate_space(sizeof(glm::mat4));
        gui.upload_model_matrix(context->device);
        gui.geometry = std::make_shared<vk_mesh>(cur_screen_buffer, context);
        gui.type = geometry_type::gui;

        LOG(INFO) << "Game render object " << gui.id << " model matrix descriptor " << (VkDescriptorSet)gui.model_matrix_descriptor;

        if(renderables_grouped_by_material.find(geo_type) == renderables_grouped_by_material.end()) {
            renderables_grouped_by_material[geo_type] = std::vector<render_object>{};
        }
        renderables_grouped_by_material.at(geo_type).push_back(gui);
    }

    void mesh_store::remove_gui_render_objects() {
        remove_render_objects([](auto& render_obj) {return render_obj.type == geometry_type::gui || render_obj.type == geometry_type::text;});
    }

    void mesh_store::remove_render_objects(std::function<bool(render_object&)> filter) {
        geometry_to_remove.push(filter);
    }

    void mesh_store::remove_old_geometry() {
        while(!geometry_to_remove.empty()) {
            auto &filter = geometry_to_remove.front();

            auto per_model_buffer = shader_resources->get_uniform_buffers().get_per_model_buffer();
            for (auto &group : renderables_grouped_by_material) {
                auto removed_elements = std::remove_if(group.second.begin(), group.second.end(), filter);

                if (removed_elements != group.second.end()) {
                    // Free the allocations of each render object
                    for (auto it = removed_elements; it != group.second.end(); ++it) {
                        LOG(INFO) << "Removing render object " << (*it).id;
                        per_model_buffer->free_allocation((*it).per_model_buffer_range);
                        shader_resources->free_descriptor((*it).model_matrix_descriptor);
                    }
                }

                group.second.erase(removed_elements, group.second.end());
            }

            geometry_to_remove.pop();
        }
    }

    void mesh_store::upload_new_geometry() {
        LOG(TRACE) << "Uploading " << geometry_to_upload.size() << " new objects";
        geometry_to_upload_lock.lock();
        while(!geometry_to_upload.empty()) {
            const auto& entry = geometry_to_upload.front();
            const auto& def = std::get<1>(entry);

            render_object obj = {};
            obj.model_matrix_descriptor = shader_resources->create_model_matrix_descriptor();
            obj.per_model_buffer_range = shader_resources->get_uniform_buffers().get_per_model_buffer()->allocate_space(sizeof(glm::mat4));
            obj.upload_model_matrix(context->device);
            obj.geometry = std::make_shared<vk_mesh>(def, context);
            obj.type = geometry_type::block;
            obj.parent_id = def.id;
            obj.position = def.position;
            obj.bounding_box.center = def.position;
            obj.bounding_box.center.y = 128;
            obj.bounding_box.extents = {16, 128, 16};   // TODO: Make these values come from Minecraft

            LOG(INFO) << "Game render object " << obj.id << " model matrix descriptor " << (VkDescriptorSet)obj.model_matrix_descriptor;

            const std::string& shader_name = std::get<0>(entry);
            if(renderables_grouped_by_material.find(shader_name) == renderables_grouped_by_material.end()) {
                renderables_grouped_by_material[shader_name] = std::vector<render_object>{};
            }
            renderables_grouped_by_material.at(shader_name).push_back(obj);

            geometry_to_upload.pop();
        }
        geometry_to_upload_lock.unlock();
    }

    void mesh_store::add_chunk_render_object(std::string filter_name, mc_chunk_render_object &chunk) {
        mesh_definition def = {};
        auto& vertex_data = def.vertex_data;

        for(int i = 0; i < chunk.vertex_buffer_size; i++) {
            vertex_data.push_back(chunk.vertex_data[i]);

            if(i % 7 == 6) {
                // Add 0s for the normals and tangets since we don't compute those yet
                vertex_data.push_back(0);
                vertex_data.push_back(0);
                vertex_data.push_back(0);
                vertex_data.push_back(0);
                vertex_data.push_back(0);
                vertex_data.push_back(0);
            }
        }

        for(int i = 0; i < chunk.index_buffer_size; i++) {
            def.indices.push_back(chunk.indices[i]);
        }

        def.vertex_format = format::all_values()[chunk.format];
        def.position = {chunk.x, chunk.y, chunk.z};
        def.id = chunk.id;

        geometry_to_upload_lock.lock();
        geometry_to_upload.emplace(filter_name, def);
        geometry_to_upload_lock.unlock();
    }

    void mesh_store::remove_render_objects_with_parent(long parent_id) {
        remove_render_objects([&](render_object& obj) { return obj.parent_id == parent_id; });
    }

    void mesh_store::add_fullscreen_quad_for_material(const std::string &material_name) {
        if(has_fullscreen_quad.find(material_name) != has_fullscreen_quad.end()) {
            if(has_fullscreen_quad.at(material_name)) {
                return;
            }
        }

        union f2i {
            float f;
            int i;

            f2i(float fl) {
                f = fl;
            }

            f2i(int in) {
                i = in;
            }
        };

        mesh_definition quad;
        quad.vertex_data = {
                f2i(0.0f).i, f2i(2.0f).i, f2i(0.0f).i,
                f2i(0.0f).i, f2i(2.0f).i,
                f2i(0.0f).i, f2i(0.0f).i,
                0,
                f2i(1.0f).i, f2i(1.0f).i, f2i(1.0f).i,
                f2i(0.0f).i, f2i(0.0f).i,
                f2i(0.0f).i, f2i(0.0f).i, f2i(0.0f).i,
                f2i(0.0f).i, f2i(0.0f).i, f2i(0.0f).i,
                f2i(0.0f).i, f2i(0.0f).i, f2i(0.0f).i, f2i(0.0f).i,


                f2i(2.0f).i, f2i(0.0f).i, f2i(0.0f).i,
                f2i(2.0f).i, f2i(0.0f).i,
                f2i(0.0f).i, f2i(0.0f).i,
                0,
                f2i(1.0f).i, f2i(1.0f).i, f2i(1.0f).i,
                f2i(0.0f).i, f2i(0.0f).i,
                f2i(0.0f).i, f2i(0.0f).i, f2i(0.0f).i,
                f2i(0.0f).i, f2i(0.0f).i, f2i(0.0f).i,
                f2i(0.0f).i, f2i(0.0f).i, f2i(0.0f).i, f2i(0.0f).i,


                f2i(0.0f).i, f2i(0.0f).i, f2i(0.0f).i,
                f2i(0.0f).i, f2i(0.0f).i,
                f2i(0.0f).i, f2i(0.0f).i,
                0,
                f2i(1.0f).i, f2i(1.0f).i, f2i(1.0f).i,
                f2i(0.0f).i, f2i(0.0f).i,
                f2i(0.0f).i, f2i(0.0f).i, f2i(0.0f).i,
                f2i(0.0f).i, f2i(0.0f).i, f2i(0.0f).i,
                f2i(0.0f).i, f2i(0.0f).i, f2i(0.0f).i, f2i(0.0f).i,
        };

        quad.indices = {0, 1, 2};
        quad.vertex_format = format::POS_COLOR_UV_LIGHTMAPUV_NORMAL_TANGENT;
        quad.position = glm::vec3(0);
        quad.id = 0;

        geometry_to_upload_lock.lock();
        geometry_to_upload.emplace(material_name, quad);
        geometry_to_upload_lock.unlock();

        has_fullscreen_quad[material_name] = true;
    }
}
