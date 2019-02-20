#ifndef WIN32_WINDOW_H
#define WIN32_WINDOW_H

#include "../../platform.hpp"
#include "../window.hpp"

#ifdef SUPPORT_DX12

#    ifndef UNICODE
#        define UNICODE
#    endif

#    include <SDKDDKVer.h>

#    include "../../util/windows.hpp"

// C RunTime Header Files
#    include <tchar.h>

#    include <DirectXMath.h>
#    include <dxgi1_2.h>
#    include <glm/glm.hpp>

#    ifdef ERROR
#        undef ERROR
#    endif

namespace nova {
    /*!
     * \brief A Win32 window for the DX12 variant of Nova
     */
    class win32_window : public iwindow {
    public:
        win32_window(uint32_t width, uint32_t height);
        ~win32_window();

        /*
         * Platform-agnostic window interface
         */

        void on_frame_end() override;

        bool should_close() const override;

        glm::uvec2 get_window_size() const override;

        /*
         * End platform-agnostic window interface
         */

        HWND get_window_handle() const;

    private:
        HWND window_handle;
        WCHAR* window_class_name;
        ATOM window_class_id;

        bool window_should_close;

        glm::uvec2 size;

        /*!
         * \brief Registers Nova's window class
         *
         * If the window class cannot be registered, this method will log an error and throw a nova_exception
         */
        void register_window_class();

        void unregister_window_class() const;

        /*!
         * \brief Creates the actual window, setting the value of handle in the process
         *
         * Precondition: the window class ahs been registered
         *
         * \param width The width of the desired window, in pixels
         * \param height The height of the desired window, in pixels
         */
        void create_window(uint32_t width, uint32_t height);

        /*!
         * \brief The actual message handler
         * \param hWnd The handle to.. our window, maybe?
         * \param message The message to (maybe) handle
         * \param wParam
         * \param lParam
         * \return The result of the message handling
         */
        LRESULT window_procedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

        /*!
         * \brief Wraps the instance's Window Procedure, allowing me to use a member function as the window procedure
         * \param hWnd A handle to the window that received the message
         * \param message The message that was received
         * \param wParam
         * \param lParam A wrapper around a pointer to the class instance
         * \return The result of the message handling
         */
        static LRESULT CALLBACK window_procedure_wrapper(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    };
} // namespace nova

#endif
#endif