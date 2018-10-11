/*!
 * \brief Utility functions for handling Windows
 *
 * \author ddubois 
 * \date 10-Oct-18.
 */

#ifndef NOVA_RENDERER_WINDOWS_UTILS_HPP
#define NOVA_RENDERER_WINDOWS_UTILS_HPP

#include <string>

/*!
 * \brief Converts a string to a wide string because Windows
 * 
 * \param s The string to convert
 * \return The converted string
 */
std::wstring s2ws(const std::string& s);

#endif //NOVA_RENDERER_WINDOWS_UTILS_HPP
