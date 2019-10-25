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

struct atlas3 {
    
    gl::texture _texture;
    table3<std::string, sprite> _table;
    
    GLsizei _n;
    
    explicit atlas3(std::string_view asset_name) {
        
        std::string t(asset_name);
        image a = from_png((t + ".png").c_str());
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
        
        
}; // atlas3
    


} // manic


#endif /* atlas_hpp */
