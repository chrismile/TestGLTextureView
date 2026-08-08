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
#include <string>
#include <vector>
#include <map>
#ifdef _WIN32
#include "WindowsUtils.hpp"
#include "WglContext.hpp"
#else
#include "EglContext.hpp"
#endif

std::string getErrorSeverityString(GLenum severity) {
    const std::map<GLenum, std::string> severityMap = {
        { GL_DEBUG_SEVERITY_HIGH,         "High" },
        { GL_DEBUG_SEVERITY_MEDIUM,       "Medium" },
        { GL_DEBUG_SEVERITY_LOW,          "Low" },
        { GL_DEBUG_SEVERITY_NOTIFICATION, "Notification" }
    };
    return severityMap.at(severity);
}

std::string getErrorSourceString(GLenum source) {
    const std::map<GLenum, std::string> sourceMap = {
        { GL_DEBUG_SOURCE_API, "OpenGL API" },
        { GL_DEBUG_SOURCE_WINDOW_SYSTEM, "Window System" },
        { GL_DEBUG_SOURCE_SHADER_COMPILER, "Shader Compiler" },
        { GL_DEBUG_SOURCE_THIRD_PARTY, "Third Party" },
        { GL_DEBUG_SOURCE_APPLICATION, "Application" },
        { GL_DEBUG_SOURCE_OTHER, "Other" }
    };
    return sourceMap.at(source);
}

std::string getErrorTypeString(GLenum type) {
    const std::map<GLenum, std::string> typeMap = {
        { GL_DEBUG_TYPE_ERROR, "API Error" },
        { GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR, "Deprecated Behavior" },
        { GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR, "Undefined Behavior" },
        { GL_DEBUG_TYPE_PORTABILITY, "Non-Portable Functionality" },
        { GL_DEBUG_TYPE_PERFORMANCE, "Bad Performance" },
        { GL_DEBUG_TYPE_MARKER, "Command Stream Annotation" },
        { GL_DEBUG_TYPE_PUSH_GROUP, "Group Pushing" },
        { GL_DEBUG_TYPE_POP_GROUP, "Group Popping" },
        { GL_DEBUG_TYPE_OTHER, "Other" }
    };
    return typeMap.at(type);
}

void openglErrorCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message,
        const void* userParam) {
    std::cout << "OpenGL Error:" << std::endl;
    std::cout << "=============" << std::endl;
    std::cout << std::string() + " Message ID: " << std::to_string(id) << std::endl;
    std::cout << std::string() + " Severity: " << getErrorSeverityString(severity) << std::endl;
    std::cout << std::string() + " Type: " << getErrorTypeString(type) << std::endl;
    std::cout << std::string() + " Source: " << getErrorSourceString(source) << std::endl;
    std::cout << std::string() + " Message: " << message << std::endl;
    std::cout << std::endl;
}

std::vector<uint8_t> getTextureData(int texSize, int layerIndex, int mipLevel) {
    int mipLevelSize = texSize / (1 << mipLevel);
    std::vector<uint8_t> textureData(mipLevelSize * mipLevelSize * 2);
    for (int i = 0; i < mipLevelSize * mipLevelSize; i++) {
        textureData[i * 2] = layerIndex;
        textureData[i * 2 + 1] = layerIndex == 0 ? 0 : mipLevel;
    }
    return textureData;
}

