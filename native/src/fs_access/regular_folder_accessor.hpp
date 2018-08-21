/*!
 * \author ddubois 
 * \date 14-Aug-18.
 */

#ifndef NOVA_RENDERER_REGULAR_FOLDER_ACCESSOR_H
#define NOVA_RENDERER_REGULAR_FOLDER_ACCESSOR_H

#include "folder_accessor.hpp"

namespace nova {
    /*!
     * \brief Allows access to resources in a regular folder
     */
    class regular_folder_accessor : public folder_accessor_base {
    public:
        explicit regular_folder_accessor(const std::experimental::filesystem::path &folder);

        bool does_resource_exist(const fs::path& resource_path) override;

        std::vector<uint8_t> read_resource(const fs::path &resource_path) override;
    };
}


#endif //NOVA_RENDERER_REGULAR_FOLDER_ACCESSOR_H
