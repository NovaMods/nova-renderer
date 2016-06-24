/*!
 * \author David
 * \date 23-Jun-16.
 */

#include "config.h"
#include "../utils/utils.h"

config::config(rapidjson::Document options_doc) : options(options_doc) {}

void config::register_change_listener(iconfig_change_listener *new_listener) {
    config_change_listeners.push_back(new_listener);
}

void config::set_data(std::string data_path, int data) {
    set_data_impl(data_path, [&](auto node) -> {node.SetInt(data);});
}

void config::set_data(std::string data_path, float data) {
    set_data_impl(data_path, [&](auto node) -> {node.SetFloat(data);});
}

void config::set_data(std::string data_path, bool data) {
    set_data_impl(data_path, [&](auto node) -> {node.SetFloat(data);});
}

void config::set_data(std::string data_path, std::string data) {
    set_data_impl(data_path, [&](auto node) -> {node.SetString(data);});
}

void config::set_data_impl(const std::string &data_path, void (*set_data_func)(rapidjson::Value &)) {
    rapidjson::Value node = get_value(data_path);

    set_data_func(node);

    update_change_listeners();
}

rapidjson::Value config::get_value(const std::string &data_path) const {
    std::vector<std::string> nodes = split_string(data_path, '.');
    rapidjson::Value cur_node = options;

    for(const std::string & s : nodes) {
        cur_node = cur_node[s];
    }
    return cur_node;
}

void config::update_change_listeners() {
    for(const iconfig_change_listener * l : config_change_listeners) {
        l->on_config_change(options);
    }
}

int config::get_int(std::string data_path) {
    rapidjson::Value value = get_value(data_path);
    return value.GetInt();
}

float config::get_float(std::string data_path) {
    rapidjson::Value value = get_value(data_path);
    return value.GetFloat();
}

bool config::get_bool(std::string data_path) {
    rapidjson::Value value = get_value(data_path);
    return value.GetBool();
}

std::string config::get_string(std::string data_path) {
    rapidjson::Value value = get_value(data_path);
    return value.GetString();
}










