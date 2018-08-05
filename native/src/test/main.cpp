#include "../render/nova_renderer.h"

#include <easylogging++.h>
#include "../render/objects/resources/texture2D.h"
#include "../mc_interface/nova.h"

#include <iostream>
#include <fstream>

int main(int argc, char **argv) {
    std::ifstream in;
    in.open("nova_dump_input", std::ios::in | std::ios::binary);
    while (!in.eof()) {
        std::string str;
        std::getline(in, str, '\0');
        std::cout << "Func: " << str;
        if (str == "dump_initialize") {
            initialize();
        } else if (str == "dump_add_texture") {
            mc_atlas_texture tex = {};
            in.read((char*) &tex.width, sizeof(tex.width));
            in.read((char*) &tex.height, sizeof(tex.height));
            in.read((char*) &tex.num_components, sizeof(tex.num_components));

            std::vector<char> data(tex.width * tex.height * tex.num_components);
            in.read(data.data(), data.size());

            std::string name;
            std::getline(in, name, '\0');
            tex.name = name.c_str();
            tex.texture_data = (unsigned char*) data.data();

            add_texture(&tex);
        } else if (str == "dump_reset_texture_manager") {
            reset_texture_manager();
        } else if (str == "dump_send_lightmap_texture") {
            int count, width, height;
            in.read((char*) &count, sizeof(count));
            in.read((char*) &width, sizeof(width));
            in.read((char*) &height, sizeof(height));

            uint32_t data[width * height];
            in.read((char*) data, sizeof(data));

            send_lightmap_texture(data, count, width, height);
        } else if (str == "dump_add_texture_location") {
            mc_texture_atlas_location loc = {};
            std::string name;
            std::getline(in, name, '\0');

            loc.name = name.c_str();
            in.read((char*) &loc.min_u, sizeof(loc.min_u));
            in.read((char*) &loc.min_v, sizeof(loc.min_v));
            in.read((char*) &loc.max_u, sizeof(loc.max_u));
            in.read((char*) &loc.max_v, sizeof(loc.max_v));

            add_texture_location(&loc);
        } else if (str == "dump_get_max_texture_size") {
            get_max_texture_size();
        } else if (str == "dump_add_chunk_geometry_for_filter") {
            std::string name;
            std::getline(in, name, '\0');

            mc_chunk_render_object chunk = {};
            in.read((char*) &chunk.format, sizeof(chunk.format));
            in.read((char*) &chunk.x, sizeof(chunk.x));
            in.read((char*) &chunk.y, sizeof(chunk.y));
            in.read((char*) &chunk.z, sizeof(chunk.z));
            in.read((char*) &chunk.id, sizeof(chunk.id));
            in.read((char*) &chunk.vertex_buffer_size, sizeof(chunk.vertex_buffer_size));
            in.read((char*) &chunk.index_buffer_size, sizeof(chunk.index_buffer_size));

            std::vector<char> vertex_data(chunk.vertex_buffer_size);
            in.read(vertex_data.data(), vertex_data.size());

            std::vector<char> index_data(chunk.index_buffer_size * sizeof(uint32_t));
            in.read(index_data.data(), index_data.size());

            chunk.vertex_data = (mc_block_vertex*) vertex_data.data();
            chunk.indices = (uint32_t*) index_data.data();

            add_chunk_geometry_for_filter(name.c_str(), &chunk);

        } else if (str == "dump_remove_chunk_geometry_for_filter") {
            std::string name;
            std::getline(in, name, '\0');

            mc_chunk_render_object chunk = {};
            in.read((char*) &chunk.format, sizeof(chunk.format));
            in.read((char*) &chunk.x, sizeof(chunk.x));
            in.read((char*) &chunk.y, sizeof(chunk.y));
            in.read((char*) &chunk.z, sizeof(chunk.z));
            in.read((char*) &chunk.id, sizeof(chunk.id));
            in.read((char*) &chunk.vertex_buffer_size, sizeof(chunk.vertex_buffer_size));
            in.read((char*) &chunk.index_buffer_size, sizeof(chunk.index_buffer_size));

            std::vector<char> vertex_data(chunk.vertex_buffer_size);
            in.read(vertex_data.data(), vertex_data.size());

            std::vector<char> index_data(chunk.index_buffer_size * sizeof(uint32_t));
            in.read(index_data.data(), index_data.size());

            chunk.vertex_data = (mc_block_vertex*) vertex_data.data();
            chunk.indices = (uint32_t*) index_data.data();

            remove_chunk_geometry_for_filter(name.c_str(), &chunk);

        } else if (str == "dump_execute_frame") {
            execute_frame();
        } else if (str == "dump_set_fullscreen") {
            uint32_t fs;
            in.read((char*) &fs, sizeof(uint32_t));
            set_fullscreen(fs);
        } else if (str == "dump_should_close") {
            if (should_close()) {
                return -1;
            }
        } else if (str == "dump_display_is_active") {
            display_is_active();
        } else if (str == "dump_add_gui_geometry") {
            std::string geo_type;
            std::string texture_name;
            std::string atlas_name;
            std::getline(in, geo_type, '\0');
            std::getline(in, texture_name, '\0');
            std::getline(in, atlas_name, '\0');

            mc_gui_geometry gui = {};
            gui.texture_name = texture_name.c_str();
            gui.atlas_name = atlas_name.c_str();

            in.read((char*) &gui.index_buffer_size, sizeof(gui.index_buffer_size));
            in.read((char*) &gui.vertex_buffer_size, sizeof(gui.vertex_buffer_size));

            char index_data[gui.index_buffer_size * sizeof(uint32_t)];
            char vertex_data[gui.vertex_buffer_size * sizeof(float)];

            in.read(index_data, sizeof(index_data));
            in.read(vertex_data, sizeof(vertex_data));

            gui.index_buffer = (uint32_t*) index_data;
            gui.vertex_buffer = (float*) vertex_data;

            add_gui_geometry(geo_type.c_str(), &gui);
        } else if (str == "dump_clear_gui_buffers") {
            clear_gui_buffers();
        } else if (str == "dump_set_string_setting") {
            std::string setting_name;
            std::string setting_value;
            std::getline(in, setting_name, '\0');
            std::getline(in, setting_value, '\0');

            set_string_setting(setting_name.c_str(), setting_value.c_str());
        } else if (str == "dump_set_float_setting") {
            std::string setting_name;
            float setting_value;
            std::getline(in, setting_name, '\0');
            in.read((char*) &setting_value, sizeof(setting_value));

            set_float_setting(setting_name.c_str(), setting_value);
        } else if (str == "dump_set_player_camera_transform") {
            double x, y, z;
            float yaw, pitch;

            in.read((char*) &x, sizeof(x));
            in.read((char*) &y, sizeof(y));
            in.read((char*) &z, sizeof(z));
            in.read((char*) &yaw, sizeof(yaw));
            in.read((char*) &pitch, sizeof(pitch));

            set_player_camera_transform(x, y, z, yaw, pitch);
        } else if (str == "dump_set_mouse_grabbed") {
            uint32_t grabbed;
            in.read((char*) &grabbed, sizeof(grabbed));
            set_mouse_grabbed(grabbed);
        } else if (str == "dump_get_materials_and_filters") {
            get_materials_and_filters();
        } else if (str == "dump_destruct") {
            destruct();
        } else {
            throw std::runtime_error("This should be impossible");
        }
    }

    nova::nova_renderer::deinit();
}
