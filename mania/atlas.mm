//
//  atlas.cpp
//  mania
//
//  Created by Antony Searle on 27/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "atlas.hpp"

namespace manic {

atlas::atlas(GLsizei n, id<MTLDevice> device) : _packer(n), _size(n) {
    //_vao.bind();
    //_vbo.bind(GL_ARRAY_BUFFER);
    //gl::vertex::bind();
    //_texture.bind(GL_TEXTURE_2D);
    //glTexImage2D(GL_TEXTURE_2D, 0,
                 // gl::format<pixel>,
    //             GL_SRGB8_ALPHA8,
    //             n, n,
    //             0,
    //             gl::format<pixel>, gl::type<pixel>,
    //             nullptr);
    
    _buffer = [device newBufferWithLength:sizeof(gl::vertex) * 1024 options:MTLResourceStorageModeShared];
    
    MTLTextureDescriptor *descriptor = [[MTLTextureDescriptor alloc] init];
    descriptor.pixelFormat = MTLPixelFormatBGRA8Unorm_sRGB;
    descriptor.width = n;
    descriptor.height = n;
    _texture = [device newTextureWithDescriptor:descriptor];
}


void atlas::commit(id<MTLRenderCommandEncoder> renderEncoder) {
    //_vbo.bind(GL_ARRAY_BUFFER);
    //gl::vbo::assign(GL_ARRAY_BUFFER, _vertices, GL_STREAM_DRAW);
    //_texture.bind(GL_TEXTURE_2D);
    //_vao.bind();
    //glDrawArrays(GL_TRIANGLES, 0, (GLsizei) _vertices.size());
    auto n = std::min<std::size_t>(_vertices.size(), 1024);
    std::memcpy(_buffer.contents,
                _vertices.data(),
                n * sizeof(gl::vertex));
    _vertices.clear();
    
    [renderEncoder setVertexBuffer:_buffer
                            offset:0
                           atIndex:0 ];
    [renderEncoder drawPrimitives:MTLPrimitiveTypeTriangle vertexStart:0 vertexCount:n];
}

void atlas::discard() {
    _vertices.clear();
}

// Place a sprite within the free space of the atlas

sprite atlas::place(const_matrix_view<pixel> v, vec2 origin) {
    auto tl = _packer.place({(GLsizei) v.columns(), (GLsizei) v.rows()});
    //glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) v.stride());
    //glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) 1);
    //_texture.bind(GL_TEXTURE_2D);
    //glTexSubImage2D(GL_TEXTURE_2D, 0,
    //                tl.x, tl.y,
    //                (GLsizei) v.columns(), (GLsizei) v.rows(),
    //                gl::format<pixel>,
    //                gl::type<pixel>,
    //                v.data());
    [_texture replaceRegion:MTLRegionMake2D(tl.x, tl.y, v.columns(), v.rows())
                mipmapLevel:0
                  withBytes:v.data()
                bytesPerRow:v.stride() * sizeof(pixel)];
    sprite s;
    s.a.position = - origin;
    s.a.texCoord = tl / (float) _size;
    s.a.color = {255, 255, 255, 255};
    s.b.position = { v.columns() - origin.x, v.rows() - origin.y };
    s.b.texCoord = vec2{ tl.x + v.columns(), tl.y + v.rows() } / _size;
    s.b.color = s.a.color;
    return s;
}

// todo:
// The atlas only knows what space is free, not how the used space is used or
// what it represents.  This is kinda cool but for debugging purposes we
// should maybe keep track of what asset is where, in some cold storage
// somewhere


void atlas::release(sprite s) {
    vec<GLint, 2> a = s.a.texCoord * _size;
    vec<GLint, 2> b = s.b.texCoord * _size;
    _packer.release(a, b);
}

} // namespace manic
