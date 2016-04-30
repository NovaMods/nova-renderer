/*!
 * \author David
 * \date 29-Apr-16.
 */

#include <algorithm>
#include <easylogging++.h>
#include "texture_manager.h"

texture_manager::texture_manager(opengl_wrapper * wrapper) {
    gl_wrapper = wrapper;
}

texture_manager::~texture_manager() {
    // gotta free up all the OpenGL textures
    // The driver probably does that for me, but I ain't about to let no stinkin' driver boss me around!
    reset();
}

void texture_manager::reset() {
    if(atlases.empty()) {
        // Nothing to deallocate, let's just return
        return;
    }
    // Gather all the textures into a list so we only need one call to delete them
    std::vector<GLuint> texture_ids(atlases.size());
    for(auto tex : atlases) {
        texture_ids.push_back(tex.second.get_gl_name());
    }

    glDeleteTextures((GLsizei) texture_ids.size(), texture_ids.data());

    loaded_textures.clear();
    atlases.clear();
    locations.clear();
}

void texture_manager::add_texture(mc_texture & new_texture) {
    loaded_textures.push_back(new_texture);
}

void texture_manager::finalize_textures() {
    LOG(INFO) << "Finalizing " << loaded_textures.size() << " textures";

    // Make one texture for each atlas
    texture2D terrain_albedo_atlas = gl_wrapper->make_texture_2D();
    std::vector<mc_texture> terrain_albedo_textures;
    auto end = std::copy_if(
            loaded_textures.begin(),
            loaded_textures.end(),
            terrain_albedo_textures.begin(),
            [](mc_texture x){return std::string(x.name).find("blocks") != std::string::npos;}
    );
    terrain_albedo_textures.resize((unsigned long long int) std::distance(terrain_albedo_textures.begin(), end));

    pack_into_atlas(terrain_albedo_textures, terrain_albedo_atlas);

}

const texture_manager::texture_location &texture_manager::get_texture_location(std::string &texture_name) {
    return locations[texture_name];
}

itexture *texture_manager::get_texture_atlas(atlas_type atlas, texture_type type) {
    auto key = std::pair<atlas_type, texture_type>(atlas, type);
    return &atlases[key];
}

void texture_manager::pack_into_atlas(std::vector<mc_texture> &textures_to_pack, texture2D &atlas) {
    // Figure out how big our texture can be
    GLint max_tex_size;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &max_tex_size);

    // Figure out how long the texture would be if I laid every texture out side to side
    unsigned int atlas_x = (unsigned int) (textures_to_pack.size() * textures_to_pack[0].width);
    unsigned int allowed_x = std::max(atlas_x, (const unsigned int &) max_tex_size);
    unsigned int num_rows = atlas_x / allowed_x;
    num_rows++; // Pretty sure integer division will give me fewer rows than I need.

    if(num_rows > max_tex_size && atlas_x > max_tex_size) {
        // TODO: Create multiple atlases and manage them somehow
    }

    // Allocate data for the atlas
    // I'm always creating textures with four components. Not the best idea in terms of memory usage, but it should
    // work for now
    // TODO: Revisit this
    float * full_texture = new float[allowed_x * num_rows * 4];
    unsigned int 
}


