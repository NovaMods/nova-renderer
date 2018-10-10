/*!
 * \author ddubois 
 * \date 10-Oct-18.
 */

#include "windows_utils.hpp"
#include <windows.h>
#include <string>

std::wstring s2ws(const std::string& s) {
    const int slength = static_cast<int>(s.length()) + 1;
    const int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, nullptr, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}
