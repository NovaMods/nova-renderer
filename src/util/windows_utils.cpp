/*!
 * \author ddubois
 * \date 10-Oct-18.
 */

#include "windows_utils.hpp"
#include "windows.hpp"

eastl::wstring s2ws(const eastl::string& s) {
    const int slength = static_cast<int>(s.length()) + 1;
    const int len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, nullptr, 0);
    wchar_t* buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    eastl::wstring r(buf);
    delete[] buf;
    return r;
}

eastl::string get_last_windows_error() {
    const DWORD errorMessageID = GetLastError();
    if(errorMessageID == 0) {
        return eastl::string(); // No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;
    const size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                       nullptr,
                                       errorMessageID,
                                       MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                                       reinterpret_cast<LPSTR>(&messageBuffer),
                                       0,
                                       nullptr);

    eastl::string message(messageBuffer, size);

    // Free the buffer.
    LocalFree(messageBuffer);

    return message;
}
