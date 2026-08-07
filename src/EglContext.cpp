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

#include <iostream>
#include <vector>
#include <set>
#include <stdexcept>

#include "EglContext.hpp"
#include "Util.hpp"

#if defined(__linux__)
#include <dlfcn.h>
#include <unistd.h>
#elif defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#define dlsym GetProcAddress
#elif defined(__APPLE__)
#include <dlfcn.h>
#endif

EglBackend::EglBackend() {
#if defined(__linux__)
    eglHandle = dlopen("libEGL.so", RTLD_NOW | RTLD_LOCAL);
    if (!eglHandle) {
        eglHandle = dlopen("libEGL.so.1", RTLD_NOW | RTLD_LOCAL);
        if (!eglHandle) {
            throw std::runtime_error("EglBackend::EglBackend: Could not load libEGL.so.");
        }
    }
#elif defined(_WIN32)
    eglHandle = LoadLibraryA("EGL.dll");
    if (!eglHandle) {
        throw std::runtime_error("EglBackend::EglBackend: Could not load EGL.dll.");
    }
#endif

    eglf.eglGetProcAddress = PFNEGLGETPROCADDRESSPROC(dlsym(eglHandle, TOSTRING(eglGetProcAddress)));
    eglf.eglGetError = PFNEGLGETERRORPROC(dlsym(eglHandle, TOSTRING(eglGetError)));
    eglf.eglQueryString = PFNEGLQUERYSTRINGPROC(dlsym(eglHandle, TOSTRING(eglQueryString)));
    eglf.eglGetDisplay = PFNEGLGETDISPLAYPROC(dlsym(eglHandle, TOSTRING(eglGetDisplay)));
    eglf.eglInitialize = PFNEGLINITIALIZEPROC(dlsym(eglHandle, TOSTRING(eglInitialize)));
    eglf.eglChooseConfig = PFNEGLCHOOSECONFIGPROC(dlsym(eglHandle, TOSTRING(eglChooseConfig)));
    eglf.eglCreatePbufferSurface = PFNEGLCREATEPBUFFERSURFACEPROC(dlsym(eglHandle, TOSTRING(eglCreatePbufferSurface)));
    eglf.eglBindAPI = PFNEGLBINDAPIPROC(dlsym(eglHandle, TOSTRING(eglBindAPI)));
    eglf.eglCreateContext = PFNEGLCREATECONTEXTPROC(dlsym(eglHandle, TOSTRING(eglCreateContext)));
    eglf.eglDestroySurface = PFNEGLDESTROYSURFACEPROC(dlsym(eglHandle, TOSTRING(eglDestroySurface)));
    eglf.eglDestroyContext = PFNEGLDESTROYCONTEXTPROC(dlsym(eglHandle, TOSTRING(eglDestroyContext)));
    eglf.eglTerminate = PFNEGLTERMINATEPROC(dlsym(eglHandle, TOSTRING(eglTerminate)));
    eglf.eglMakeCurrent = PFNEGLMAKECURRENTPROC(dlsym(eglHandle, TOSTRING(eglMakeCurrent)));

    if (!eglf.eglGetProcAddress
            || !eglf.eglGetError
            || !eglf.eglQueryString
            || !eglf.eglGetDisplay
            || !eglf.eglInitialize
            || !eglf.eglChooseConfig
            || !eglf.eglCreatePbufferSurface
            || !eglf.eglBindAPI
            || !eglf.eglCreateContext
            || !eglf.eglDestroySurface
            || !eglf.eglDestroyContext
            || !eglf.eglTerminate
            || !eglf.eglMakeCurrent) {
        throw std::runtime_error(
                "Error in EglBackend::EglBackend: At least one function pointer could not be loaded.");
    }

    // EXT functions are optional.
    eglf.eglQueryDevicesEXT = PFNEGLQUERYDEVICESEXTPROC(eglf.eglGetProcAddress(TOSTRING(eglQueryDevicesEXT)));
    eglf.eglQueryDeviceStringEXT = PFNEGLQUERYDEVICESTRINGEXTPROC(eglf.eglGetProcAddress(TOSTRING(eglQueryDeviceStringEXT)));
    eglf.eglGetPlatformDisplayEXT = PFNEGLGETPLATFORMDISPLAYEXTPROC(eglf.eglGetProcAddress(TOSTRING(eglGetPlatformDisplayEXT)));
    eglf.eglQueryDeviceBinaryEXT = PFNEGLQUERYDEVICEBINARYEXTPROC(eglf.eglGetProcAddress(TOSTRING(eglQueryDeviceBinaryEXT)));

    if (!eglf.eglQueryDevicesEXT || !eglf.eglQueryDeviceStringEXT
            || !eglf.eglGetPlatformDisplayEXT || !eglf.eglQueryDeviceBinaryEXT) {
        return;
    }

    numEglDevices = 0;
    if (!eglf.eglQueryDevicesEXT(0, nullptr, &numEglDevices)) {
        throw std::runtime_error(
                "Error in OffscreenContextEGL::initialize: eglQueryDevicesEXT failed.");
    }
    eglDevices = new EGLDeviceEXT[numEglDevices];
    if (!eglf.eglQueryDevicesEXT(numEglDevices, eglDevices, &numEglDevices)) {
        throw std::runtime_error(
                "Error in OffscreenContextEGL::initialize: eglQueryDevicesEXT failed.");
    }
}

