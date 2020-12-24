//
//  atlas.cpp
//  mania
//
//  Created by Antony Searle on 27/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include <chrono>
#include <thread>

#include "atlas.hpp"
#include "debug.hpp"
#include "MyShaderTypes.h"

namespace manic {

atlas::atlas(std::size_t n, id<MTLDevice> device) : _size(n), _packer(n) {

    MTLTextureDescriptor *descriptor = [[MTLTextureDescriptor alloc] init];
    descriptor.pixelFormat = MTLPixelFormatRGBA8Unorm_sRGB;
    descriptor.width = n;
    descriptor.height = n;
    _texture = [device newTextureWithDescriptor:descriptor];

    _vertices.reserve(65536);
    _buffer = [device newBufferWithLength:sizeof(gl::vertex) * _vertices.capacity()
                                  options:MTLResourceStorageModeShared];
    _buffer2 = [device newBufferWithLength:sizeof(gl::vertex) * _vertices.capacity()
                                  options:MTLResourceStorageModeShared];
    _semaphore = dispatch_semaphore_create(2);

}


void atlas::commit(id<MTLRenderCommandEncoder> renderEncoder) {

    dispatch_semaphore_wait(_semaphore, DISPATCH_TIME_FOREVER);

    assert(_buffer.length >= _vertices.size() * sizeof(gl::vertex));
    std::memcpy(_buffer.contents,
                _vertices.data(),
                _vertices.size() * sizeof(gl::vertex));
    [renderEncoder setVertexBuffer:_buffer
                            offset:0
                           atIndex:MyVertexInputIndexVertices];
    [renderEncoder setFragmentTexture:_texture
                              atIndex:AAPLTextureIndexBaseColor];
    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle
                      vertexStart:0
                      vertexCount:_vertices.size()];
    _vertices.clear();
    std::swap(_buffer, _buffer2);

}

void atlas::discard() {
    _vertices.clear();
}

// Place a sprite within the free space of the atlas

sprite atlas::place(const_matrix_view<pixel> v, vec2 origin) {
    auto tl = _packer.place({v.columns(), v.rows()});
    [_texture replaceRegion:MTLRegionMake2D(tl.x, tl.y,
                                            v.columns(), v.rows())
                mipmapLevel:0
                  withBytes:v.data()
                bytesPerRow:v.stride() * sizeof(pixel)];
    sprite s;
    s.a.position = - origin;
    s.a.texCoord = tl / (float) _size;
    s.b.position = { v.columns() - origin.x, v.rows() - origin.y };
    s.b.texCoord = vec2{ tl.x + v.columns(), tl.y + v.rows() } / _size;
    return s;
}

// todo:
// The atlas only knows what space is free, not how the used space is used or
// what it represents.  This is kinda cool but for debugging purposes we
// should maybe keep track of what asset is where, in some cold storage
// somewhere


void atlas::release(sprite s) {
    vec<int, 2> a = s.a.texCoord * _size;
    vec<int, 2> b = s.b.texCoord * _size;
    _packer.release(a, b);
}

} // namespace manic
