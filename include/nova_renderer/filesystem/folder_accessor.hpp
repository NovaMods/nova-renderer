#pragma once

#include <rx/core/concurrency/mutex.h>
#include <rx/core/filesystem/directory.h>
#include <rx/core/map.h>
#include <rx/core/optional.h>
#include <rx/core/string.h>
#include <stdint.h>

namespace nova::filesystem {
    /*!
     * \brief A collection of resources on the filesystem
     *
     * "resourcepack" isn't the exact right name here. This isn't strictly a resourcepack in the Minecraft sense - it
     * can be, sure, but it can also be a pure renderpack. Ths main point is to abstract away loading resources from a
     * folder or a zip file - the calling code shouldn't care how the data is stored on the filesystem
     */
    class FolderAccessorBase {
    public:
        [[nodiscard]] static FolderAccessorBase* create(const std::string& path);

        /*!
         * \brief Initializes this resourcepack to load resources from the folder/zip file with the provided name
         * \param folder The name of the folder or zip file to load resources from, relative to Nova's working directory
         */
        explicit FolderAccessorBase(std::string folder);

        FolderAccessorBase(FolderAccessorBase&& other) noexcept = default;
        FolderAccessorBase& operator=(FolderAccessorBase&& other) noexcept = default;

        FolderAccessorBase(const FolderAccessorBase& other) = delete;
        FolderAccessorBase& operator=(const FolderAccessorBase& other) = delete;

        virtual ~FolderAccessorBase() = default;

        /*!
         * \brief Checks if the given resource exists
         *
         * This function locks resource_existence_mutex, so any methods which are called by this -
         * does_resource_exist_internal and does_resource_exist_in_map - MUST not try to lock resource_existence_mutex
         *
         * \param resource_path The path to the resource you want to know the existence of, relative to this
         * resourcepack's root
         * \return True if the resource exists, false if it does not
         */
        [[nodiscard]] bool does_resource_exist(const std::string& resource_path);

        [[nodiscard]] virtual std::vector<uint8_t> read_file(const std::string& path) = 0;

        /*!
         * \brief Loads the resource with the given path
         * \param resource_path The path to the resource to load, relative to this resourcepack's root
         * \return All the bytes in the loaded resource
         */
        [[nodiscard]] std::string read_text_file(const std::string& resource_path);

        /*!
         * \brief Retrieves the paths of all the items in the specified folder
         * \param folder The folder to get all items from
         * \return A list of all the paths in the provided folder
         */
        [[nodiscard]] virtual std::vector<std::string> get_all_items_in_folder(const std::string& folder) = 0;

        [[nodiscard]] const std::string& get_root() const;

        [[nodiscard]] virtual FolderAccessorBase* create_subfolder_accessor(const std::string& path) const = 0;

    protected:
        std::string root_folder;

        /*!
         * \brief I expect certain resources, like textures, to be requested a lot as Nova streams them in and out of
         * VRAM. This map caches if a resource exists or not - if a path is absent from the map, it's never been
         * requested and we don't know if it exists. However, if a path has been checked before, we can now save an IO
         * call!
         */
        std::unordered_map<std::string, bool> resource_existence;

        rx::concurrency::mutex* resource_existence_mutex;

        [[nodiscard]] rx::optional<bool> does_resource_exist_in_map(const std::string& resource_string) const;

        /*!
         * \brief Like the non-internal one, but does not add the folder's root to resource_path
         *
         * \param resource_path The path to the resource, with `our_root` already appended
         */
        [[nodiscard]] virtual bool does_resource_exist_on_filesystem(const std::string& resource_path) = 0;
    };

    /*!
     * \brief Checks if the given path has the other path as its root
     * \param path The path to check if it has the root
     * \param root The potential root path of the file
     * \return True if `path` has `root` as its root, false otherwise
     */
    [[nodiscard]] bool has_root(const std::string& path, const std::string& root);
} // namespace nova::filesystem
