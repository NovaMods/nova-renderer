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
#include "mesh_store.h"
#include "../../../render/nova_renderer.h"
#include "../utils/io.h"

namespace nova {
    std::vector<render_object>& mesh_store::get_meshes_for_shader(std::string shader_name) {
        return renderables_grouped_by_shader[shader_name];
    }

    void print_buffers(const std::string &texture_name, std::vector<float>& vertex_buffer, std::vector<unsigned int>& index_buffer) {
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

        std::vector<float> vertex_buffer(static_cast<size_t>(command->vertex_buffer_size));
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
        std::vector<unsigned int> index_buffer(static_cast<size_t>(command->index_buffer_size));
        for(int i = 0; i < command->index_buffer_size; i++) {
            index_buffer[i] = (unsigned int)command->index_buffer[i];
        }

        mesh_definition cur_screen_buffer;
        cur_screen_buffer.vertex_data = vertex_buffer;
        cur_screen_buffer.indices = index_buffer;
        cur_screen_buffer.vertex_format = format::POS_UV_COLOR;

        render_object gui = {};
        gui.geometry = std::make_shared<gl_mesh>(cur_screen_buffer);
        gui.type = geometry_type::gui;
        gui.name = "gui";
        gui.is_solid = true;
        gui.color_texture = command->atlas_name;

        renderables_grouped_by_shader["gui"].push_back(gui);
    }

    void mesh_store::remove_gui_render_objects() {
        for(auto& group : renderables_grouped_by_shader) {
            auto removed_elements = std::remove_if(group.second.begin(), group.second.end(),
                                                   [](auto& render_obj) {return render_obj.type == geometry_type::gui;});
            group.second.erase(removed_elements, group.second.end());
        }
    }

    void mesh_store::remove_render_objects(std::function<bool(render_object&)> filter) {
        for(auto& group : renderables_grouped_by_shader) {
            std::remove_if(group.second.begin(), group.second.end(), filter);
        }
    }

    void mesh_store::upload_new_geometry() {
        chunk_parts_to_upload_lock.lock();
        while(!chunk_parts_to_upload.empty()) {
            const auto& entry = chunk_parts_to_upload.front();
            const auto def = std::get<1>(entry);

            render_object obj = {};
            obj.geometry = std::make_shared<gl_mesh>(def);
            obj.type = geometry_type::block;
            obj.name = "chunk";
            obj.is_solid = true;
            obj.parent_id = def.id;
            obj.color_texture = "block_color";

            renderables_grouped_by_shader[std::get<0>(entry)].push_back(obj);

            chunk_parts_to_upload.pop();
        }
        chunk_parts_to_upload_lock.unlock();
    }

    void mesh_store::add_geometry_for_filter(std::string filter_name, mc_basic_render_object &chunk) {
        mesh_definition def = {};

        def.vertex_data.reserve(static_cast<size_t>(chunk.vertex_buffer_size));
        for(int i = 0 ; i < chunk.vertex_buffer_size; i++) {
            def.vertex_data.push_back(((float*)chunk.vertex_data)[i]);
        }

        def.indices.reserve(static_cast<size_t>(chunk.index_buffer_size));
        for(int i = 0; i < chunk.index_buffer_size; i++) {
            def.indices.push_back(((unsigned int*)chunk.indices)[i]);
        }

        def.vertex_format = format::POS_UV_LIGHTMAPUV_NORMAL_TANGENT;
        def.position = {chunk.x, chunk.y, chunk.z};
        def.id = chunk.id;

        chunk_parts_to_upload_lock.lock();
        chunk_parts_to_upload.emplace(filter_name, def);
        chunk_parts_to_upload_lock.unlock();
    }
}
