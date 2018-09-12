/*!
 * \author ddubois 
 * \date 14-Aug-18.
 */

#ifndef NOVA_RENDERER_RESOURCEPACK_H
#define NOVA_RENDERER_RESOURCEPACK_H

#include <string>
#include <vector>
#include <experimental/filesystem>
#include <unordered_map>
#include <optional>

namespace fs = std::experimental::filesystem;

namespace nova {
    class resource_not_found_error : public std::runtime_error {
    public:
        explicit resource_not_found_error(const std::string &resource_name);

    };

    /*!
     * \brief A collection of resources on the filsysstem
     *
     * "resourcepack" isn't the exact right name here. This isn't strictly a resourcepack in the Minecraft sense - it
     * can be, sure, but it can also be a pure shaderpack. Ths main point is to abstract away loading resources from a
     * folder or a zip file - the calling code shouldn't care how the data is stored on the filesystem
     */
    class folder_accessor_base {
    public:
        /*!
         * \brief Initializes this resourcepack to load resources from the folder/zip file with the provided name
         * \param folder The name of the folder or zip file to load resources from, relative to Nova's working directory
         */
        explicit folder_accessor_base(const fs::path& folder);

        /*!
         * \brief Checks if the given resource exists
         * \param resource_path The path to the resource you want to know the existence of, relative to this
         * resourcepack's root
         * \return True if the resource exists, false if it does not
         */
        bool does_resource_exist(const fs::path& resource_path);

        /*!
         * \brief Loads the resource with the given path
         * \param resource_path The path to the resource to load, relative to this resourcepack's root
         * \return All the bytes in the loaded resource
         */
        virtual std::string read_text_file(const fs::path& resource_path) = 0;

        /*!
         * \brief Retrieves the paths of all the items in the specified folder
         * \param folder The folder to get all items from
         * \return A list of all the paths in the provided folder
         */
        virtual std::vector<fs::path> get_all_items_in_folder(const fs::path& folder) = 0;

    protected:
        fs::path our_folder;

        /*!
         * \brief I expect certain resources, like textures, to be requested a lot as Nova streams them in and out of
         * VRAM. This map caches if a resource exists or not - if a path is absent from the map, it's never been
         * requested and we don't know if it exists. However, if a path has been checked before, we can now save an IO
         * call!
         */
        std::unordered_map<std::string, bool> resource_existance;

        std::optional<bool> does_resource_exist_in_map(const std::string& resource_path) const;
        
        /*!
         * \brief Like the non-internal one, but does not add the folder's root to resource_path
         * 
         * \param resource_path The path to the resource, with `our_root` already appended
         */
        virtual bool does_resource_exist_internal(const fs::path& resource_path) = 0;
    };
}

#endif //NOVA_RENDERER_RESOURCEPACK_H
