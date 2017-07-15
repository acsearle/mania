//
//  renderer.cpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include "renderer.hpp"

#include <cmath>
#include <iostream>
#include <cstdlib>
#include <vector>

#include <OpenGL/gl3.h>

#include "program.hpp"
#include "vao.hpp"
#include "vbo.hpp"
#include "vertex.hpp"
#include "vec.hpp"
#include "mat.hpp"

using namespace gl;
using namespace std;

typedef vec<GLushort, 2> edge;
typedef vec<GLushort, 3> triangle;


class blenderer
: public renderer {
    
    gl::program _program;
    gl::vao _vao;
    gl::vbo _vbo;
    gl::vbo _ebo;
    
    vector<vertex> _vertices;
    vector<edge> _edges;
    vector<triangle> _triangles;
    vector<vec2> _velocities;
    
    vector<double> _lengths;
    vector<double> _areas;
    
    GLsizei _width, _height;
    
    
    
public:
    
    blenderer();
    virtual ~blenderer() = default;
    void resize(GLsizei width, GLsizei height);
    void render();
    
};

std::unique_ptr<renderer> renderer::make() {
    return std::make_unique<blenderer>();
}

double uniform() {
    return rand() / (double) RAND_MAX;
}

float norm(gl::vec2 v) {
    return v[0] * v[0] + v[1] * v[1];
}

gl::vec2 disk() {
    gl::vec2 v;
    do {
        v = vec2(uniform(), uniform()) * 2 - 1;
    } while (norm(v) > 1.0);
    return v;
}

double area(vec2 a, vec2 b, vec2 c) {
    return 0.5 * cross(a - b, b - c);
}

vec2 normal(vec2 a) {
    vec2 b;
    b[0] = a[1];
    b[1] = -a[0];
    return b;
}

vec2 un(vec2 a) {
    vec2 b;
    double c = hypot(a[0], a[1]);
    b[0] = -a[1] / c;
    b[1] = a[0] / c;
    return b;
}

blenderer::blenderer()
: _program("basic") {
    
    glBindAttribLocation(_program, (GLuint) gl::attribute::position, "position");
    glBindAttribLocation(_program, (GLuint) gl::attribute::color, "color");
    _program.link();
    
    _vao.bind();
    _vbo.bind(GL_ARRAY_BUFFER);
    _ebo.bind(GL_ELEMENT_ARRAY_BUFFER);
    
    gl::vertex::bind();

    const int N = 12;

    /*
    for (int i = 0; i != N; ++i) {
        gl::vertex v;
        v.position = disk();
        v.color[0] = uniform();
        v.color[1] = uniform();
        v.color[2] = uniform();
        v.color[3] = 1.0;
        _vertices.push_back(v);
        _velocities.push_back(disk() * 0.01f);
        
        
    }
     */
    
    for (size_t i = 0; i != N; ++i) {
        gl::vertex v;
        v.color[0] = uniform();
        v.color[1] = uniform();
        v.color[2] = uniform();
        v.color[3] = 1;
        v.position[0] = 0.75 * cos(i * 0.5);
        v.position[1] = 0.75 * sin(i * 0.5);
        _vertices.push_back(v);
        v.position[0] = 0.5 * cos(i * 0.5 + 0.25);
        v.position[1] = 0.5* sin(i * 0.5 + 0.25);
        _vertices.push_back(v);
    }
    
    for (size_t i = 0; i != _vertices.size(); ++i) {
        edge e;
        e[1] = i;
        if (i > 1) {
            e[0] = i-2;
            _edges.push_back(e);
        }
        if (i > 0) {
            e[0] = i-1;
            _edges.push_back(e);
        }
    }
    
    for (size_t i = 2; i != _vertices.size(); ++i) {
        triangle t;
        t[0] = i - 2;
        t[1] = i - 1;
        t[2] = i;
        _triangles.push_back(t);
    }
    
    for (size_t i = 0; i != _edges.size(); ++i) {
        auto& e = _edges[i];
        _lengths.push_back(distance(_vertices[e[0]].position, _vertices[e[1]].position));
    }
    
    for (auto&& a : _triangles) {
        auto b = area(_vertices[a[0]].position,
                              _vertices[a[1]].position,
                              _vertices[a[2]].position);
        if (b < 0) {
            b = -b;
            std::swap(a[0], a[1]);
        }
        _areas.push_back(b);
    }
    
    {
        vec2 v;
        v[0] = 0;
        v[1] = 0;
        _velocities.resize(_vertices.size(), v);
    }
    
    {
        auto& c = _vertices[_edges[15][0]].color;
        c[0] = 1.0;
        auto& d = _vertices[_edges[15][1]].color;
        d[0] = 1.0;
    }
    
    
    _program.validate();
    _program.use();
    
    
    
    glPointSize(10.0);
    
}

void blenderer::resize(GLsizei width, GLsizei height) {
    
    _width = width;
    _height = height;
    
}

