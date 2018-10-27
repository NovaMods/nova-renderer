/******************************************************************************
* The MIT License (MIT)
*
* Copyright (c) 2014 Fredrik Lindh
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
******************************************************************************/

#ifndef RENDER_DOC_MANAGER_H
#define RENDER_DOC_MANAGER_H

#include "renderdoc_app.h"
#include <string>
#include <GLFW/glfw3.h>

#ifdef _WIN32
#include <windows.h>
#endif

class RenderDocManager
{
public:
    RenderDocManager(GLFWwindow *window, std::string render_doc_path, std::string pCapturePath);
    ~RenderDocManager(void);
    void StartFrameCapture();
    void EndFrameCapture();

private:
#ifdef _WIN32
    HINSTANCE m_RenderDocDLL;
    //UINT32 m_SocketPort;
    HWND m_Handle;
#endif
    bool m_CaptureStarted;



    RENDERDOC_API_1_1_1* m_renderDocFns = nullptr;
};

#endif