EglBackend::~EglBackend() {
    eglf = {};
    if (eglHandle) {
#if defined(__linux__)
        dlclose(eglHandle);
#elif defined(_WIN32)
        FreeLibrary(eglHandle);
#endif
        eglHandle = {};
    }
}

OglContext* EglBackend::createContextForDevice(uint32_t deviceIndex) {
    auto* context = new EglContext(this, deviceIndex);
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



EglContext::EglContext(EglBackend* backend, uint32_t deviceIndex) : backend(backend) {
    auto& eglf = backend->eglf;
    auto& eglDevices = backend->eglDevices;
    const char* deviceExtensions = eglf.eglQueryDeviceStringEXT(eglDevices[deviceIndex], EGL_EXTENSIONS);
    if (!deviceExtensions) {
        std::cerr << "Error in EglContext::EglContext: eglQueryDeviceStringEXT failed." << std::endl;
        return;
    }
    std::string deviceExtensionsString(deviceExtensions);
    std::vector<std::string> deviceExtensionsVector;
    splitStringWhitespace(deviceExtensionsString, deviceExtensionsVector);
    std::set<std::string> deviceExtensionsSet(deviceExtensionsVector.begin(), deviceExtensionsVector.end());
    if (deviceExtensionsSet.find("EGL_EXT_device_query_name") != deviceExtensionsSet.end()) {
        const char* deviceVendor = eglf.eglQueryDeviceStringEXT(eglDevices[deviceIndex], EGL_VENDOR);
        if (deviceVendor) {
            std::cout << "Device EGL vendor: " << deviceVendor << std::endl;
        } else {
            std::cout << "Empty EGL device vendor. Skipping device." << std::endl;
            return;
        }
        const char* deviceRenderer = eglf.eglQueryDeviceStringEXT(eglDevices[deviceIndex], EGL_RENDERER_EXT);
        if (deviceRenderer) {
            std::cout << "Device EGL renderer: " << deviceRenderer << std::endl;
        }
    } else {
        std::cout << "EGL_EXT_device_query_name not supported. Skipping EGL device." << std::endl;
        return;
    }

    if (deviceExtensionsSet.find("EGL_EXT_device_persistent_id") != deviceExtensionsSet.end()) {
        const char* deviceDriverName = eglf.eglQueryDeviceStringEXT(eglDevices[deviceIndex], EGL_DRIVER_NAME_EXT);
        if (deviceDriverName) {
            std::cout << "Device EGL driver: " << deviceDriverName << std::endl;
        }
    }

    if (deviceExtensionsSet.find("EGL_EXT_device_drm") != deviceExtensionsSet.end()) {
        const char* deviceDrmFile = eglf.eglQueryDeviceStringEXT(eglDevices[deviceIndex], EGL_DRM_DEVICE_FILE_EXT);
        if (deviceDrmFile) {
            std::cout << "Device EGL DRM file: " << deviceDrmFile << std::endl;
        }
    }

    if (deviceExtensionsSet.find("EGL_EXT_device_drm_render_node") != deviceExtensionsSet.end()) {
        const char* deviceDrmRenderNodeFile = eglf.eglQueryDeviceStringEXT(
                eglDevices[deviceIndex], EGL_DRM_RENDER_NODE_FILE_EXT);
        if (deviceDrmRenderNodeFile) {
            std::cout << "Device EGL DRM render node file: " << deviceDrmRenderNodeFile << std::endl;
        }
    }

    eglDisplay = eglf.eglGetPlatformDisplayEXT(
            EGL_PLATFORM_DEVICE_EXT, eglDevices[deviceIndex], nullptr);

    if (!eglDisplay) {
        EGLint errorCode = eglf.eglGetError();
        std::cerr << "Error in EglContext::EglContext: eglGetPlatformDisplayEXT failed (error code: "
                  << std::to_string(errorCode) << ")." << std::endl;
        return;
    }

    const char* extensionsDeviceDisplay = eglf.eglQueryString(eglDisplay, EGL_EXTENSIONS);
    if (extensionsDeviceDisplay) {
        std::string extensionsDeviceDisplayString(extensionsDeviceDisplay);
    }

    EGLint major, minor;
    if (!eglf.eglInitialize(eglDisplay, &major, &minor)) {
        std::cerr << "Error in EglContext::EglContext: eglInitialize failed." << std::endl;
        return;
    }
    std::cout << "EGL display version: " << std::to_string(major) << "." << std::to_string(minor) << std::endl;

    const char* displayVendor = eglf.eglQueryString(eglDisplay, EGL_VENDOR);
    std::cout << "EGL display vendor: " << displayVendor << std::endl;

    EGLint numConfigs;
    EGLConfig eglConfig;
    bool resultEglChooseConfig;
    constexpr EGLint configAttributes[] = {
        EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
        EGL_BLUE_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_RED_SIZE, 8,
        EGL_DEPTH_SIZE, 8,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };
    resultEglChooseConfig = eglf.eglChooseConfig(eglDisplay, configAttributes, &eglConfig, 1, &numConfigs);

    if (numConfigs <= 0) {
        throw std::runtime_error("Error in EglContext::EglContext: eglChooseConfig returned 0.");
        return;
    }
    if (!resultEglChooseConfig) {
        throw std::runtime_error("Error in EglContext::EglContext: eglChooseConfig failed.");
        return;
    }

    int pbufferWidth = 32;
    int pbufferHeight = 32;
    static const EGLint pbufferAttributes[] = {
        EGL_WIDTH, pbufferWidth,
        EGL_HEIGHT, pbufferHeight,
        EGL_NONE,
    };
    eglSurface = eglf.eglCreatePbufferSurface(eglDisplay, eglConfig, pbufferAttributes);
    if (!eglSurface) {
        throw std::runtime_error(
                "Error in EglContext::EglContext: eglCreatePbufferSurface failed.");
        if (eglDisplay) {
            if (!eglf.eglTerminate(eglDisplay)) {
                throw std::runtime_error(
                        "Error in EglContext::EglContext: eglTerminate failed.");
            }
            eglDisplay = {};
        }
        return;
    }

    if (!eglf.eglBindAPI(EGL_OPENGL_API)) {
        throw std::runtime_error("Error in EglContext::EglContext: eglBindAPI failed.");
        return;
    }

    EGLint attribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 4,
        EGL_CONTEXT_MINOR_VERSION, 5,
        EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
        EGL_NONE
    };
    eglContext = eglf.eglCreateContext(eglDisplay, eglConfig, EGL_NO_CONTEXT, attribs);
    if (!eglContext) {
        EGLint errorCode = eglf.eglGetError();
        throw std::runtime_error(
                "Error in EglContext::EglContext: eglCreateContext failed (error code: "
                + std::to_string(errorCode) + ").");
        if (eglSurface) {
            if (!eglf.eglDestroySurface(eglDisplay, eglSurface)) {
                throw std::runtime_error(
                        "Error in EglContext::EglContext: eglDestroySurface failed.");
            }
            eglSurface = {};
        }
        if (eglDisplay) {
            if (!eglf.eglTerminate(eglDisplay)) {
                throw std::runtime_error(
                        "Error in EglContext::EglContext: eglTerminate failed.");
            }
            eglDisplay = {};
        }
        return;
    }

    EGLBoolean retVal;
    if (!eglSurface) {
        retVal = eglf.eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, eglContext);
    } else {
        retVal = eglf.eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
    }
    if (!retVal) {
        std::cerr << "Error in EglContext::EglContext: eglMakeCurrent failed." << std::endl;
        return;
    }
    initialized = true;
}

EglContext::~EglContext() {
    auto& eglf = backend->eglf;
    if (eglSurface) {
        if (!eglf.eglDestroySurface(eglDisplay, eglSurface)) {
            std::cerr << "Error in EglContext::~EglContext: eglDestroySurface failed." << std::endl;
        }
        eglSurface = {};
    }
    if (eglContext) {
        if (!eglf.eglDestroyContext(eglDisplay, eglContext)) {
            std::cerr << "Error in EglContext::~EglContext: eglDestroyContext failed." << std::endl;
        }
        eglContext = {};
    }
    if (eglDisplay) {
        if (!eglf.eglTerminate(eglDisplay)) {
            std::cerr << "Error in EglContext::~EglContext: eglTerminate failed." << std::endl;
        }
        eglDisplay = {};
    }
}

void* EglContext::getFunctionPointer(const char* functionName) {
    auto& eglf = backend->eglf;
    if (!eglf.eglGetProcAddress) {
        return nullptr;
    }
    return (void*)eglf.eglGetProcAddress(functionName);
}
