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
#include <set>
#include <vector>

#include "texture.hpp"
#include "vec.hpp"
#include "image.hpp"
#include "vertex.hpp"

namespace manic {
    
    struct rect {
        gl::vec<GLsizei, 2> xy;
        gl::vec<GLsizei, 2> wh;
        GLsizei area() const { return wh.x * wh.y; }
        struct cmp {
            bool operator()(rect const& a, rect const& b) const {
                return a.area() < b.area();
            }
        };
        rect(ptrdiff_t x, ptrdiff_t y, ptrdiff_t w, ptrdiff_t h) : xy(x,y), wh(w, h) {}
    };
    
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
        std::multiset<rect, rect::cmp> _free;
        std::vector<sprite> _used;
        GLsizei _n;
        
        
        explicit atlas(GLsizei n) {
            _n = n;
            _texture.bind(GL_TEXTURE_2D);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, n, n, 0, GL_RGBA,
                         GL_UNSIGNED_BYTE, nullptr);
            _free.insert(rect(0,0,n,n));
        }
        
        sprite& operator[](ptrdiff_t i) {
            return _used[i];
        }
        
        void push(image const& img) {
            auto i = _free.lower_bound(rect(0,0, img.columns(), img.rows()));
            while ((i != _free.end()) && ((i->wh.x < img.columns()) || (i->wh.y < img.rows())))
                ++i;
            if (i == _free.end()) {
                //assert(false);
                return;
            }
            auto old = *i;
            _free.erase(i);
            glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) img.stride());
            glTexSubImage2D(GL_TEXTURE_2D, 0, old.xy.x, old.xy.y,
                            (GLsizei) img.columns(), (GLsizei) img.rows(), GL_RGBA,
                            GL_UNSIGNED_BYTE, img.data());
            sprite s;
            s.a.position.x = -img.columns() / 2;
            s.a.position.y = -img.rows() / 2;
            s.a.texCoord.s = old.xy.x / (float) _n;
            s.a.texCoord.t = old.xy.y / (float) _n;
            s.b.position.x = s.a.position.x + img.columns();
            s.b.position.y = s.a.position.y + img.rows();
            s.b.texCoord.s = (old.xy.x + img.columns()) / (float) _n;
            s.b.texCoord.t = (old.xy.y + img.rows()) / (float) _n;
            _used.push_back(s);
            ptrdiff_t w = old.wh.x - img.columns();
            ptrdiff_t h = old.wh.y - img.rows();
            
            if (old.wh.x * h >= w * old.wh.y) {
                if (h)
                    _free.insert(rect(old.xy.x, old.xy.y + img.rows(), old.wh.x, h));
                if (w)
                    _free.insert(rect(old.xy.x + img.columns(), old.xy.y, w, img.rows()));
            } else {
                if (w)
                    _free.insert(rect(old.xy.x + img.columns(), old.xy.y, w, old.wh.y));
                if (h)
                    _free.insert(rect(old.xy.x, old.xy.y + img.rows(), img.columns(), h));
            }
        }
    };
    
    
}


#endif /* atlas_hpp */
