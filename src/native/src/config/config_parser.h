/*!
 * \author David
 * \date 21-Jun-16.
 */

#ifndef RENDERER_CONFIG_PARSER_H
#define RENDERER_CONFIG_PARSER_H

#include <string>

/*!
 * \brief Parses the config file, building the configuration data structure
 */
class config_parser {
public:
    /*!
     * \brief Parses the file at the given filename into the configuration data structure
     *
     * \param filename The name of the file to read the config from
     */
    config_parser(std::string filename);

    rapidjson::Document get_config();
private:
    rapidjson::Document config;
};


#endif //RENDERER_CONFIG_PARSER_H
