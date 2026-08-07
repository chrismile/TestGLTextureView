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

#ifndef TESTGLTEXTUREVIEW_OGLCONTEXT_HPP
#define TESTGLTEXTUREVIEW_OGLCONTEXT_HPP

#include <cstdint>
#include <GL/glcorearb.h>
#include <GL/glext.h>

class OglContext;

class OglBackend {
public:
    OglBackend() = default;
    virtual ~OglBackend() = default;
    virtual uint32_t getNumDevices() = 0;
    virtual OglContext* createContextForDevice(uint32_t deviceIndex) = 0;
};

struct GLFunctionTable {
    PFNGLGETSTRINGPROC glGetString;
    PFNGLGETSTRINGIPROC glGetStringi;
    PFNGLGETINTEGERVPROC glGetIntegerv;
    PFNGLGETINTEGER64VPROC glGetInteger64v;
    PFNGLPIXELSTOREIPROC glPixelStorei;

    PFNGLENABLEPROC glEnable;
    PFNGLDISABLEPROC glDisable;
    PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl;
    PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;

    PFNGLGENTEXTURESPROC glGenTextures;
    PFNGLBINDTEXTUREPROC glBindTexture;
    PFNGLDELETETEXTURESPROC glDeleteTextures;
    PFNGLTEXTURESTORAGE3DPROC glTextureStorage3D;
    PFNGLTEXTURESUBIMAGE3DPROC glTextureSubImage3D;
    PFNGLTEXTUREPARAMETERIPROC glTextureParameteri;
    PFNGLCLEARTEXIMAGEPROC glClearTexImage;
    PFNGLGETTEXTUREIMAGEPROC glGetTextureImage;
    PFNGLTEXTUREVIEWPROC glTextureView;
};

class OglContext {
public:
    OglContext() = default;
    virtual ~OglContext() = default;
    GLFunctionTable f{};
    bool initializeFunctionTable();

protected:
    virtual void* getFunctionPointer(const char* functionName) = 0;
    bool initialized = false;
};

#ifndef TOSTRING
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#endif

#endif //TESTGLTEXTUREVIEW_OGLCONTEXT_HPP
