/*!
 * \author ddubois 
 * \date 15-Aug-18.
 */

#ifndef NOVA_RENDERER_ZIP_FOLDER_ACCESSOR_H
#define NOVA_RENDERER_ZIP_FOLDER_ACCESSOR_H

#define MINIZ_HEADER_FILE_ONLY
#include <miniz_zip.h>
#include "folder_accessor.hpp"

namespace fs = std::experimental::filesystem;

namespace nova {
    /*!
     * \brief Allows access to a zip folder
     */
    class zip_folder_accessor : public folder_accessor_base {
    public:
        explicit zip_folder_accessor(const fs::path &folder);
        ~zip_folder_accessor();

        bool does_resource_exist(const fs::path& resource_path) override;

        std::vector<uint8_t> read_resource(const fs::path &resource_path) override;

    private:
        mz_zip_archive zip_archive;

        /*!
         * \brief Map from filename to its index in the zip folder. Miniz seems to like indexes
         */
        std::unordered_map<std::string, uint32_t> resource_indexes;
    };
}


#endif //NOVA_RENDERER_ZIP_FOLDER_ACCESSOR_H
