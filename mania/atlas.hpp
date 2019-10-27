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
#include "image.hpp"
#include "json.hpp"
#include "vao.hpp"
#include "vbo.hpp"


namespace manic {


// sprite has to store texture rect and screen-space rect, so 8x floats
// is minimal for full generality.  We store in the format that is closest
// to the vertices that will be emitted - just add the offset and construct
// the opposite corners.
struct sprite {
    gl::vertex a;
    gl::vertex b;
};



struct atlas1 {
    
    gl::texture _texture;
    std::vector<sprite> _used;
    GLsizei _n;
    
    packer<GLint> _packer;
    
    
    explicit atlas1(GLsizei n) : _packer(n) {
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

struct atlas3 {
    
    gl::texture _texture;
    table3<std::string, sprite> _table;
    
    GLsizei _n;
    
    explicit atlas3(std::string_view asset_name) {
        
        std::string t(asset_name);
        image a = from_png((t + ".png").c_str());
        clean_image(a);
        FILE* fp = fopen((t + ".json").c_str(), "rb");
        auto z = _string_from_file(fp);
        fclose(fp);
        json b = json::from(z);
        
        _n = (GLsizei) std::max(a.rows(), a.columns());
        _texture.bind(GL_TEXTURE_2D);

        glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) a.stride());
        glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _n, _n, 0, GL_RGBA,
                     GL_UNSIGNED_BYTE, a.data());
        
        ptrdiff_t c = b["tile_size"].as_i64();
        json const& d = b["names"];
        for (size_t i = 0; i != d.size(); ++i) {
            json const& e = d[i];
            for (size_t j = 0; j != e.size(); ++j) {
                // matrix_view<pixel> e = a.sub(i * c, j * c, c, c);
                sprite z;
                float n = _n;
                z.a.position.x = - c / 2;
                z.a.position.y = - c / 2;
                z.a.texCoord.x = (j * c) / n;
                z.a.texCoord.y = (i * c) / n;
                z.b.position.x = + c / 2;
                z.b.position.y = + c / 2;
                z.b.texCoord.x = ((j + 1) * c) / n;
                z.b.texCoord.y = ((i + 1) * c) / n;
                _table.insert(e[j].as_string(), z);
            }
            
        }
    }
        
    sprite const& operator[](std::string_view v) const {
        return _table[v];
    }
    
    bool contains(std::string_view v) const {
        return _table.contains(v);
    }
    
    
    void clean_image(image& a) {
        for (i64 i = a.rows() - 1; i--;)
            for (i64 j = 0; j != a.columns(); ++j)
                if ((i & 63) && (j & 63)) {
                    a(i + 1, j).a = std::max(a(i + 1, j).a, a(i, j).a);
                } else {
                    //a(i, j) = pixel(0,0,0,0);
                }
            
    }
        
        
}; // atlas3


struct atlas_base {

    gl::texture _texture;
    GLsizei _size;
    gl::vao _vao;
    gl::vbo _vbo;

    std::vector<gl::vertex> _vertices;
    
    // We cannot resize a texture without invalidating the texture coordinates
    // of all current sprites
    packer<GLsizei> _packer;
    
    atlas_base(GLsizei n = 1024) : _packer(n), _size(n) {
        _vao.bind();
        _vbo.bind(GL_ARRAY_BUFFER);
        gl::vertex::bind();
        _texture.bind(GL_TEXTURE_2D);
        glTexImage2D(GL_TEXTURE_2D, 0,
                     gl::format<pixel>,
                     n, n,
                     0,
                     gl::format<pixel>, gl::type<pixel>,
                     nullptr);
    }

    void push_sprite(sprite s) {
        // a - x
        // | \ | => abx axb
        // x - b
        _vertices.push_back(s.a);
        _vertices.push_back(s.b);
        _vertices.push_back({{s.b.position.x, s.a.position.y}, {s.b.texCoord.x, s.a.texCoord.y}});
        _vertices.push_back(s.a);
        _vertices.push_back({{s.a.position.x, s.b.position.y}, {s.a.texCoord.x, s.b.texCoord.y}});
        _vertices.push_back(s.b);
    }
    
    void push_texture() {
        // Draw the whole texture, for debugging purposes
        push_sprite({{{0, 0}, {0, 0}}, {{_size, _size}, {1, 1}}});
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
        
    void commit() {
        // Upload the vertices
        _vbo.bind(GL_ARRAY_BUFFER);
        gl::vbo::assign(GL_ARRAY_BUFFER, _vertices, GL_STREAM_DRAW);
        // Bind the texture atlas
        _texture.bind(GL_TEXTURE_2D);
        // Bind the vertex array
        _vao.bind();
        // Draw from the vertex array
        glDrawArrays(GL_TRIANGLES, 0, (GLsizei) _vertices.size());
        // Discard the vertices ready for next cycle
        _vertices.clear();        
    }
    
    void discard() {
        _vertices.clear();
    }
    
    sprite place(const_matrix_view<pixel> v, gl::vec2 origin = { 0, 0 }) {
        auto tl = _packer.place({v.columns(), v.rows()});
        glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) v.stride());
        glPixelStorei(GL_UNPACK_ALIGNMENT, (GLint) 1);
        _texture.bind(GL_TEXTURE_2D);
        glTexSubImage2D(GL_TEXTURE_2D, 0,
                        tl.x, tl.y,
                        (GLsizei) v.columns(), (GLsizei) v.rows(),
                        gl::format<pixel>,
                        gl::type<pixel>,
                        v.data());
        sprite s;
        s.a.position = - origin;
        s.a.texCoord = tl / _size;
        s.b.position = { v.columns() - origin.x, v.rows() - origin.y };
        s.b.texCoord = gl::vec2{ tl.x + v.columns(), tl.y + v.rows() } / _size;
        return s;
    }
    
    void release(sprite s) {
        gl::vec<GLint, 2> a = s.a.texCoord * _size;
        gl::vec<GLint, 2> b = s.b.texCoord * _size;
        _packer.release(a, b);
    }
    
};
    


} // manic


#endif /* atlas_hpp */
