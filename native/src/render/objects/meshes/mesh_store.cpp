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
#include <glm/gtc/matrix_transform.hpp>

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
            nova_vertex new_vertex = {};

            // position
            new_vertex.position.x = command->vertex_buffer[i];
            new_vertex.position.y = command->vertex_buffer[i+1];
            new_vertex.position.z = command->vertex_buffer[i+2];

            // UV0
            float u = command->vertex_buffer[i+3] * tex_size.x + tex_location.min.x;
            new_vertex.uv0.x = u;
            float v = command->vertex_buffer[i+4] * tex_size.y + tex_location.min.y;
            new_vertex.uv0.y = v;

            // Color
            new_vertex.color.r = command->vertex_buffer[i+5];
            new_vertex.color.g = command->vertex_buffer[i+6];
            new_vertex.color.b = command->vertex_buffer[i+7];
            new_vertex.color.a = command->vertex_buffer[i+8];

            cur_screen_buffer.vertex_data.push_back(new_vertex);
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

        LOG(INFO) << "Adding GUI render object " << gui.id << " model matrix descriptor " << (VkDescriptorSet)gui.model_matrix_descriptor;

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

            glm::mat4 model_matrix(1.0);
            model_matrix = glm::translate(model_matrix, def.position);
            memcpy(((uint8_t*)shader_resources->get_uniform_buffers().get_per_model_buffer()->get_allocation_info().pMappedData)
                   + obj.per_model_buffer_range.offset, &model_matrix, obj.per_model_buffer_range.range);

            obj.upload_model_matrix(context->device);

            obj.geometry = std::make_shared<vk_mesh>(def, context);
            obj.type = geometry_type::block;
            obj.parent_id = def.id;
            obj.position = def.position;

            obj.bounding_box.center = {def.position.x + 8, def.position.y + 8, def.position.z + 8};
            obj.bounding_box.extents = {16, 16, 16};   // TODO: Make these values come from Minecraft
            // obj.needs_deletion = false; // TODO: needed for anything?

            LOG(INFO) << "Adding render object " << obj.id << " model matrix descriptor " << (VkDescriptorSet)obj.model_matrix_descriptor;

            const std::string& material_name = std::get<0>(entry);
            if(renderables_grouped_by_material.find(material_name) == renderables_grouped_by_material.end()) {
                renderables_grouped_by_material[material_name] = std::vector<render_object>{};
                LOG(TRACE) << "Initialized storage for render objects with material " << material_name;
            }
            renderables_grouped_by_material.at(material_name).push_back(obj);
            LOG(TRACE) << "Added object to list of things with material " << material_name;

            geometry_to_upload.pop();
            LOG(TRACE) << "Removed the object from the list of geometry to upload";
        }
        geometry_to_upload_lock.unlock();
    }

    void mesh_store::remove_chunk_render_object(std::string filter_name, mc_chunk_render_object &chunk) {
        try {
            if (renderables_grouped_by_material.find(filter_name) != renderables_grouped_by_material.end()) {
                auto &group = renderables_grouped_by_material.at(filter_name);
                for (auto& obj : group) {
                    bool del = (static_cast<int>(obj.position.x) == static_cast<int>(chunk.x)) &&
                               (static_cast<int>(obj.position.y) == static_cast<int>(chunk.y)) &&
                               (static_cast<int>(obj.position.z) == static_cast<int>(chunk.z));
                    if (del) {
                        remove_render_objects(obj);
                    }
                }
            }
        } catch (std::exception &e) {
            LOG(ERROR) << "REMOVING CHUNK ERROR: " << e.what();
        }
    }

    void mesh_store::add_chunk_render_object(std::string filter_name, mc_chunk_render_object &chunk) {
        mesh_definition def = {};
        auto& vertex_data = def.vertex_data;

        for(int i = 0; i < chunk.vertex_buffer_size / 7; i++) {
            const mc_block_vertex& cur_vertex = chunk.vertex_data[i];
            nova_vertex new_vertex = {};

            // Position
            new_vertex.position.x = cur_vertex.x;
            new_vertex.position.y = cur_vertex.y;
            new_vertex.position.z = cur_vertex.z;

            new_vertex.uv0.x = cur_vertex.uv0_u;
            new_vertex.uv0.y = cur_vertex.uv0_v;

            new_vertex.color.r = (float)cur_vertex.r / 255.0f;
            new_vertex.color.g = (float)cur_vertex.g / 255.0f;
            new_vertex.color.b = (float)cur_vertex.b / 255.0f;
            new_vertex.color.a = (float)cur_vertex.a / 255.0f;

            new_vertex.uv1.x = cur_vertex.uv1_u;
            new_vertex.uv1.y = cur_vertex.uv1_v;

            vertex_data.push_back(new_vertex);
        }

        for(int i = 0; i < chunk.index_buffer_size; i++) {
            def.indices.push_back(chunk.indices[i]);
        }

        def.vertex_format = format::all_values()[chunk.format];
        def.position = {chunk.x, chunk.y, chunk.z};
        def.id = chunk.id;

        remove_chunk_render_object(filter_name,chunk);

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

        mesh_definition quad;
        quad.vertex_data = std::vector<nova_vertex>{
                nova_vertex{
                        {-1.0f, -3.0f, 0.0f},                 // Position
                        {0.0f, -1.0f},                               // UV0
                        {0.0f, 0.0f},                               // MidTexCoord
                        0,                                                      // VirtualTextureId
                        {1.0f, 1.0f, 1.0f, 1.0f},     // Color
                        {0.0f, 1.0f},                               // UV1
                        {0.0f, 0.0f, 0.0f},                  // Normal
                        {0.0f, 0.0f, 0.0f},                  // Tangent
                        {0.0f, 0.0f, 0.0f, 0.0f}     // McEntityId
                },

                nova_vertex{
                        {3.0f,  1.0f,  0.0f},                  // Position
                        {2.0f, 1.0f},                               // UV0
                        {0.0f, 0.0f},                               // MidTexCoord
                        0,                                                      // VirtualTextureId
                        {1.0f, 1.0f, 1.0f, 1.0f},     // Color
                        {0.0f, 0.0f},                               // UV1
                        {0.0f, 0.0f, 0.0f},                  // Normal
                        {0.0f, 0.0f, 0.0f},                  // Tangent
                        {0.0f, 0.0f, 0.0f, 0.0f}     // McEntityId
                },

                nova_vertex{
                        {-1.0f, 1.0f,  0.0f},                  // Position
                        {0.0f, 1.0f},                               // UV0
                        {0.0f, 0.0f},                               // MidTexCoord
                        0,                                                      // VirtualTextureId
                        {1.0f, 1.0f, 1.0f, 1.0f},     // Color
                        {0.0f, 0.0f},                 // UV1
                        {0.0f, 0.0f, 0.0f},           // Normal
                        {0.0f, 0.0f, 0.0f},           // Tangent
                        {0.0f, 0.0f, 0.0f, 0.0f}     // McEntityId
                }
        };

        quad.indices = {2, 1, 0};
        quad.vertex_format = format::POS_COLOR_UV_LIGHTMAPUV_NORMAL_TANGENT;
        quad.position = glm::vec3(0);
        quad.id = 0;

        geometry_to_upload_lock.lock();
        geometry_to_upload.emplace(material_name, quad);
        geometry_to_upload_lock.unlock();

        has_fullscreen_quad[material_name] = true;
    }
}
