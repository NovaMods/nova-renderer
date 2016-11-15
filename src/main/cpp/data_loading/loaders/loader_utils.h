/*!
 * \brief Defines a literal ton of functions to aid in the loading of resources
 *
 * \author ddubois 
 * \date 05-Sep-16.
 */

#ifndef RENDERER_LOADER_UTILS_H
#define RENDERER_LOADER_UTILS_H

#include <string>

namespace nova {
    namespace model {
        /*!
         * \brief Checks if the given file is a zip file or not
         *
         * \param filename The name of the file to check
         * \return True if the file is a zipfile, false otherwise
         */
        bool is_zip_file(const std::string& filename);
    }
}


#endif //RENDERER_LOADER_UTILS_H
