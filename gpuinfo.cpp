#include "gpuinfo.h"

#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <iomanip>

#include "glad/gl.h"


static void printHeader(const std::string& title, int width) {
    std::cout << "\n" << std::string(width, '=') << "\n";
    int padding = (width - title.length()) / 2;
    std::cout << std::string(padding, ' ') << title << "\n";
    std::cout << std::string(width, '=') << "\n\n";
}

static void printSubHeader(const std::string& title, int width) {
    std::cout << title << "\n";
    std::cout << std::string(title.length(), '-') << "\n";
}

static void printSeparator(int width) {
    std::cout << std::string(width, '-') << "\n";
}

static void printField(const std::string& name, const std::string& value, int width) {
    std::cout << std::left << std::setw(25) << (name + ":") << value << "\n";
}



static std::string getString(GLenum name)
{
    const GLubyte* str = glGetString(name);
    return str ? reinterpret_cast<const char*>(str) : "Unknown";
}

static std::vector<std::string> getExtensions()
{
    std::vector<std::string> extensions;

    // Modern OpenGL (3.0+)
    GLint numExtensions = 0;
    glGetIntegerv(GL_NUM_EXTENSIONS, &numExtensions);

    if (numExtensions > 0 && glGetStringi) {
        for (GLint i = 0; i < numExtensions; i++) {
            const GLubyte* ext = glGetStringi(GL_EXTENSIONS, i);
            if (ext) {
                extensions.emplace_back(reinterpret_cast<const char*>(ext));
            }
        }
    } else {
        // Fallback for older OpenGL
        const GLubyte* extString = glGetString(GL_EXTENSIONS);
        if (extString) {
            std::string str(reinterpret_cast<const char*>(extString));
            size_t pos = 0;
            while ((pos = str.find(' ')) != std::string::npos) {
                extensions.push_back(str.substr(0, pos));
                str.erase(0, pos + 1);
            }
            if (!str.empty()) {
                extensions.push_back(str);
            }
        }
    }

    std::sort(extensions.begin(), extensions.end());
    return extensions;
}


void getInfo(GPUInfo & info)
{
    // Basic info
    info.renderer = getString(GL_RENDERER);
    info.vendor = getString(GL_VENDOR);
    info.version = getString(GL_VERSION);
    info.glslVersion = getString(GL_SHADING_LANGUAGE_VERSION);

    // Extensions
    info.extensions = getExtensions();

    // Capabilities
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &info.maxTextureSize);
    glGetIntegerv(GL_MAX_VIEWPORT_DIMS, info.maxViewportDims);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &info.maxTextureUnits);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &info.maxVertexAttribs);
    glGetIntegerv(GL_MAX_VARYING_FLOATS, &info.maxVaryingFloats);
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_COMPONENTS, &info.maxFragmentUniformComponents);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &info.maxVertexUniformComponents);
}

static void displayExtensionsByCategory(const std::vector<std::string>& extensions, int width)
{
    std::map<std::string, std::vector<std::string>> categories;

    for (const auto& ext : extensions) {
        std::string prefix = "Other";

        if (ext.find("GL_ARB_") == 0) prefix = "ARB (Architecture Review Board)";
        else if (ext.find("GL_EXT_") == 0) prefix = "EXT (Multi-vendor)";
        else if (ext.find("GL_NV_") == 0) prefix = "NV (NVIDIA)";
        else if (ext.find("GL_AMD_") == 0) prefix = "AMD";
        else if (ext.find("GL_ATI_") == 0) prefix = "ATI";
        else if (ext.find("GL_INTEL_") == 0) prefix = "INTEL";
        else if (ext.find("GL_KHR_") == 0) prefix = "KHR (Khronos)";

        categories[prefix].push_back(ext);
    }

    for (const auto& [category, extensions] : categories) {
        printSubHeader(category + " (" + std::to_string(extensions.size()) + ")", width);
        for (const auto& ext : extensions) {
            std::cout << "  - " << ext << "\n";
        }
        std::cout << "\n";
    }
}

static void displayExtensionsCompact(const std::vector<std::string>& extensions, int width)
{
    for (const auto& ext : extensions) {
        std::cout << "  - " << ext << "\n";
    }
}

void displayInfo(const GPUInfo & info, bool showAllExtensions)
{
    const int width = 80;

    printHeader("GPU INFORMATION", width);

    printField("Renderer", info.renderer, width);
    printField("Vendor", info.vendor, width);
    printField("OpenGL Version", info.version, width);
    printField("GLSL Version", info.glslVersion, width);

    printHeader("CAPABILITIES", width);

    printField("Max Texture Size", std::to_string(info.maxTextureSize) + " x " +
               std::to_string(info.maxTextureSize), width);
    printField("Max Viewport", std::to_string(info.maxViewportDims[0]) + " x " +
               std::to_string(info.maxViewportDims[1]), width);
    printField("Max Texture Units", std::to_string(info.maxTextureUnits), width);
    printField("Max Vertex Attributes", std::to_string(info.maxVertexAttribs), width);
    printField("Max Varying Floats", std::to_string(info.maxVaryingFloats), width);
    printField("Max Fragment Uniforms", std::to_string(info.maxFragmentUniformComponents), width);
    printField("Max Vertex Uniforms", std::to_string(info.maxVertexUniformComponents), width);

    printHeader("EXTENSIONS (" + std::to_string(info.extensions.size()) + " total)", width);

    if (showAllExtensions) {
        displayExtensionsByCategory(info.extensions, width);
    } else {
        displayExtensionsCompact(info.extensions, width);
    }
}
