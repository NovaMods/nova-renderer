/*!
 * \author ddubois 
 * \date 10-Oct-18.
 */

#include "windows_utils.hpp"
#include <winnls.h>
#include <string>

std::wstring s2ws(const std::string& s) {
    // from https://stackoverflow.com/questions/27220/how-to-convert-stdstring-to-lpcwstr-in-c-unicode#27296
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
}
