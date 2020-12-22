//
//  MyShaderTypes.h
//  mania
//
//  Created by Antony Searle on 22/12/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#ifndef MyShaderTypes_h
#define MyShaderTypes_h

#include <simd/simd.h>

typedef enum MyVertexInputIndex
{
    MyVertexInputIndexVertices = 0,
    MyVertexInputIndexUniforms = 1,
} MyVertexInputIndex;

typedef struct
{
    // Positions in pixel space (i.e. a value of 100 indicates 100 pixels from the origin/center)
    vector_float2 position;
    
    vector_float2 texCoord;
    
    // 2D texture coordinate
    vector_uchar4 color;
} MyVertex;

typedef struct
{
    float scale;
    vector_uint2 viewportSize;
} MyUniforms;



#endif /* MyShaderTypes_h */


