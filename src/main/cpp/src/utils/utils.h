/*!
 * \brief Contains a bunch of utility functions which may or may not be actually used anywhere
 *
 * \author David
 * \date 18-May-16.
 */

#ifndef RENDERER_UTILS_H
#define RENDERER_UTILS_H

#include <vector>
#include <string>

/*!
 * \brief Initializes the logging system
 */
void initialize_logging();

/*!
 * \brief Splits a string into a vector of strings
 *
 * \param s The string to split
 * \param delim The character to split on
 *
 * \return The vector containing the split string
 */
std::vector<std::string> split_string(const std::string &s, char delim = ' ');

#endif //RENDERER_UTILS_H
