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

#include "Util.hpp"
#include "OglContext.hpp"

#include <iostream>

bool OglContext::initializeFunctionTable() {
    f.glGetString = PFNGLGETSTRINGPROC(getFunctionPointer("glGetString"));
    f.glGetStringi = PFNGLGETSTRINGIPROC(getFunctionPointer("glGetStringi"));
    f.glGetIntegerv = PFNGLGETINTEGERVPROC(getFunctionPointer("glGetIntegerv"));
    f.glGetInteger64v = PFNGLGETINTEGER64VPROC(getFunctionPointer("glGetInteger64v"));
    f.glPixelStorei = PFNGLPIXELSTOREIPROC(getFunctionPointer("glPixelStorei"));

    f.glEnable = PFNGLENABLEPROC(getFunctionPointer("glEnable"));
    f.glDisable = PFNGLDISABLEPROC(getFunctionPointer("glDisable"));
    f.glDebugMessageControl = PFNGLDEBUGMESSAGECONTROLPROC(getFunctionPointer("glDebugMessageControl"));
    f.glDebugMessageCallback = PFNGLDEBUGMESSAGECALLBACKPROC(getFunctionPointer("glDebugMessageCallback"));

    f.glGenTextures = PFNGLGENTEXTURESPROC(getFunctionPointer("glGenTextures"));
    f.glBindTexture = PFNGLBINDTEXTUREPROC(getFunctionPointer("glBindTexture"));
    f.glDeleteTextures = PFNGLDELETETEXTURESPROC(getFunctionPointer("glDeleteTextures"));
    f.glTextureStorage3D = PFNGLTEXTURESTORAGE3DPROC(getFunctionPointer("glTextureStorage3D"));
    f.glTextureSubImage3D = PFNGLTEXTURESUBIMAGE3DPROC(getFunctionPointer("glTextureSubImage3D"));
    f.glTextureParameteri = PFNGLTEXTUREPARAMETERIPROC(getFunctionPointer("glTextureParameteri"));
    f.glClearTexImage = PFNGLCLEARTEXIMAGEPROC(getFunctionPointer("glClearTexImage"));
    f.glGetTextureImage = PFNGLGETTEXTUREIMAGEPROC(getFunctionPointer("glGetTextureImage"));
    f.glTextureView = PFNGLTEXTUREVIEWPROC(getFunctionPointer("glTextureView"));

    std::string versionString = (const char*)f.glGetString(GL_VERSION);
    auto majorVersionNumber = fromString<int>(std::string() + versionString.at(0));
    auto minorVersionNumber = fromString<int>(std::string() + versionString.at(2));
    if (majorVersionNumber < 4 || (majorVersionNumber == 4 && minorVersionNumber < 5)) {
        std::cerr << "Error: App needs at least OpenGL 4.5 (got " << versionString << ")" << std::endl;
        return false;
    }
    return true;
}
