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
             const device MyVertex *vertexArray [[ buffer(MyVertexInputIndexVertices) ]],
             constant MyUniforms &uniforms  [[ buffer(MyVertexInputIndexUniforms) ]])

{
    RasterizerData out;

    out.clipSpacePosition = float4(uniforms.position_transform
                                   * float3(vertexArray[vertexID].position, 1), 1);
    out.texCoord = vertexArray[vertexID].texCoord;
    out.color = float4(vertexArray[vertexID].color) / 255;
    
    return out;
}

/*
fragment float4
fragmentShader(RasterizerData in [[stage_in]])
{
    return in.color;
}
 */

// Fragment function
fragment float4
fragmentShader(RasterizerData in [[stage_in]],
               texture2d<half> colorTexture [[ texture(AAPLTextureIndexBaseColor) ]])
{
    constexpr sampler textureSampler (mag_filter::nearest,
                                      min_filter::nearest);
    
    // Sample the texture to obtain a color
    const half4 colorSample = colorTexture.sample(textureSampler, in.texCoord);
    
    // return the color of the texture
    return float4(colorSample) * in.color;
}

