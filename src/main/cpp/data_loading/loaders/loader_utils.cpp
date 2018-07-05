/*!
 * \brief
 *
 * \author ddubois
 * \date 05-Sep-16.
 */

#define MINIZ_HEADER_FILE_ONLY

#include <miniz_zip.h>
#include "loader_utils.h"

namespace nova {
    bool is_zip_file(const std::string &filename) {
        mz_zip_archive dummy_zip_archive = {};
        return (bool) mz_zip_reader_init_file(&dummy_zip_archive, filename.c_str(), 0);
    }
}