/*
 See LICENSE folder for this sample’s licensing information.
 
 Abstract:
 Metal shaders used for this sample
 */

#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

// Include header shared between this Metal shader code and C code executing Metal API commands
#include "MyShaderTypes.h"

// Vertex shader outputs and per-fragment inputs
struct RasterizerData
{
    float4 clipSpacePosition [[position]];
    float2 texCoord;
    float4 color;
};

vertex RasterizerData
vertexShader(uint vertexID [[ vertex_id ]],
             constant MyVertex *vertexArray [[ buffer(MyVertexInputIndexVertices) ]],
             constant MyUniforms &uniforms  [[ buffer(MyVertexInputIndexUniforms) ]])

{
    RasterizerData out;
    
    float2 pixelSpacePosition = vertexArray[vertexID].position.xy;
    
    // Scale the vertex by scale factor of the current frame
    pixelSpacePosition *= uniforms.scale;
    
    float2 viewportSize = float2(uniforms.viewportSize);
    
    // Divide the pixel coordinates by half the size of the viewport to convert from positions in
    // pixel space to positions in clip space
    out.clipSpacePosition.xy = pixelSpacePosition / (viewportSize / 2.0);
    out.clipSpacePosition.z = 0.0;
    out.clipSpacePosition.w = 1.0;
    
    out.texCoord = vertexArray[vertexID].texCoord;
    
    out.color = float4(vertexArray[vertexID].color) / 255;
    
    return out;
}

fragment float4
fragmentShader(RasterizerData in [[stage_in]])
{
    return in.color;
}
