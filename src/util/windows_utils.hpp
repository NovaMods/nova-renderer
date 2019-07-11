/*!
 * \brief Utility functions for handling Windows
 *
 * \author ddubois
 * \date 10-Oct-18.
 */

#ifndef NOVA_RENDERER_WINDOWS_UTILS_HPP
#define NOVA_RENDERER_WINDOWS_UTILS_HPP

#include <EASTL/string.h>

/*!
 * \brief Converts a string to a wide string because Windows
 *
 * \param s The string to convert
 * \return The converted string
 */
eastl::wstring s2ws(const eastl::string& s);

/*!
 * \brief Retrieves the most recent Windows error and returns it to the user
 * \return The error string of the most recent Windows error
 */
eastl::string get_last_windows_error();

#endif // NOVA_RENDERER_WINDOWS_UTILS_HPP
