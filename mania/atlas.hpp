//
//  atlas.hpp
//  mania
//
//  Created by Antony Searle on 10/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef atlas_hpp
#define atlas_hpp

//#define GL_SILENCE_DEPRECATION

#include <vector>

#import <Metal/Metal.h>

#include "const_matrix_view.hpp"
#include "image.hpp"
#include "packer.hpp"
//#include "texture.hpp"
//#include "vao.hpp"
//#include "vbo.hpp"
#include "vertex.hpp"



namespace manic {


// sprite has to store texture rect and screen-space rect, so 8x floats
// is minimal for full generality.  We store in the format that is closest
// to the vertices that will be emitted - just add the offset and construct
// the opposite corners.
struct sprite {
    gl::subvert a;
    gl::subvert b;
};
    
    inline sprite operator+(sprite s, vec2 x) {
        s.a.position += x;
        s.b.position += x;
        return s;
    }
    
    inline sprite operator*(sprite s, float k) {
        s.a.position *= k;
        s.b.position *= k;
        return s;
    }


    // atlas bundles together some functionality, some of which should move elsewhere
    //
    // allocate subregions of the texture atlas, returning sprite keys
    //
    // managing the lifetime of the texture
    //
    // provide the interface for gethering sprite draws
    //
    // setting the texture passing the sprites to the GPU
    //
    // managing the round-robin use of GPU vertex buffers
    
struct atlas {

    std::size_t _size;
    packer<std::size_t> _packer;

    std::vector<gl::vertex> _vertices;

    id<MTLTexture> _texture;
    id<MTLBuffer> _buffer;
    id<MTLBuffer> _buffer2;
    dispatch_semaphore_t _semaphore;
    
    atlas(std::size_t n, id<MTLDevice> device);
    
    sprite as_sprite() const {
        return sprite{
            {{0.0f, 0.0f}, {0.0f, 0.0f}},
            {{(float) _size, (float)_size}, {1.0f, 1.0f}},
        };
    }

    void push_sprite(sprite s, vec<std::uint8_t, 4> c = { 255, 255, 255, 255 }) {
        // a - x
        // | \ | => abx ayb
        // y - b
        _vertices.push_back({s.a, c});
        _vertices.push_back({s.b, c});
        _vertices.push_back({{{s.b.position.x, s.a.position.y}, {s.b.texCoord.x, s.a.texCoord.y}}, c});
        _vertices.push_back({s.a, c});
        _vertices.push_back({{{s.a.position.x, s.b.position.y}, {s.a.texCoord.x, s.b.texCoord.y}}, c});
        _vertices.push_back({s.b, c});
    }
        
    void push_quad(gl::vertex v[]) {
        // Draw an arbitrary quad, such as one resulting from a rotation
        _vertices.push_back(v[0]);
        _vertices.push_back(v[1]);
        _vertices.push_back(v[2]);
        _vertices.push_back(v[0]);
        _vertices.push_back(v[2]);
        _vertices.push_back(v[3]);
    }
    
    void commit(id<MTLRenderCommandEncoder> renderEncoder);
    
    void discard();
    
    sprite place(const_matrix_view<pixel>, vec2 origin = { 0, 0 });
    
    void release(sprite);
    
};

} // manic

#endif /* atlas_hpp */
