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
#include <algorithm>
#include <exception>

#include <json.hpp>
#include <fstream>

#include <easylogging++.h>
#include <glm/glm.hpp>
#include <optional.hpp>

using namespace std::experimental;

/*!
 * \brief Initializes the logging system
 */
void initialize_logging();

namespace nova {
    /*!
     * \brief Calls the function once for every element in the provided container
     *
     * \param container The container to perform an action for each element in
     * \param thingToDo The action to perform for each element in the collection
     */
    template <typename Cont, typename Func>
    void foreach(Cont container, Func thingToDo) {
        std::for_each(std::cbegin(container), std::cend(container), thingToDo);
    }

    template<typename Out>
    void split(const std::string &s, char delim, Out result) {
        std::stringstream ss;
        ss.str(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            *(result++) = item;
        }
    }

    std::vector<std::string> split(const std::string &s, char delim);

    /*!
     * \brief Simple exception to represent that a resouce can not be found
     */
    class resource_not_found : public std::exception {
    public:
        explicit resource_not_found(const std::string& msg);
        const char * what() const noexcept override;
    private:
        std::string message;
    };

    /*!
     * \brief Loads a json data structure from the provided stream
     *
     * A couple assumptions are made here:
     *  - The stream is already open
     *  - The stream will return a valid JSON string
     *
     * If one or both of these is not true, this method will throw an exception
     *
     * Also worth noting: this function will close the stream. Do not try to
     * use it after calling this function of you'll have a bad time
     */
    nlohmann::json load_json_from_stream(std::istream& stream);

    /*!
     * \brief Stream insertion for glm::ivec3
     *
     * \param out The stream to write to
     * \param vec The vector to write
     * \return The written to stream
     */
    el::base::Writer &operator<<(el::base::Writer &out, const glm::ivec3 &vec);

    /*!
     * \brief Stream insertion for glm::vec2
     *
     * \param out The stream to write to
     * \param vec The vector to write
     * \return The written to stream
     */
    el::base::Writer &operator<<(el::base::Writer &out, const glm::vec2 &vec);

    /*!
     * \brief Stream insertion for glm::vec3
     *
     * \param out The stream to write to
     * \param vec The vector to write
     * \return The written to stream
     */
    el::base::Writer &operator<<(el::base::Writer &out, const glm::vec3 &vec);

    /*!
     * \brief Stream insertion for glm::vec4
     *
     * \param out The stream to write to
     * \param vec The vector to write
     * \return The written to stream
     */
    el::base::Writer &operator<<(el::base::Writer &out, const glm::vec4 &vec);

    /*!
     * \brief Stream insertion for glm::vec4
     *
     * \param out The stream to write to
     * \param vec The vector to write
     * \return The written to stream
     */
    template <typename T>
    el::base::Writer &operator<<(el::base::Writer &out, const std::vector<T> &vec) {
        out << "(";
        for(auto& val : vec) {
            out << val << ", ";
        }
        out << ")";

        return out;
    }

    std::string print_color(unsigned int color);

    std::string print_array(int data[], int num_elements);

    /*!
     * \brief Checks if the given value is in the provided json
     * \param key The key to look for
     * \param json_obj The JSON object to look for the key in
     * \param decoder A function that deserializes the value
     * \return An optional that wraps the
     */
    template <typename ValType>
    optional<ValType> get_json_value(const nlohmann::json& json_obj, const std::string key, std::function<ValType(const nlohmann::json&)> decoder) {
        const auto& itr = json_obj.find(key);
        if(itr != json_obj.end()) {
            auto& json_node = json_obj.at(key);
            ValType val = decoder(json_node);
            return optional<ValType>{std::move(val)};
        }

        return optional<ValType>{};
    }

    /*!
     * \brief Checks if the given value is in the provided json
     * \param key The key to look for
     * \param json_obj The JSON object to look for the key in
     * \param decoder A function that deserializes the value
     * \return An optional that wraps the
     */
    template <typename ValType>
    optional<ValType> get_json_value(const nlohmann::json& json_obj, const std::string key) {
        const auto& itr = json_obj.find(key);
        if(itr != json_obj.end()) {
            auto& json_node = json_obj.at(key);
            return optional<ValType>(json_node.get<ValType>());
        }

        return optional<ValType>{};
    }
}

#endif //RENDERER_UTILS_H
