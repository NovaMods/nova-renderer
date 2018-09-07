/*!
 * \author ddubois 
 * \date 15-Aug-18.
 */

#ifndef NOVA_RENDERER_ZIP_FOLDER_ACCESSOR_H
#define NOVA_RENDERER_ZIP_FOLDER_ACCESSOR_H

#include <miniz_zip.h>
#include "folder_accessor.hpp"

namespace fs = std::experimental::filesystem;

namespace nova {
    struct file_tree_node {
        std::string name;
        std::vector<file_tree_node*> children;
        file_tree_node* parent = nullptr;

        std::string get_full_path() const;
    };

    /*!
     * \brief Allows access to a zip folder
     */
    class zip_folder_accessor : public folder_accessor_base {
    public:
        explicit zip_folder_accessor(const fs::path &folder);
        ~zip_folder_accessor();

        bool does_resource_exist(const fs::path& resource_path) override;

        std::vector<uint8_t> read_resource(const fs::path &resource_path) override;

        std::vector<fs::path> get_all_items_in_folder(const fs::path& folder) override;

    private:

        /*!
         * \brief Map from filename to its index in the zip folder. Miniz seems to like indexes
         */
        std::unordered_map<std::string, uint32_t> resource_indexes;

        mz_zip_archive zip_archive = {};

        file_tree_node* files = nullptr;

        void build_file_tree();
    };

    /*!
     * \brief Prints out the nodes in a depth-first fashion
     */
    void print_file_tree(const file_tree_node* folder, uint32_t depth);
}


#endif //NOVA_RENDERER_ZIP_FOLDER_ACCESSOR_H
