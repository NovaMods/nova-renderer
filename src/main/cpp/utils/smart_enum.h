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
    static void initMap(std::unordered_map<int, std::string>& tmp)  \
    {                                                               \
        using namespace std;                                        \
                                                                    \
        int val = 0;                                                \
        std::string buf_1, buf_2, str = #__VA_ARGS__;               \
        std::replace(str.begin(), str.end(), '=', ' ');             \
        std::stringstream stream(str);                              \
        vector<string> strings;                                     \
        while(std::getline(stream, buf_1, ','))                     \
            strings.push_back(buf_1);                               \
        for(auto str : strings)                                     \
        {                                                           \
            buf_1.clear(); buf_2.clear();                           \
            stringstream localStream(str);                          \
            localStream>> buf_1 >> buf_2;                           \
            if(buf_2.size() > 0)                                    \
                val = atoi(buf_2.c_str());                          \
            tmp[val++] = buf_1;                                     \
        }                                                           \
    }                                                               \
    int value;                                                      \
public:                                                             \
    operator int () const { return value; }                         \
    std::string to_string(void) const {                              \
            return to_string(value);                                 \
    }                                                               \
    static std::string to_string(int aInt)                           \
    {                                                               \
        return nameMap()[aInt];                                     \
    }                                                               \
    static EnumName from_string(const std::string& s)                \
    {                                                               \
        auto it = find_if(nameMap().begin(), nameMap().end(), [s](const std::pair<int,std::string>& p) { \
            return p.second == s;                                   \
        });                                                         \
        if (it == nameMap().end()) {                                \
        /*value not found*/                                         \
            throw EnumName::Exception();                            \
        } else {                                                    \
            return EnumName(it->first);                             \
        }                                                           \
    }                                                               \
    class Exception : public std::exception {};                     \
    static std::unordered_map<int,std::string>& nameMap() {                   \
      static std::unordered_map<int,std::string> nameMap0;                    \
      if (nameMap0.size() ==0) initMap(nameMap0);                   \
      return nameMap0;                                              \
    }                                                               \
    static std::vector<EnumName> allValues() {                      \
      std::vector<EnumName> x{ __VA_ARGS__ };                       \
      return x;                                                     \
    }                                                               \
    bool operator<(const EnumName a) const { return (int)*this < (int)a; } \
};

#endif //RENDERER_SMART_ENUM_H
