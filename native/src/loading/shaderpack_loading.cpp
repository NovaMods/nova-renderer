/*!
 * \author ddubois 
 * \date 21-Aug-18.
 */

#include "shaderpack_loading.hpp"
#include "folder_accessor.hpp"
#include "loading_utils.hpp"
#include "zip_folder_accessor.hpp"
#include "regular_folder_accessor.hpp"

namespace nova {
    fs::path SHADERPACK_ROOT("shaderpacks");
    fs::path BEDROCK_SHADERPACK_ROOT("resourcepacks");

    folder_accessor_base* get_shaderpack_accessor(const fs::path &shaderpack_name);

    shaderpack_data load_shaderpack_data(const fs::path& shaderpack_name) {
        folder_accessor_base* folder_access = get_shaderpack_accessor(shaderpack_name);

        // The shaderpack has a number of items: There's the shaders themselves, of course, but there's so, so much more
        // What else is there?
        // - resources.json, to describe the dynamic resources that a shaderpack needs
        // - passes.json, to describe the frame graph itself
        // - All the pipeline descriptions
        // - All the material descriptions
        //
        // All these things are loaded from the filesystem


        delete folder_access;
    }

    folder_accessor_base* get_shaderpack_accessor(const fs::path &shaderpack_name) {
        folder_accessor_base* folder_access = nullptr;
        fs::path path_to_shaderpack = SHADERPACK_ROOT / shaderpack_name;

        // Where is the shaderpack, and what kind of folder is it in?
        if(is_zip_folder(path_to_shaderpack)) {
            // zip folder in shaderpacks folder
            path_to_shaderpack.replace_extension(".zip");
            folder_access = new zip_folder_accessor(path_to_shaderpack);

        } else if(std::experimental::filesystem::v1::exists(path_to_shaderpack)) {
            // regular folder in shaderpacks folder
            folder_access = new regular_folder_accessor(path_to_shaderpack);

        } else {
            path_to_shaderpack = BEDROCK_SHADERPACK_ROOT / shaderpack_name;

            if(is_zip_folder(path_to_shaderpack)) {
                // zip folder in the resourcepacks folder
                path_to_shaderpack.replace_extension(".zip");
                folder_access = new zip_folder_accessor(path_to_shaderpack);

            } else if(std::experimental::filesystem::v1::exists(path_to_shaderpack)) {
                folder_access = new regular_folder_accessor(path_to_shaderpack);
            }
        }

        if(folder_access == nullptr) {
            throw resource_not_found_error(shaderpack_name.string());
        }

        return folder_access;
    }
}
