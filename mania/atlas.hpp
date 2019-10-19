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
#include "packer.hpp"
#include "texture.hpp"
#include "vertex.hpp"
#include "table3.hpp"

namespace manic {


// sprite has to store texture rect and screen-space rect, so 8x floats
// is minimal for full generality.  We store in the format that is closest
// to the vertices that will be emitted - just add the offset and construct
// the opposite corners.
struct sprite {
    gl::vertex a;
    gl::vertex b;
};



struct atlas {
    
    gl::texture _texture;
    std::vector<sprite> _used;
    GLsizei _n;
    
    packer<GLint> _packer;
    
    
    explicit atlas(GLsizei n) : _packer(n) {
        _n = n;
        _texture.bind(GL_TEXTURE_2D);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, n, n, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, nullptr);
    }
    
    sprite& operator[](isize i) {
        return _used[i];
    }
    
    template<typename T>
    void push(const_matrix_view<T> const& img) {
        
        gl::vec<GLint, 2> xy = _packer.place(gl::vec<GLint, 2>(img.columns(), img.rows()) + 1);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) img.stride());
        glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) 1);
        glTexSubImage2D(GL_TEXTURE_2D, 0, xy.x, xy.y,
                        (GLsizei) img.columns(), (GLsizei) img.rows(), gl::format<T>,
                        gl::type<T>, img.data());
        sprite s;
        s.a.position.x = -img.columns() / 2;
        s.a.position.y = -img.rows() / 2;
        s.a.texCoord.s = xy.x / (float) _n;
        s.a.texCoord.t = xy.y / (float) _n;
        s.b.position.x = s.a.position.x + img.columns();
        s.b.position.y = s.a.position.y + img.rows();
        s.b.texCoord.s = (xy.x + img.columns()) / (float) _n;
        s.b.texCoord.t = (xy.y + img.rows()) / (float) _n;
        _used.push_back(s);
        
    }
    
}; // atlas


template<typename Key>
struct atlas2 {
    
       gl::texture _texture;
       table3<Key, sprite> _used;
       GLsizei _n;
       
       packer<GLint> _packer;
       
       explicit atlas2(GLsizei n) : _packer(n) {
           _n = n;
           _texture.bind(GL_TEXTURE_2D);
           glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, n, n, 0, GL_RGBA,
                        GL_UNSIGNED_BYTE, nullptr);
       }
       
       sprite& operator[](Key const& k) {
           return _used[k];
       }
       
       template<typename T>
       void push(Key const& k, const_matrix_view<T> const& img, int x, int y) {
           
           gl::vec<GLint, 2> xy = _packer.place(gl::vec<GLint, 2>(img.columns(), img.rows()) + 1);
           glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) img.stride());
           glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) 1);
           glTexSubImage2D(GL_TEXTURE_2D, 0, xy.x, xy.y,
                           (GLsizei) img.columns(), (GLsizei) img.rows(), gl::format<T>,
                           gl::type<T>, img.data());
           sprite s;
           s.a.position.x = x;
           s.a.position.y = -y;
           s.a.texCoord.s = xy.x / (float) _n;
           s.a.texCoord.t = xy.y / (float) _n;
           s.b.position.x = s.a.position.x + img.columns();
           s.b.position.y = s.a.position.y + img.rows();
           s.b.texCoord.s = (xy.x + img.columns()) / (float) _n;
           s.b.texCoord.t = (xy.y + img.rows()) / (float) _n;
           _used.insert(k, s);
           
       }
       
}; // atlas2





} // manic


#endif /* atlas_hpp */
