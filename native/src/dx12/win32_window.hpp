#include "../platform.hpp"

#if SUPPORT_DX12

#ifndef UNICODE
  #define UNICODE
#endif

#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

// C RunTime Header Files
#include <cstdlib>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <d3d12.h>
#include <dxgi1_2.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

namespace nova {
    /*!
     * \brief A Win32 window for the DX12 variant of Nova
     */
    class win32_window {
    public:
        win32_window(uint32_t width, uint32_t height);
        ~win32_window();

    private:
        HWND handle;
        WCHAR* window_class_name;

        /*!
         * \brief Registers Nova's window class
         *
         * If the window class cannot be registered, this method will log an error and throw a runtime_error
         */
        void register_window_class();

        void unregister_window_class();

        LRESULT window_procedure(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

        /*!
         * \brief Retrieves the most recent Windows error and returns it to the user
         * \return
         */
        std::string get_last_windows_error();

        /*!
         * \brief Creates the actual window, setting the value of handle in the process
         *
         * Precondition: the window class ahs been registered
         *
         * \param width The width of the desired window, in pixels
         * \param height The height of the desired window, in pixels
         */
        void create_window(uint32_t width, uint32_t height);
    };

}

#endif