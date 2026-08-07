/*
* BSD 3-Clause License
 *
 * Copyright (c) 2026, Christoph Neuhauser
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "WglContext.hpp"

#include <stdexcept>

WglBackend::WglBackend() = default;

WglBackend::~WglBackend() = default;

OglContext* WglBackend::createContextForDevice(uint32_t deviceIndex) {
    auto* context = new WglContext(this);
    if (!context) {
        return nullptr;
    }
    if (!context->initialized) {
        delete context;
        return nullptr;
    }
    if (!context->initializeFunctionTable()) {
        delete context;
        return nullptr;
    }
    return context;
}


WglContext::WglContext(WglBackend* backend) : backend(backend) {
    const char* windowClassName = "wglwindowclass";
    const char* windowName = "wglwindowname";
    WNDCLASS wc{};
    wc.lpfnWndProc = DefWindowProc;
    wc.hInstance = GetModuleHandleA(nullptr);
    wc.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
    wc.lpszClassName = windowClassName;
    wc.style = CS_OWNDC;
    if (!RegisterClass(&wc)) {
        return;
    }
    hWnd = CreateWindowExA(
            0, windowClassName, windowName, WS_OVERLAPPEDWINDOW,
            0, 0, 640, 480, nullptr, nullptr, nullptr, nullptr);
    HDC deviceContext = GetDC(hWnd);

    openglModule = LoadLibrary("opengl32.dll");
    if (!openglModule) {
        throw std::runtime_error("WglContext::WglContext: Could not load opengl32.dll.");
    }
    wglf.wglCreateContext = PFNWGLCREATECONTEXTPROC(GetProcAddress(openglModule, TOSTRING(wglCreateContext)));
    wglf.wglDeleteContext = PFNWGLDELETECONTEXTPROC(GetProcAddress(openglModule, TOSTRING(wglDeleteContext)));
    wglf.wglMakeCurrent = PFNWGLMAKECURRENTPROC(GetProcAddress(openglModule, TOSTRING(wglMakeCurrent)));
    wglf.wglGetProcAddress = PFNWGLGETPROCADDRESSPROC(GetProcAddress(openglModule, TOSTRING(wglGetProcAddress)));
    if (!wglf.wglCreateContext
            || !wglf.wglDeleteContext
            || !wglf.wglMakeCurrent
            || !wglf.wglGetProcAddress) {
        throw std::runtime_error("Error in WglContext::WglContext: At least one function pointer could not be loaded.");
    }

    PIXELFORMATDESCRIPTOR pfd{};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags =
            PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER_DONTCARE | PFD_STEREO_DONTCARE
            | PFD_DEPTH_DONTCARE;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    int pfi = ChoosePixelFormat(deviceContext, &pfd);
    SetPixelFormat(deviceContext, pfi, &pfd);

    glRenderingContext = wglf.wglCreateContext(deviceContext);
    if (!glRenderingContext) {
        throw std::runtime_error("Error in checkDefaultWglContext: wglCreateContext failed.");
    }
    if (!wglf.wglMakeCurrent(deviceContext, glRenderingContext)) {
        throw std::runtime_error("Error in checkDefaultWglContext: wglMakeCurrent failed.");
    }

    initialized = true;
}

WglContext::~WglContext() {
    wglf.wglDeleteContext(glRenderingContext);
    DestroyWindow(hWnd);
    FreeLibrary(openglModule);
}

void* WglContext::getFunctionPointer(const char* functionName) {
    void* p = (void*)wglf.wglGetProcAddress(functionName);
    if (p == nullptr || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1)) {
        p = (void*)GetProcAddress(openglModule, functionName);
    }
    return p;
}
