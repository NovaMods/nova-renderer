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
#include "builders/gui_geometry_builder.h"
#include "../../../render/objects/textures/texture_manager.h"
#include "../../../render/nova_renderer.h"

namespace nova {
    std::vector<render_object *> mesh_store::get_meshes_for_shader(std::string shader_name) {
        return renderables_grouped_by_shader[shader_name];
    }

    void mesh_store::add_gui_geometry(mc_gui_screen &screen) {
        if (are_different_screens(screen, cur_gui_screen)) {
            remove_render_objects([](render_object* object) {return object->type == geometry_type::gui; });

            mesh_definition gui_mesh = build_gui_geometry(screen);

            render_object *gui = new render_object();
            gui->geometry = new gl_mesh(gui_mesh);
            gui->type = geometry_type::gui;
            gui->name = "gui";
            gui->is_solid = true;

            sort_render_object(gui);
        }
    }

    void print_buffers(std::string texture_name, std::vector<float>& vertex_buffer, std::vector<unsigned short>& index_buffer) {
        // debug
        LOG(DEBUG) << "texture name: " << texture_name << std::endl;
        LOG(DEBUG) << "new buffers:" << std::endl;
        for (int i = 0; i + 4 < vertex_buffer.size(); i += 5) {
            std::ostringstream ss;
            ss << "  vertex ";
            for (int k = 0; k < 5; k++) {
                ss << std::setfill(' ') << std::setw(4) << i + k << " = " << std::setfill(' ') << std::setw(12) << std::fixed << std::setprecision(5) << vertex_buffer[i + k] << "  ";
            }
            LOG(DEBUG) << ss.str();
        }
        for (int i = 0; i + 2 < index_buffer.size(); i += 3) {
            std::ostringstream ss;
            ss << "  index ";
            for (int k = 0; k < 3; k++) {
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
        LOG(DEBUG) << "Recieved GUI buffer with texture " << texture_name;
        const texture_manager::texture_location tex_location = nova_renderer::instance->get_texture_manager().get_texture_location(texture_name);
        glm::vec2 tex_size = tex_location.max - tex_location.min;

        std::vector<float> vertex_buffer(command->vertex_buffer_size);
        for (int i = 0; i + 4 < command->vertex_buffer_size; i += 5) {
            vertex_buffer[i] = command->vertex_buffer[i];
            vertex_buffer[i+1] = command->vertex_buffer[i+1];
            vertex_buffer[i+2] = command->vertex_buffer[i+2];
            vertex_buffer[i+3] = command->vertex_buffer[i+3] * tex_size.x + tex_location.min.x;
            vertex_buffer[i+4] = command->vertex_buffer[i+4] * tex_size.y + tex_location.min.y;
        }
        std::vector<unsigned short> index_buffer(command->index_buffer_size);
        for (int i = 0; i < command->index_buffer_size; i++) {
            index_buffer[i] = (unsigned short)command->index_buffer[i];
        }

        // debug
        print_buffers(texture_name, vertex_buffer, index_buffer);

        mesh_definition cur_screen_buffer;
        cur_screen_buffer.vertex_data = vertex_buffer;
        cur_screen_buffer.indices = index_buffer;
        cur_screen_buffer.vertex_format = format::POS_UV;

        render_object *gui = new render_object();
        gui->geometry = new gl_mesh(cur_screen_buffer);
        gui->type = geometry_type::gui;
        gui->name = "gui";
        gui->is_solid = true;

        sort_render_object(gui);
    }

    void mesh_store::sort_render_object(render_object *object) {
        auto& all_shaders = shaders->get_loaded_shaders();
        for (auto& entry : all_shaders) {
            auto& filter = entry.second.get_filter();
            if (matches_filter(object, filter)) {
                renderables_grouped_by_shader[entry.first].push_back(object);
            }
        }
    }

    void mesh_store::remove_render_objects(std::function<bool(render_object*)> filter) {
        for(auto& group : renderables_grouped_by_shader) {
            std::remove_if(group.second.begin(), group.second.end(), filter);
        }
    }

    bool mesh_store::matches_filter(render_object *object, geometry_filter &filter) {
        for(auto& name : filter.names) {
            if(object->name == name) {
                return true;
            }
        }

        for(auto& name_part : filter.name_parts) {
            if(object->name.find(name_part) != std::string::npos) {
                return true;
            }
        }

        bool matches = false;
        bool matches_geometry_type = false;
        for(auto& geom_type : filter.geometry_types) {
            if(object->type == geom_type) {
                matches_geometry_type = true;
            }
        }

        matches |= matches_geometry_type;
        if(filter.geometry_types.size() == 0) {
            matches = true;
        }

        if(filter.should_be_solid) {
            matches |= *filter.should_be_solid && object->is_solid;
        }
        if(filter.should_be_transparent) {
            matches |= *filter.should_be_transparent && object->is_transparent;
        }
        if(filter.should_be_cutout) {
            matches |= *filter.should_be_cutout && object->is_cutout;
        }
        if(filter.should_be_emissive) {
            matches |= *filter.should_be_emissive&& object->is_emissive;
        }
        if(filter.should_be_damaged) {
            matches |= *filter.should_be_damaged ? object->damage_level > 0 : object->damage_level == 0;
        }

        return matches;
    }

    void mesh_store::set_shaderpack(shaderpack &new_shaderpack) {
        shaders = &new_shaderpack;
    }

    bool are_different_screens(const mc_gui_screen &screen1, const mc_gui_screen &screen2) {
        if(screen1.num_buttons != screen2.num_buttons) {
            return true;
        }

        for(int i = 0; i < MAX_NUM_BUTTONS; i++) {
            if(i >= screen1.num_buttons || i >= screen2.num_buttons) {
                break;
            }
            LOG(TRACE) << "Checking button " << i << " for similarity";
            if(are_different_buttons(screen1.buttons[i], screen2.buttons[i])) {
                LOG(TRACE) << "Button " << i << " is different";
                return true;
            }

            LOG(TRACE) << "Button " << i << " is the same in both screens";
        }

        return false;
    }

    bool are_different_buttons(const mc_gui_button &button1, const mc_gui_button &button2) {
        bool same_rect = button1.x_position == button2.x_position &&
                         button1.y_position == button2.y_position &&
                         button1.width == button2.width &&
                         button1.height == button2.height;

        bool same_text = !are_different_strings(button1.text, button2.text);

        bool same_pressed = button1.is_pressed == button2.is_pressed;

        return !same_rect || !same_text || !same_pressed;
    }

    bool are_different_strings(const char *text1, const char *text2) {
        if(text1 == nullptr && text2 == nullptr) {
            // They're both null, and null equals null, so they're the same
            // If this causes problems I'll change it
            return false;
        }

        if(text1 == nullptr) {
            return true;
        }

        if(text2 == nullptr) {
            return true;
        }

        return strcmp(text1, text2) != 0;
    }
}