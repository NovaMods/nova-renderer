/*!
* \brief
*
* \author ddubois
* \date 27-Sep-16.
*/

#include <algorithm>
#include <easylogging++.h>
#include <regex>
#include "mesh_store.h"
#include "../../../render/nova_renderer.h"
#include "builders/chunk_builder.h"
#include "../utils/io.h"

namespace nova {
    std::vector<render_object>& mesh_store::get_meshes_for_shader(std::string shader_name) {
        return renderables_grouped_by_shader[shader_name];
    }

    void print_buffers(std::string texture_name, std::vector<float>& vertex_buffer, std::vector<unsigned int>& index_buffer) {
        // debug
        LOG(DEBUG) << "texture name: " << texture_name << std::endl;
        LOG(DEBUG) << "new buffers:" << std::endl;
        for(int i = 0; i + 7 < vertex_buffer.size(); i += 8) {
            std::ostringstream ss;
            ss << "  vertex ";
            for(int k = 0; k < 8; k++) {
                ss << std::setfill(' ') << std::setw(4) << i + k << " = " << std::setfill(' ') << std::setw(12) << std::fixed << std::setprecision(5) << vertex_buffer[i + k] << "  ";
            }
            LOG(DEBUG) << ss.str();
        }
        for(int i = 0; i + 2 < index_buffer.size(); i += 3) {
            std::ostringstream ss;
            ss << "  index ";
            for(int k = 0; k < 3; k++) {
                ss << std::setfill(' ') << std::setw(4) << i + k << " = " << std::setfill(' ') << std::setw(8) << index_buffer[i + k] << "  ";
            }
            LOG(DEBUG) << ss.str();
        }
    }

    void mesh_store::add_gui_buffers(mc_gui_send_buffer_command* command) {
        std::string texture_name(command->texture_name);
        texture_name = std::regex_replace(texture_name, std::regex("^textures/"), "");
        texture_name = std::regex_replace(texture_name, std::regex(".png$"), "");
        texture_name = "minecraft:" + texture_name;
        const texture_manager::texture_location tex_location = nova_renderer::instance->get_texture_manager().get_texture_location(texture_name);
        glm::vec2 tex_size = tex_location.max - tex_location.min;

        std::vector<float> vertex_buffer(command->vertex_buffer_size);
        for (int i = 0; i + 8 < command->vertex_buffer_size; i += 9) {
            vertex_buffer[i]   = command->vertex_buffer[i];
            vertex_buffer[i+1] = command->vertex_buffer[i+1];
            vertex_buffer[i+2] = command->vertex_buffer[i+2];
            vertex_buffer[i+3] = command->vertex_buffer[i+3] * tex_size.x + tex_location.min.x;
            vertex_buffer[i+4] = command->vertex_buffer[i+4] * tex_size.y + tex_location.min.y;
            vertex_buffer[i+5] = command->vertex_buffer[i+5];
            vertex_buffer[i+6] = command->vertex_buffer[i+6];
            vertex_buffer[i+7] = command->vertex_buffer[i+7];
            vertex_buffer[i+8] = command->vertex_buffer[i+8];
        }
        std::vector<unsigned int> index_buffer(command->index_buffer_size);
        for(int i = 0; i < command->index_buffer_size; i++) {
            index_buffer[i] = (unsigned int)command->index_buffer[i];
        }

        // debug
        //print_buffers(texture_name, vertex_buffer, index_buffer);

        mesh_definition cur_screen_buffer;
        cur_screen_buffer.vertex_data = vertex_buffer;
        cur_screen_buffer.indices = index_buffer;
        cur_screen_buffer.vertex_format = format::POS_UV_COLOR;

        render_object gui = {};
        gui.geometry.reset(new gl_mesh(cur_screen_buffer));
        gui.type = geometry_type::gui;
        gui.name = "gui";
        gui.is_solid = true;
        gui.color_texture = command->atlas_name;

        sort_render_object(gui);
    }

    void mesh_store::remove_gui_render_objects() {
        for(auto& group : renderables_grouped_by_shader) {
            auto removed_elements = std::remove_if(group.second.begin(), group.second.end(),
                                                   [](auto& render_obj) {return render_obj.type == geometry_type::gui;});
            group.second.erase(removed_elements, group.second.end());
        }
    }

    void mesh_store::sort_render_object(render_object& object) {
        auto& all_shaders = shaders->get_loaded_shaders();
        for(auto& entry : all_shaders) {
            auto filter = entry.second.get_filter();
            if(filter->matches(object)) {
                renderables_grouped_by_shader[entry.first].push_back(std::move(object));
            }
        }
    }

    void mesh_store::remove_render_objects(std::function<bool(render_object&)> filter) {
        for(auto& group : renderables_grouped_by_shader) {
            std::remove_if(group.second.begin(), group.second.end(), filter);
        }
    }

    void mesh_store::set_shaderpack(std::shared_ptr<shaderpack> new_shaderpack) {
        shaders = new_shaderpack;

        renderables_grouped_by_shader.clear();
        LOG(INFO) << "Rebuilding chunk geometry to fit new shaderpack";
        for(auto& chunk : all_chunks) {
            add_or_update_chunk(chunk);
        }
        LOG(INFO) << "Rebuild complete";
    }

    void mesh_store::add_or_update_chunk(mc_chunk &chunk) {
        try {
            all_chunks_lock.lock();
            chunk.needs_update = true;
            all_chunks.push_back(chunk);
            int chunk_id = chunk.chunk_id;
            remove_render_objects([chunk_id](render_object &obj) { return obj.parent_id == chunk_id; });
            all_chunks_lock.unlock();

            for(const auto &shader_entry : shaders->get_loaded_shaders()) {
                auto blocks_for_shader = m_chunk_builder.get_blocks_that_match_filter(chunk, shader_entry.second.get_filter());
                if(blocks_for_shader.size() == 0) {
                    continue;
                }

                auto block_mesh_definition = m_chunk_builder.make_mesh_for_blocks(blocks_for_shader, chunk);
                LOG(DEBUG) << "Made mesh for chunk at position " << chunk.x << ", " << chunk.z << " and shader "
                          << shader_entry.first;
                chunk_parts_to_upload_lock.lock();
                chunk_parts_to_upload.emplace(shader_entry.first, block_mesh_definition);
                chunk_parts_to_upload_lock.unlock();
            }
        } catch(std::exception& e) {
            LOG(ERROR) << "Could not build chunk at position " << chunk.x << ", " << chunk.z << " because '" << e.what() << "'";
        }
    }

    void mesh_store::generate_needed_chunk_geometry() {
        bool has_thing_to_upload = false;
        std::tuple<std::string, mesh_definition> definition_to_upload;
        chunk_parts_to_upload_lock.lock();
        if(!chunk_parts_to_upload.empty()) {
            definition_to_upload = std::move(chunk_parts_to_upload.front());
            chunk_parts_to_upload.pop();
            has_thing_to_upload = true;
        }
        chunk_parts_to_upload_lock.unlock();

        if(has_thing_to_upload) {
            auto chunk_part_render_object = render_object{};
            chunk_part_render_object.geometry = std::make_unique<gl_mesh>(std::get<1>(definition_to_upload));
            chunk_part_render_object.position = std::get<1>(definition_to_upload).position;
            chunk_part_render_object.color_texture = "block_color";

            renderables_grouped_by_shader[std::get<0>(definition_to_upload)].push_back(std::move(chunk_part_render_object));
        }
    }

    chunk_builder& mesh_store::get_chunk_builder() {
        return m_chunk_builder;
    }
}
