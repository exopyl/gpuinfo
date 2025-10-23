#pragma once

#include <string>
#include <vector>

struct GPUInfo
{
    std::string renderer;
    std::string vendor;
    std::string version;
    std::string glslVersion;
    std::vector<std::string> extensions;

    // Capabilities
    int maxTextureSize;
    int maxViewportDims[2];
    int maxTextureUnits;
    int maxVertexAttribs;
    int maxVaryingFloats;
    int maxFragmentUniformComponents;
    int maxVertexUniformComponents;
};

extern void getInfo(GPUInfo & info);
extern void displayInfo(const GPUInfo & info, bool showAllExtensions = true);
