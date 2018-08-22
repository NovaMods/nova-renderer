/*!
 * \author ddubois 
 * \date 21-Aug-18.
 */

#ifndef NOVA_RENDERER_SHADERPACK_LOADING_HPP
#define NOVA_RENDERER_SHADERPACK_LOADING_HPP

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

namespace nova {
    void load_shaderpack_data(const fs::path& shaderpack_name);
}

#endif //NOVA_RENDERER_SHADERPACK_LOADING_HPP
