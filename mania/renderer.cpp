//
//  renderer.cpp
//  mania
//
//  Created by Antony Searle on 11/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include "renderer.hpp"

#include <cmath>
#include <cstdlib>
#include <vector>

#include <OpenGL/gl3.h>

#include "program.hpp"
#include "vao.hpp"
#include "vbo.hpp"
#include "vertex.hpp"

using namespace gl;
using namespace std;

typedef vec<GLushort, 2> edge;


class blenderer
: public renderer {
    
    gl::program _program;
    gl::vao _vao;
    gl::vbo _vbo;
    gl::vbo _ebo;
    
    vector<vertex> _vertices;
    vector<edge> _edges;
    vector<vec2> _velocities;
    
    vector<double> _lengths;
    
    
    
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
        v[0] = uniform() * 2 - 1;
        v[1] = uniform() * 2 - 1;
    } while (norm(v) > 1.0);
    return v;
}

double distance(const vertex& a, const vertex& b) {
    return hypot(a.position[0] - b.position[0], a.position[1] - b.position[1]);
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
        v.color[0] = 0;
        v.color[1] = 0;
        v.color[2] = 0;
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
    
    for (size_t i = 0; i != _edges.size(); ++i) {
        auto& e = _edges[i];
        _lengths.push_back(distance(_vertices[e[0]], _vertices[e[1]]));
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
    
    vbo::assign(GL_ELEMENT_ARRAY_BUFFER, &*_edges.cbegin(), &*_edges.cend(), GL_STATIC_DRAW);
    
    _program.validate();
    _program.use();
    
    
    
    glPointSize(10.0);
    
}

void blenderer::resize(GLsizei width, GLsizei height) {
    glViewport(0, 0, width, height);
}

void blenderer::render() {
    
    gl::vec2 gravity;
    gravity[0] = 0;
    gravity[1] = -0.0001;
    
    static double t = 0;
    t += 0.02;
    
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
        
        //(_lengths[i] *= 0.9) += 0.1 * l;
        
    
        
    }
    
    for (size_t i = 0; i != _vertices.size(); ++i) {
        
        auto& x = _vertices[i].position;
        auto& v = _velocities[i];
        
        v += gravity;
        x += v;
        
        if (x[0] < -1.0) {
            x[0] = -1.0;
            v[0] = -v[0];
            v[0] *= 0.5;
            v[1] *= 0.5;
        }
        
        if (x[0] > 1.0) {
            x[0] = 1.0;
            v[0] = -v[0];
            v[0] *= 0.5;
            v[1] *= 0.5;
        }
        
        if (x[1] < -1.0) {
            x[1] = -1.0;
            v[1] = -v[1];
            v[1] *= 0.5;
            v[0] *= 0.5;
        }
            
    }

    gl::vbo::assign(GL_ARRAY_BUFFER, &*_vertices.cbegin(), &*_vertices.cend(), GL_STREAM_DRAW);


    glClearColor(0.5, 0.5, 0.5, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glDrawArrays(GL_POINTS, 0, (GLsizei) _vertices.size());
    glDrawElements(GL_LINES, (GLsizei) _edges.size() * 2, GL_UNSIGNED_SHORT, (void*) 0);
    
}