void blenderer::render() {
    
    glViewport(0, 0, std::min(_width, _height), std::min(_width, _height));
    
    gl::vec2 gravity;
    gravity[0] = 0;
    gravity[1] = -9.8/60.0/60.0/60.0;
    
    static double t = 0;
    t += 0.02;
    

    auto b = normalize(ivec2(1,1));

    std::cout << b << std::endl;
    
    //_lengths[15] = 0.25 + 0.125 * sin(t);
    
    
    for (size_t i = 0; i != _edges.size(); ++i) {
        auto j = _edges[i][0];
        auto k = _edges[i][1];
        vec2 d = _vertices[j].position - _vertices[k].position;
        
        // the stick damps relative velocity along its length
        vec2 common = (_velocities[j] + _velocities[k]) * 0.5f;
        double l = length(d);
        vec2 u = d * (float) (-1.0 / l);
        _velocities[j] -= u * dot(u, _velocities[j] - common);
        _velocities[k] -= u * dot(u, _velocities[k] - common);
        
        
        
        // Stiffest spring produces impulse to return to rest immediately
        
        double f = 0.5 * (l - _lengths[i]) / l;
        _velocities[j] -= (d * (float) f);
        _velocities[k] += (d * (float) f);
        
        // Creep -- rods slowly adapt to their new length
        //(_lengths[i] *= 0.99) += 0.01 * l;
        
    
        
    }
    
     
    
    for (size_t i = 0; i != _triangles.size(); ++i) {
        
        {
            
            auto x0 = _vertices[_triangles[i][0]].position;
            auto x1 = _vertices[_triangles[i][1]].position;
            auto x2 = _vertices[_triangles[i][2]].position;
            
            auto& v0 = _velocities[_triangles[i][0]];
            auto& v1 = _velocities[_triangles[i][1]];
            auto& v2 = _velocities[_triangles[i][2]];
            
            auto a = (v0 + v1 + v2) / 3.0f;
            v0 -= a;
            v1 -= a;
            v2 -= a;
            v0 *= 0.5f;
            v1 *= 0.5f;
            v2 *= 0.5f;
            v0 += a;
            v1 += a;
            v2 += a;
            
            auto w = (cross(v0, x0) + cross(v1, x1) + cross(v1, x1)) / 3.0f;
            
            /*
            v0 = w * normal(x0) / length(x0) + a;
            v1 = w * normal(x1) / length(x1) + a;
            v2 = w * normal(x2) / length(x2) + a;
            */
            
            
            // V = A . X
            // V . X^-1 = A
            /*
            mat2 X;
            X[0] = y1; X[1] = y2;
            mat2 invX = inv(X);
            mat2 V;
            V[0] = u1; V[1] = u2;
            mat2 A = dot(V, invX);
            
            auto curl = A[0][1] - A[1][0];
            auto div = A[0][0] + A[1][1];
            auto shear1 = A[0][1] + A[0][1];
            auto shear2 = A[0][0] - A[1][1];
            
            A[0][0] = 0;
            A[0][1] = curl / 2;
            A[1][0] = -curl / 2;
            A[1][1] = 0;
            V = dot(A, X);
            
            auto d1 = V[0] - u1;
            auto d2 = V[1] - u2;
            v1 += d1 * 0.1f;
            v2 += d2 * 0.1f;
            v0 -= (d1 + d2) * 0.1f;
            
            //std::cout << (V[0] - u1)[0] << std::endl;
             */
            
            
        }
        
        {
        auto a = _vertices[_triangles[i][0]].position;
        auto b = _vertices[_triangles[i][1]].position;
        auto c = _vertices[_triangles[i][2]].position;
        
        auto d = area(a, b, c);
        
        auto e = d - _areas[i];
        
        auto ab = a-b;
        auto bc = b-c;
        auto ca = c-a;
        
        auto f = length(ab) + length(bc) + length(ca);
        
        auto g = e / f * 0.9;
        
        _velocities[_triangles[i][0]] -= (un(ab) + un(ca)) * (float) g;
        _velocities[_triangles[i][1]] -= (un(bc) + un(ab)) * (float) g;
        _velocities[_triangles[i][2]] -= (un(ca) + un(bc)) * (float) g;
        }
        
        
        
    }
    
    for (size_t i = 0; i != _vertices.size(); ++i) {
        
        auto& x = _vertices[i].position;
        auto& v = _velocities[i];
        
        v += gravity;
        x += v;
        
        if (x[0] < -1.0) {
            x[0] = -1.0;
            v[0] = -v[0];
            v *= 0.5;
        }
        
        if (x[0] > 1.0) {
            x[0] = 1.0;
            v[0] = -v[0];
            v *= 0.5;
        }
        
        if (x[1] < -1.0) {
            x[1] = -1.0;
            v[1] = -v[1];
            v *= 0.5;
        }
            
    }
    
    vec4 black(0,0,0,1);
    vec4 white(1,1,1,1);
    

    glClearColor(0.5, 0.5, 0.5, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    gl::vbo::assign(GL_ARRAY_BUFFER, _vertices, GL_STREAM_DRAW);
    
    
    _program.assign("mColor", white);
    gl::vbo::assign(GL_ELEMENT_ARRAY_BUFFER, _triangles, GL_STREAM_DRAW);
    glDrawElements(GL_TRIANGLES, (GLsizei) _triangles.size() * 3, GL_UNSIGNED_SHORT, (void*) 0);


    vbo::assign(GL_ELEMENT_ARRAY_BUFFER, _edges, GL_STATIC_DRAW);
    _program.assign("mColor", black);
    glDrawElements(GL_LINES, (GLsizei) _edges.size() * 2, GL_UNSIGNED_SHORT, (void*) 0);

    
    glDrawArrays(GL_POINTS, 0, (GLsizei) _vertices.size());
    
    
    
    
}
