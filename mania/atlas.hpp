//
//  atlas.hpp
//  mania
//
//  Created by Antony Searle on 10/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef atlas_hpp
#define atlas_hpp

#define GL_SILENCE_DEPRECATION

#include <vector>

#include "const_matrix_view.hpp"
#include "image.hpp"
#include "packer.hpp"
#include "texture.hpp"
#include "vao.hpp"
#include "vbo.hpp"
#include "vertex.hpp"

namespace manic {


// sprite has to store texture rect and screen-space rect, so 8x floats
// is minimal for full generality.  We store in the format that is closest
// to the vertices that will be emitted - just add the offset and construct
// the opposite corners.
struct sprite {
    gl::vertex a;
    gl::vertex b;
};


// atlas holds the OpenGL texture, vertex array and buffer objects needed to
// draw textured triangles, and it provides an interface to pack the texture
// with smaller images such as png assets and font glyphs.
struct atlas {

    gl::texture _texture;
    GLsizei _size;
    gl::vao _vao;
    gl::vbo _vbo;

    std::vector<gl::vertex> _vertices;
    
    packer<GLsizei> _packer;
    
    atlas(GLsizei n = 1024);

    void push_sprite(sprite s) {
        // a - x
        // | \ | => abx axb
        // x - b
        _vertices.push_back(s.a);
        _vertices.push_back(s.b);
        _vertices.push_back({{s.b.position.x, s.a.position.y}, {s.b.texCoord.x, s.a.texCoord.y}, s.a.color});
        _vertices.push_back(s.a);
        _vertices.push_back({{s.a.position.x, s.b.position.y}, {s.a.texCoord.x, s.b.texCoord.y}, s.b.color});
        _vertices.push_back(s.b);
    }
    
    void push_atlas_translated(gl::vec2 v) {
        // Draw the whole texture, typically for debugging
        push_sprite({{{v.x, v.y}, {0, 0}}, {{v.x + _size, v.y + _size}, {1, 1}}});
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
    
    void push_sprite_translated(sprite s, gl::vec2 v) {
        s.a.position += v;
        s.b.position += v;
        push_sprite(s);
    }
        
    void commit();
    
    void discard();
    
    sprite place(const_matrix_view<pixel>, gl::vec2 origin = { 0, 0 });
    
    void release(sprite);
    
};

} // manic

#endif /* atlas_hpp */
