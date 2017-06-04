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

namespace nova {
    std::vector<render_object>& mesh_store::get_meshes_for_shader(std::string shader_name) {
        return renderables_grouped_by_shader[shader_name];
    }

    void print_buffers(std::string texture_name, std::vector<float>& vertex_buffer, std::vector<unsigned short>& index_buffer) {
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
        std::vector<unsigned short> index_buffer(command->index_buffer_size);
        for(int i = 0; i < command->index_buffer_size; i++) {
            index_buffer[i] = (unsigned short)command->index_buffer[i];
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
            auto& filter = entry.second.get_filter();
            if(filter.matches(object)) {
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
        for(auto& chunk : all_chunks) {
            generate_chunk_geometry(chunk);
        }
    }

    void mesh_store::add_or_update_chunk(mc_chunk &chunk) {
        LOG(DEBUG) << "Adding chunk";
        auto start_time = std::clock();

        remove_render_objects([&](render_object& obj) {return obj.parent_id == chunk.chunk_id;});
        LOG(DEBUG) << "Removed old geometry";

        auto time_after_removing_objects = std::clock();

        generate_chunk_geometry(chunk);
        LOG(DEBUG) << "Generated chunk geometry";

        auto time_after_generating_chunk_geometry = std::clock();

        // Save the chunk so that we can re-parse it when we get a new shaderpack
        std::remove_if(all_chunks.begin(), all_chunks.end(), [&](mc_chunk& chunk1) {return chunk.chunk_id == chunk1.chunk_id;});
        LOG(DEBUG) << "Removed old geometry with this chunk's ID";
        auto time_after_removing_chunks = std::clock();
        all_chunks.push_back(chunk);
        LOG(DEBUG) << "Added new chunk to list of chunks";
        auto time_after_adding_chunk = std::clock();
        LOG(DEBUG) << "time_after_adding_chunks: " << time_after_adding_chunk;

        auto total_time = float(std::clock() - start_time) * 1000 / CLOCKS_PER_SEC;
        LOG(DEBUG) << "total_time: " << total_time;
        total_chunks_updated += 1;
        LOG(DEBUG) << "total_chunks_updated: " << total_chunks_updated;

        if(total_chunks_updated % 10 == 0) {
            LOG(INFO) << "We have spent:\n\t"
                      << float(time_after_removing_objects - start_time) * 1000 / CLOCKS_PER_SEC << "ms removing old render objects\n\t"
                      << float(time_after_generating_chunk_geometry - time_after_removing_objects) * 1000 / CLOCKS_PER_SEC << "ms generating chunk geometry\n\t"
                      << float(time_after_removing_chunks - time_after_generating_chunk_geometry) * 1000 / CLOCKS_PER_SEC << "ms removing chunks\n\t"
                      << float(time_after_adding_chunk - time_after_removing_chunks) * 1000 / CLOCKS_PER_SEC << "ms adding the new chunk\n\t"
                      << total_time << "ms in total";
        }
    }

    void mesh_store::generate_chunk_geometry(mc_chunk &chunk) {
        auto render_objects_from_chunk = get_renderables_from_chunk(chunk, *shaders);
        for(auto& item : render_objects_from_chunk) {
            if(item.second) {
                renderables_grouped_by_shader[item.first].push_back(std::move(*item.second));
            }
        }
    }

    void mesh_store::register_model(std::string model_name, mc_simple_model &mc_model) {
        mesh_definition model = make_mesh_from_mc_model(mc_model);

		simple_models[model_name] = model;
    }

    mesh_definition mesh_store::make_mesh_from_mc_model(mc_simple_model &model) {
        mesh_definition mesh;
        // TODO: This method needs to happen
        return mesh;
    }

    void mesh_store::deregister_model(std::string model_name) {
        simple_models.erase(model_name);
    }
}
