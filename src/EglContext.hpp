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

#ifndef TESTGLTEXTUREVIEW_EGLCONTEXT_HPP
#define TESTGLTEXTUREVIEW_EGLCONTEXT_HPP

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "OglContext.hpp"

#ifdef _WIN32
typedef void* HANDLE;
#endif

struct OffscreenContextEGLFunctionTable {
    PFNEGLGETPROCADDRESSPROC eglGetProcAddress;
    PFNEGLGETERRORPROC eglGetError;
    PFNEGLQUERYSTRINGPROC eglQueryString;
    PFNEGLGETDISPLAYPROC eglGetDisplay;
    PFNEGLINITIALIZEPROC eglInitialize;
    PFNEGLCHOOSECONFIGPROC eglChooseConfig;
    PFNEGLCREATEPBUFFERSURFACEPROC eglCreatePbufferSurface;
    PFNEGLBINDAPIPROC eglBindAPI;
    PFNEGLCREATECONTEXTPROC eglCreateContext;
    PFNEGLDESTROYSURFACEPROC eglDestroySurface;
    PFNEGLDESTROYCONTEXTPROC eglDestroyContext;
    PFNEGLTERMINATEPROC eglTerminate;
    PFNEGLMAKECURRENTPROC eglMakeCurrent;

    // EXT functions are optional.
    PFNEGLQUERYDEVICESEXTPROC eglQueryDevicesEXT;
    PFNEGLQUERYDEVICESTRINGEXTPROC eglQueryDeviceStringEXT;
    PFNEGLGETPLATFORMDISPLAYEXTPROC eglGetPlatformDisplayEXT;
    PFNEGLQUERYDEVICEBINARYEXTPROC eglQueryDeviceBinaryEXT;
};

class EglBackend : public OglBackend {
    friend class EglContext;
public:
    EglBackend();
    ~EglBackend() override;
    uint32_t getNumDevices() override { return uint32_t(numEglDevices); }
    OglContext* createContextForDevice(uint32_t deviceIndex) override;

private:
#ifdef _WIN32
    HMODULE eglHandle = nullptr;
#else
    void* eglHandle = nullptr;
#endif
    OffscreenContextEGLFunctionTable eglf{};
    EGLint numEglDevices = 0;
    EGLDeviceEXT* eglDevices = nullptr;
};

class EglContext : public OglContext {
    friend class EglBackend;
public:
    EglContext(EglBackend* backend, uint32_t deviceIndex);
    ~EglContext() override;

protected:
    void* getFunctionPointer(const char* functionName) override;

private:
    EglBackend* backend;
    EGLContext eglContext{};
    EGLSurface eglSurface{};
    EGLDisplay eglDisplay{};
};

#endif //TESTGLTEXTUREVIEW_EGLCONTEXT_HPP
