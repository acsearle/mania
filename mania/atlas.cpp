//
//  atlas.cpp
//  mania
//
//  Created by Antony Searle on 27/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "atlas.hpp"

namespace manic {

atlas::atlas(GLsizei n) : _packer(n), _size(n) {
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


void atlas::commit() {
    _vbo.bind(GL_ARRAY_BUFFER);
    gl::vbo::assign(GL_ARRAY_BUFFER, _vertices, GL_STREAM_DRAW);
    _texture.bind(GL_TEXTURE_2D);
    _vao.bind();
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei) _vertices.size());
    _vertices.clear();
}

void atlas::discard() {
    _vertices.clear();
}

sprite atlas::place(const_matrix_view<pixel> v, gl::vec2 origin) {
    auto tl = _packer.place({(GLsizei) v.columns(), (GLsizei) v.rows()});
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
    s.a.texCoord = tl / (float) _size;
    s.a.color = {255, 255, 255, 255};
    s.b.position = { v.columns() - origin.x, v.rows() - origin.y };
    s.b.texCoord = gl::vec2{ tl.x + v.columns(), tl.y + v.rows() } / _size;
    s.b.color = s.a.color;
    return s;
}

void atlas::release(sprite s) {
    gl::vec<GLint, 2> a = s.a.texCoord * _size;
    gl::vec<GLint, 2> b = s.b.texCoord * _size;
    _packer.release(a, b);
}

} // namespace manic
