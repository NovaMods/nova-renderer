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

#ifdef DUMP_NOVA_CALLS
// func name is null terminated, no need to write length
#define DUMP_FUNC do { dump_out.write(__func__, sizeof(__func__)); dump_out.flush(); } while(false)
#define W_UINT32(x) do { uint32_t d[] = { (uint32_t) x }; dump_out.write((char*) d, sizeof(uint32_t)); } while(false)
#define W_FLOAT(x) do { float d[] = { (float) x }; dump_out.write((char*) d, sizeof(float)); } while(false)
#define W_DOUBLE(x) do { double d[] = { (double) x }; dump_out.write((char*) d, sizeof(double)); } while(false)

#else

#define DUMP_FUNC
#define W_UINT32(x)
#define W_FLOAT(x)
#define W_DOUBLE(x)

#endif

using namespace nova;

static std::ofstream dump_out;
static std::mutex dump_lock;
void dump_initialize() {
#ifdef DUMP_NOVA_CALLS
    dump_out.open("nova_debug_dump", std::ios::out | std::ios::trunc | std::ios::binary);
    dump_lock.lock();
    DUMP_FUNC;
    dump_lock.unlock();
#endif
}

void dump_add_texture(mc_atlas_texture* texture) {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    W_UINT32(texture->width);
    W_UINT32(texture->height);
    W_UINT32(texture->num_components);
    dump_out.write((char*) texture->texture_data, texture->width * texture->height * texture->num_components);
    dump_out.write(texture->name, strlen(texture->name) + 1);
    dump_out.flush();
    dump_lock.unlock();
#endif
}

void dump_reset_texture_manager() {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    dump_out.flush();
    dump_lock.unlock();
#endif
}

void dump_send_lightmap_texture(uint32_t* data, int count, int width, int height) {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    W_UINT32(count);
    W_UINT32(width);
    W_UINT32(height);
    dump_out.write((char*) data, width * height * sizeof(uint32_t));
    dump_out.flush();
    dump_lock.unlock();
#endif
}

void dump_add_texture_location(mc_texture_atlas_location* location) {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    dump_out.write((char*) location->name, strlen(location->name) + 1);
    W_FLOAT(location->min_u);
    W_FLOAT(location->min_v);
    W_FLOAT(location->max_u);
    W_FLOAT(location->max_v);
    dump_out.flush();
    dump_lock.unlock();
#endif
}

void dump_get_max_texture_size() {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    dump_lock.unlock();
#endif
}

void dump_add_chunk_geometry_for_filter(const char* filter_name, mc_chunk_render_object * chunk) {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    dump_out.write((char*) filter_name, strlen(filter_name) + 1);
    W_UINT32(chunk->format);
    W_FLOAT(chunk->x);
    W_FLOAT(chunk->y);
    W_FLOAT(chunk->z);
    W_UINT32(chunk->id);
    W_UINT32(chunk->vertex_buffer_size);
    W_UINT32(chunk->index_buffer_size);
    dump_out.write((char*) chunk->vertex_data, chunk->vertex_buffer_size);
    dump_out.write((char*) chunk->indices, chunk->index_buffer_size * sizeof(uint32_t));
    dump_out.flush();
    dump_lock.unlock();
#endif
}

void dump_remove_chunk_geometry_for_filter(const char* filter_name, mc_chunk_render_object * chunk) {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    dump_out.write((char*) filter_name, strlen(filter_name) + 1);
    W_UINT32(chunk->format);
    W_FLOAT(chunk->x);
    W_FLOAT(chunk->y);
    W_FLOAT(chunk->z);
    W_UINT32(chunk->id);
    W_UINT32(chunk->vertex_buffer_size);
    W_UINT32(chunk->index_buffer_size);
    dump_out.write((char*) chunk->vertex_data, chunk->vertex_buffer_size);
    dump_out.write((char*) chunk->indices, chunk->index_buffer_size * sizeof(uint32_t));
    dump_out.flush();
    dump_lock.unlock();
#endif
}

void dump_execute_frame() {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    dump_lock.unlock();
#endif
}

void dump_set_fullscreen(int fs) {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    W_UINT32(fs);
    dump_out.flush();
    dump_lock.unlock();
#endif
}

void dump_should_close() {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    dump_lock.unlock();
#endif
}

void dump_display_is_active() {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    dump_lock.unlock();
#endif
}

void dump_add_gui_geometry(const char * geo_type, mc_gui_geometry * gui_geometry) {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    dump_out.write((char*) geo_type, strlen(geo_type) + 1);
    dump_out.write((char*) gui_geometry->texture_name, strlen(gui_geometry->texture_name) + 1);
    dump_out.write((char*) gui_geometry->atlas_name, strlen(gui_geometry->atlas_name) + 1);
    W_UINT32(gui_geometry->index_buffer_size);
    W_UINT32(gui_geometry->vertex_buffer_size);
    dump_out.write((char*) gui_geometry->index_buffer, gui_geometry->index_buffer_size * sizeof(uint32_t));
    dump_out.write((char*) gui_geometry->vertex_buffer, gui_geometry->vertex_buffer_size * sizeof(float));
    dump_out.flush();
    dump_lock.unlock();
#endif
}

void dump_clear_gui_buffers() {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    dump_lock.unlock();
#endif
}

void dump_set_string_setting(const char * setting_name, const char * setting_value) {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    dump_out.write((char*) setting_name, strlen(setting_name) + 1);
    dump_out.write((char*) setting_value, strlen(setting_value) + 1);
    dump_out.flush();
    dump_lock.unlock();
#endif
}

void dump_set_float_setting(const char * setting_name, float setting_value) {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    dump_out.write((char*) setting_name, strlen(setting_name) + 1);
    W_FLOAT(setting_value);
    dump_out.flush();
    dump_lock.unlock();
#endif
}

void dump_set_player_camera_transform(double x, double y, double z, float yaw, float pitch) {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    W_DOUBLE(x);
    W_DOUBLE(y);
    W_DOUBLE(z);
    W_FLOAT(yaw);
    W_FLOAT(pitch);
    dump_out.flush();
    dump_lock.unlock();
#endif
}

void dump_set_mouse_grabbed(int grabbed) {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    W_UINT32(grabbed);
    dump_out.flush();
    dump_lock.unlock();
#endif
}

void dump_get_materials_and_filters() {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    dump_lock.unlock();
#endif
}

void dump_destruct() {
#ifdef DUMP_NOVA_CALLS
    dump_lock.lock();
    DUMP_FUNC;
    dump_lock.unlock();
    dump_out.close();
#endif
}
