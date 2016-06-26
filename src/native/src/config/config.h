/*!
 * \author David
 * \date 23-Jun-16.
 */

#ifndef RENDERER_CONFIG_H
#define RENDERER_CONFIG_H

#include <string>
#include <vector>
#include <json.hpp>

class config;

class iconfig_change_listener {
public:
     virtual void on_config_change(nlohmann::json &new_config) = 0;
};

/*!
 * \brief Holds the configuration of Nova
 *
 * Stores values like the graphics settings, performance settings, any settings shaderpacks define, etc. Uses a JSON
 * document as the data model
 */
class config {
public:
    /*!
     * \brief Constructs this config from the given JSON document
     *
     * \param filename The name of the file to load the config from
     */
    config(std::string filename);

    /*!
     * \brief Registers the given iconfig_change_listener as an Observer
     */
    void register_change_listener(iconfig_change_listener * new_listener);

    nlohmann::json & get_options();

    /*!
     * \brief Updates all the change listeners with the current state of the settings
     *
     * This method is public so that whatever changes values can delay calling it. You can set a bunch of options that
     * are pretty computationally intensive to change, the update listeners after all the values are changed
     */
    void update_change_listeners();
private:
    nlohmann::json options;
    std::vector<iconfig_change_listener *> config_change_listeners;
};

#endif //RENDERER_CONFIG_H
