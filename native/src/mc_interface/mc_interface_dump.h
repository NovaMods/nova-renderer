#include <nova/profiler.h>
#include "nova.h"
#include "../render/nova_renderer.h"
#include "../render/objects/resources/texture_manager.h"
#include "../input/InputHandler.h"
#include "../render/windowing/glfw_vk_window.h"
#include "../utils/utils.h"
#include "../render/objects/meshes/mesh_store.h"
#include <iostream>
#include <fstream>

// func name is null terminated, no need to write length
#define DUMP_FUNC do { out.write(__func__, sizeof(__func__)); out.flush(); } while(false)
#define W_UINT32(x) do { uint32_t d[] = { (uint32_t) x }; out.write((char*) d, sizeof(uint32_t)); } while(false)
#define W_FLOAT(x) do { float d[] = { (float) x }; out.write((char*) d, sizeof(uint32_t)); } while(false)
#define W_DOUBLE(x) do { double d[] = { (double) x }; out.write((char*) d, sizeof(uint32_t)); } while(false)

using namespace nova;

static std::ofstream out;

void dump_initialize() {
    out.open("nova_debug_dump", std::ios::out | std::ios::trunc | std::ios::binary);
    DUMP_FUNC;
}

void dump_add_texture(mc_atlas_texture* texture) {
    DUMP_FUNC;
    W_UINT32(texture->width);
    W_UINT32(texture->height);
    W_UINT32(texture->num_components);
    out.write((char*) texture->texture_data, texture->width * texture->height * texture->num_components);
    out.write(texture->name, strlen(texture->name) + 1);
    out.flush();
}

void dump_reset_texture_manager() {
    DUMP_FUNC;
}

void dump_send_lightmap_texture(uint32_t* data, int count, int width, int height) {
    DUMP_FUNC;
    W_UINT32(count);
    W_UINT32(width);
    W_UINT32(height);
    out.write((char*) data, width * height * sizeof(uint32_t));
    out.flush();
}

void dump_add_texture_location(mc_texture_atlas_location* location) {
    DUMP_FUNC;
    out.write((char*) location->name, strlen(location->name) + 1);
    W_FLOAT(location->min_u);
    W_FLOAT(location->min_v);
    W_FLOAT(location->max_u);
    W_FLOAT(location->max_v);
    out.flush();
}

void dump_get_max_texture_size() {
    DUMP_FUNC;
}

void dump_add_chunk_geometry_for_filter(const char* filter_name, mc_chunk_render_object * chunk) {
    DUMP_FUNC;
    out.write((char*) filter_name, strlen(filter_name) + 1);
    W_UINT32(chunk->format);
    W_FLOAT(chunk->x);
    W_FLOAT(chunk->y);
    W_FLOAT(chunk->z);
    W_UINT32(chunk->id);
    W_UINT32(chunk->vertex_buffer_size);
    W_UINT32(chunk->index_buffer_size);
    out.write((char*) chunk->vertex_data, chunk->vertex_buffer_size);
    out.write((char*) chunk->indices, chunk->index_buffer_size * sizeof(uint32_t));
    out.flush();
}

void dump_remove_chunk_geometry_for_filter(const char* filter_name, mc_chunk_render_object * chunk) {
    DUMP_FUNC;
    out.write((char*) filter_name, strlen(filter_name) + 1);
    W_UINT32(chunk->format);
    W_FLOAT(chunk->x);
    W_FLOAT(chunk->y);
    W_FLOAT(chunk->z);
    W_UINT32(chunk->id);
    W_UINT32(chunk->vertex_buffer_size);
    W_UINT32(chunk->index_buffer_size);
    out.write((char*) chunk->vertex_data, chunk->vertex_buffer_size);
    out.write((char*) chunk->indices, chunk->index_buffer_size * sizeof(uint32_t));
    out.flush();
}

void dump_execute_frame() {
    DUMP_FUNC;
}

void dump_set_fullscreen(int fs) {
    DUMP_FUNC;
    W_UINT32(fs);
    out.flush();
}

void dump_should_close() {
    DUMP_FUNC;
}

void dump_display_is_active() {
    DUMP_FUNC;
}

void dump_add_gui_geometry(const char * geo_type, mc_gui_geometry * gui_geometry) {
    DUMP_FUNC;
    out.write((char*) geo_type, strlen(geo_type) + 1);
    out.write((char*) gui_geometry->texture_name, strlen(gui_geometry->texture_name) + 1);
    out.write((char*) gui_geometry->atlas_name, strlen(gui_geometry->atlas_name) + 1);
    W_UINT32(gui_geometry->index_buffer_size);
    W_UINT32(gui_geometry->vertex_buffer_size);
    out.write((char*) gui_geometry->index_buffer, gui_geometry->index_buffer_size * sizeof(uint32_t));
    out.write((char*) gui_geometry->vertex_buffer, gui_geometry->vertex_buffer_size * sizeof(float));
    out.flush();
}

void dump_clear_gui_buffers() {
    DUMP_FUNC;
}

void dump_set_string_setting(const char * setting_name, const char * setting_value) {
    DUMP_FUNC;
    out.write((char*) setting_name, strlen(setting_name) + 1);
    out.write((char*) setting_value, strlen(setting_value) + 1);
}

void dump_set_float_setting(const char * setting_name, float setting_value) {
    DUMP_FUNC;
    out.write((char*) setting_name, strlen(setting_name) + 1);
    W_FLOAT(setting_value);
}

void dump_set_player_camera_transform(double x, double y, double z, float yaw, float pitch) {
    DUMP_FUNC;
    W_DOUBLE(x);
    W_DOUBLE(y);
    W_DOUBLE(z);
    W_FLOAT(yaw);
    W_FLOAT(pitch);
}

void dump_set_mouse_grabbed(int grabbed) {
    DUMP_FUNC;
    W_UINT32(grabbed);
}

void dump_get_materials_and_filters() {
    DUMP_FUNC;
}

void dump_destruct() {
    DUMP_FUNC;
    out.close();
}
