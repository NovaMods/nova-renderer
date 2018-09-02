#include <string>
#include "win32_window.hpp"
#include "../../util/logger.hpp"

#if SUPPORT_DX12

namespace nova {
    win32_window::win32_window(const uint32_t width, const uint32_t height) : size(width, height),
            window_class_name(const_cast<WCHAR *>(L"NovaWindowClass")), window_should_close(false) {
        // Very strongly inspired by GLFW's Win32 variant of createNativeWindow - but GLFW is strictly geared towards
        // OpenGL/Vulkan so I don't want to try and fit it into here

        register_window_class();

        create_window(width, height);

        ShowWindow(window_handle, 1);
        UpdateWindow(window_handle);
    }

    win32_window::~win32_window() {
        unregister_window_class();
    }

    void win32_window::create_window(const uint32_t width, const uint32_t height) {
        DWORD style = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP;
        DWORD extended_style = WS_EX_APPWINDOW | WS_EX_TOPMOST;

        auto* title = const_cast<WCHAR *>(L"Minecraft Nova Renderer");

        window_handle = CreateWindowExW(extended_style, window_class_name, title, style, 100, 100, width, height, nullptr, nullptr, GetModuleHandleW(nullptr), this);
    }

    void win32_window::register_window_class() {
        // Basically the same code as GLFW's `_glfwRegisterWindowClassWin32`
        WNDCLASSEXW window_class = {};
        window_class.cbSize = sizeof(window_class);
        window_class.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        window_class.lpfnWndProc = &window_procedure_wrapper;
        window_class.hInstance = GetModuleHandleW(nullptr);
        window_class.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        window_class.lpszClassName = window_class_name;

        // We will one day have our own window icon, but for now the default icon will work
        window_class.hIcon = static_cast<HICON>(LoadImageW(nullptr, IDI_APPLICATION, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_SHARED));

        window_class_id = RegisterClassExW(&window_class);
        if(window_class_id == 0) {
            std::string windows_err = get_last_windows_error();
            logger::instance.log(FATAL) << "Could not register window class: " << windows_err;

            throw std::runtime_error("Could not register window class");
        }
    }

    void win32_window::unregister_window_class() {
        UnregisterClassW(window_class_name, nullptr);
    }

    LRESULT win32_window::window_procedure_wrapper(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
        win32_window *view;

        if(message == WM_NCCREATE) {
            CREATESTRUCT *cs = (CREATESTRUCT *) lParam;
            view = (win32_window *) cs->lpCreateParams;

            SetLastError(0);
            if(SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR) view) == 0) {
                if(GetLastError() != 0)
                    return FALSE;
            }
        } else {
            view = (win32_window *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
        }

        if(view) {
            return view->window_procedure(message, wParam, lParam);
        }

        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    LRESULT win32_window::window_procedure(UINT message, WPARAM wParam, LPARAM lParam) {
        window_should_close = false;

        // Handle window messages, passing input data to a theoretical input manager
        switch(message) {
            case WM_KEYDOWN:
                // Pressed a key. We should save this somewhere I guess
            case WM_DESTROY:
                // DIE DIE DIE
                window_should_close = true;
                break;
            default:
                return DefWindowProc(window_handle, message, wParam, lParam);
        }

        return DefWindowProc(window_handle, message, wParam, lParam);
    }

    std::string win32_window::get_last_windows_error() {
        WCHAR buffer[1024];
        char message[1024];

        FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_MAX_WIDTH_MASK,
                nullptr, GetLastError() & 0xffff, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                buffer, sizeof(buffer), nullptr);

        WideCharToMultiByte(CP_UTF8, 1, buffer, -1, message, sizeof(message), nullptr, nullptr);

        return std::string(message);
    }

    bool win32_window::should_close() const {
        return window_should_close;
    }

    const glm::uvec2& win32_window::get_size() const {
        return size;
    }

    HWND win32_window::get_window_handle() const {
        return window_handle;
    }
}

#endif