void runTests(OglContext* context) {
    auto& f = context->f;

    // Print information about the OpenGL context.
    int n = 0;
    std::string extensionString;
    f.glGetIntegerv(GL_NUM_EXTENSIONS, &n);
    for (int i = 0; i < n; i++) {
        std::string extension = (const char*)f.glGetStringi(GL_EXTENSIONS, i);
        extensionString += extension;
        if (i + 1 < n) {
            extensionString += ", ";
        }
    }
    std::cout << "OpenGL Version: " << (const char*)f.glGetString(GL_VERSION) << std::endl;
    std::cout << "OpenGL Vendor: " << (const char*)f.glGetString(GL_VENDOR) << std::endl;
    std::cout << "OpenGL Renderer: " << (const char*)f.glGetString(GL_RENDERER) << std::endl;
    std::cout << "OpenGL Shading Language Version: "
              << reinterpret_cast<const char*>(f.glGetString(GL_SHADING_LANGUAGE_VERSION)) << std::endl;

    // Set up the debug message callback.
    f.glEnable(GL_DEBUG_OUTPUT);
    f.glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
    f.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_HIGH, 0, nullptr, GL_TRUE);
    f.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_MEDIUM, 0, nullptr, GL_TRUE);
    f.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_LOW, 0, nullptr, GL_TRUE);
    f.glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_TRUE);
    f.glDebugMessageCallback((GLDEBUGPROC)openglErrorCallback, nullptr);

    // Create the 4x4 base texture with 3 layers and 3 mip levels.
    const int texSize = 4;
    const int arrayLayers = 3;
    const int mipLevels = 3;
    GLuint baseTexture{};
    f.glGenTextures(1, &baseTexture);
    f.glBindTexture(GL_TEXTURE_2D_ARRAY, baseTexture);
    f.glTextureStorage3D(baseTexture, mipLevels, GL_RG8UI, texSize, texSize, arrayLayers);
    f.glTextureParameteri(baseTexture, GL_TEXTURE_BASE_LEVEL, 0);
    f.glTextureParameteri(baseTexture, GL_TEXTURE_MAX_LEVEL, mipLevels - 1);
    f.glTextureParameteri(baseTexture, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    f.glTextureParameteri(baseTexture, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    std::vector<uint8_t> clearData(0, 0);
    for (int layerIndex = 0; layerIndex < arrayLayers; layerIndex++) {
        f.glClearTexImage(baseTexture, layerIndex, GL_RG_INTEGER, GL_UNSIGNED_BYTE, clearData.data());
    }

    // Upload some texture data to all mip levels.
    for (int mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
        // Write data to array layer 1 via glTextureSubImage3D.
        int mipLevelSize = texSize / (1 << mipLevel);
        auto textureData = getTextureData(texSize, 1, mipLevel);
        f.glTextureSubImage3D(
                baseTexture, mipLevel, 0, 0, 1, mipLevelSize, mipLevelSize, 1,
                GL_RG_INTEGER, GL_UNSIGNED_BYTE, textureData.data());

        // Write data to array layer 2 via a texture view.
        GLuint viewTexture{};
        f.glGenTextures(1, &viewTexture);
        f.glTextureView(viewTexture, GL_TEXTURE_2D, baseTexture, GL_RG8UI, mipLevel, 1, 2, 1);
        clearData = {2, uint8_t(mipLevel)};
        f.glClearTexImage(viewTexture, 0, GL_RG_INTEGER, GL_UNSIGNED_BYTE, clearData.data());
        f.glDeleteTextures(1, &viewTexture);
    }

    // Check for equality between the read back and expected data.
    bool isDataEqual = true;
    f.glPixelStorei(GL_PACK_ALIGNMENT, 1);
    for (int layerIndex = 0; layerIndex < arrayLayers; layerIndex++) {
        for (int mipLevel = 0; mipLevel < mipLevels; mipLevel++) {
            int mipLevelSize = texSize / (1 << mipLevel);
            auto expectedData = getTextureData(texSize, layerIndex, mipLevel);
            std::vector<uint8_t> readbackData(mipLevelSize * mipLevelSize * 2, 0);
            GLuint viewTexture{};
            f.glGenTextures(1, &viewTexture);
            f.glTextureView(viewTexture, GL_TEXTURE_2D, baseTexture, GL_RG8UI, mipLevel, 1, layerIndex, 1);
            f.glGetTextureImage(
                    viewTexture, 0, GL_RG_INTEGER, GL_UNSIGNED_BYTE,
                    static_cast<GLsizei>(readbackData.size()), readbackData.data());
            if (expectedData != readbackData) {
                isDataEqual = false;
                std::cout << "Mismatch at layer index " << layerIndex << ", mip level " << mipLevel << "." << std::endl;
                std::cout << "Expected: " << std::endl;
                for (int i = 0; i < mipLevelSize * mipLevelSize * 2; i++) {
                    std::cout << int(expectedData[i]);
                    if (i == mipLevelSize * mipLevelSize * 2 - 1) {
                        std::cout << std::endl;
                    } else {
                        std::cout << ", ";
                    }
                }
                std::cout << "Got: " << std::endl;
                for (int i = 0; i < mipLevelSize * mipLevelSize * 2; i++) {
                    std::cout << int(readbackData[i]);
                    if (i == mipLevelSize * mipLevelSize * 2 - 1) {
                        std::cout << std::endl;
                    } else {
                        std::cout << ", ";
                    }
                }
                std::cout << std::endl;
            }
            f.glDeleteTextures(1, &viewTexture);
        }
    }
    if (isDataEqual) {
        std::cout << "Data matches." << std::endl;
    }
    f.glDeleteTextures(1, &baseTexture);
}

int main() {
#ifdef _WIN32
    OglBackend* backend = new WglBackend();
#else
    OglBackend* backend = new EglBackend();
#endif
    auto numDevices = backend->getNumDevices();
    for (uint32_t deviceIdx = 0; deviceIdx < numDevices; deviceIdx++) {
        if (deviceIdx != 0) {
            std::cout << std::endl << "--------------------------------------------" << std::endl << std::endl;
        }
        if (numDevices > 1) {
            std::cout << "Device index: " << deviceIdx << std::endl;
        }
        if (auto* context = backend->createContextForDevice(deviceIdx)) {
            runTests(context);
            delete context;
        }
    }
    delete backend;

#ifdef _WIN32
    pauseIfAppOwnsConsole();
#endif

    return 0;
}
