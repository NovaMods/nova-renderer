/*!
 * \author David
 * \date 23-Jun-16.
 */

#ifndef RENDERER_CONFIG_H
#define RENDERER_CONFIG_H

#include <string>
#include <vector>
#include <rapidjson/document.h>

class iconfig_change_listener {
public:
     virtual void on_config_change(config &new_config) = 0;
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
     * \param options_doc The JSON document which holds all the options we want
     */
    config(rapidjson::Document options_doc);

    /*!
     * \brief Registers the given iconfig_change_listener as an Observer
     */
    void register_change_listener(iconfig_change_listener * new_listener);

    /*!
     * \brief Sets the data at the given data path to the provided value
     *
     * \param data_path The path to the data in the JSON document. There's a comple things to know about this:
     * - First, this path MUST be to a node that already exists. You'll get an exception otherwise. I should change this
     * before release
     * - Second, this path uses the period character to delimit nodes. Thus, periods cannot be used in nodes, or this
     * will break. Hopefully I find a way to make this better, but all well
     * \param data The data to set the node to
     */
    void set_data(std::string data_path, int data);

    /*!
     * \brief Sets the data at the given data path to the provided value
     *
     * \param data_path The path to the data in the JSON document. There's a comple things to know about this:
     * - First, this path MUST be to a node that already exists. You'll get an exception otherwise. I should change this
     * before release
     * - Second, this path uses the period character to delimit nodes. Thus, periods cannot be used in nodes, or this
     * will break. Hopefully I find a way to make this better, but all well
     * \param data The data to set the node to
     */
    void set_data(std::string data_path, float data);

    /*!
     * \brief Sets the data at the given data path to the provided value
     *
     * \param data_path The path to the data in the JSON document. There's a comple things to know about this:
     * - First, this path MUST be to a node that already exists. You'll get an exception otherwise. I should change this
     * before release
     * - Second, this path uses the period character to delimit nodes. Thus, periods cannot be used in nodes, or this
     * will break. Hopefully I find a way to make this better, but all well
     * \param data The data to set the node to
     */
    void set_data(std::string data_path, bool data);

    /*!
     * \brief Sets the data at the given data path to the provided value
     *
     * \param data_path The path to the data in the JSON document. There's a comple things to know about this:
     * - First, this path MUST be to a node that already exists. You'll get an exception otherwise. I should change this
     * before release
     * - Second, this path uses the period character to delimit nodes. Thus, periods cannot be used in nodes, or this
     * will break. Hopefully I find a way to make this better, but all well
     * \param data The data to set the node to
     */
    void set_data(std::string data_path, std::string data);

    int get_int(std::string data_path);
    float get_float(std::string data_path);
    bool get_bool(std::string data_path);
    std::string get_string(std::string data_path);

    /*!
     * \brief Updates all the change listeners with the current state of the settings
     *
     * This method is public so that its calling may be delayed
     */
    void update_change_listeners();
private:
    rapidjson::Document options;
    std::vector<iconfig_change_listener *> config_change_listeners;

    rapidjson::Value get_value(const std::string &data_path) const;

    void set_data_impl(const std::string & data_path, void (*set_data_func)(rapidjson::Value &));
};


#endif //RENDERER_CONFIG_H
