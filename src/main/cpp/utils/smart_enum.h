/*!
 * \brief Defines a smart enum which provides to_string and from_string methods
 *
 * \author ddubois 
 * \date 13-Jan-17.
 */

#ifndef RENDERER_SMART_ENUM_H
#define RENDERER_SMART_ENUM_H

#include <unordered_map>
#include <vector>
#include <algorithm>
#include <string>
#include <sstream>
#include <exception>

#define SMART_ENUM(EnumName, ...)                                   \
class EnumName                                                      \
{                                                                   \
public:                                                             \
    EnumName() : value(0) {}                                        \
    EnumName(int x) : value(x) {}                                   \
public:                                                             \
    enum {__VA_ARGS__};                                             \
private:                                                            \
    static void init_map(std::unordered_map<int, std::string>& tmp) { \
        int val = 0;                                                \
        std::string buf_1, buf_2, str = #__VA_ARGS__;               \
        std::replace(str.begin(), str.end(), '=', ' ');             \
        std::stringstream stream(str);                              \
        std::vector<std::string> strings;                           \
        while(std::getline(stream, buf_1, ',')) {                   \
            strings.push_back(buf_1);                               \
        }                                                           \
        for(auto str : strings) {                                   \
            buf_1.clear(); buf_2.clear();                           \
            std::stringstream local_stream(str);                    \
            local_stream >> buf_1 >> buf_2;                         \
            if(buf_2.size() > 0) {                                  \
                val = atoi(buf_2.c_str());                          \
            }                                                       \
            tmp[val++] = buf_1;                                     \
        }                                                           \
    }                                                               \
    int value;                                                      \
public:                                                             \
    operator int () const { return value; }                         \
    std::string to_string(void) const {                             \
            return to_string(value);                                \
    }                                                               \
    static std::string to_string(int aInt) {                        \
        return name_map()[aInt];                                    \
    }                                                               \
    static EnumName from_string(const std::string& s) {             \
        auto it = std::find_if(name_map().begin(), name_map().end(), [s](const auto& p) { \
            return p.second == s;                                   \
        });                                                         \
        if (it == name_map().end()) {                               \
            /*value not found*/                                     \
            throw EnumName::Exception();                            \
        } else {                                                    \
            return EnumName(it->first);                             \
        }                                                           \
    }                                                               \
    class Exception : public std::exception {};                     \
    static std::unordered_map<int,std::string>& name_map() {        \
        static std::unordered_map<int,std::string> name_map_0;      \
        if(name_map_0.size() == 0) {                                \
            init_map(name_map_0);                                   \
        }                                                           \
        return name_map_0;                                          \
    }                                                               \
    static std::vector<EnumName> all_values() {                      \
        std::vector<EnumName> x{ __VA_ARGS__ };                     \
        return x;                                                   \
    }                                                               \
    bool operator<(const EnumName a) const { return (int)*this < (int)a; } \
};

#endif //RENDERER_SMART_ENUM_H
