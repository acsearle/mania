//
//  image.cpp
//  mania
//
//  Created by Antony Searle on 16/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include "image.hpp"
#include <png.h>

namespace manic {
    
    void image::multiply_alpha() {
        for (auto&& row : *this)
            for (auto&& px : row)
                px.rgb = px.rgb * px.a / 255;
    }
    
    void image::divide_alpha() {
        for (auto&& row : *this)
            for (auto&& px : row)
                px.rgb = px.rgb * 255 / px.a;
    }
    
    image image::from_png(const char * filename) {
        png_image a;
        memset(&a, 0, sizeof(a));
        a.version = PNG_IMAGE_VERSION;
        png_image_begin_read_from_file(&a, filename);
        a.format = PNG_FORMAT_RGBA;
        //auto b = malloc(a.width * a.height * 4);
        image c(a.height, a.width);
        //png_image_finish_read(&a, nullptr, b, a.width * 4, nullptr);
        png_image_finish_read(&a, nullptr, c._begin._begin, c._begin._stride * sizeof(pixel), nullptr);
        png_image_free(&a);
        
        //image c;
        //c._allocation = (pixel*) b;
        //c._data = (pixel*) b;
        //c._width = a.width;
        //c._height = a.height;
        //c._stride = a.width;
        //c.multiply_alpha();
        return c;
    }
    
    void image::to_png(const char* filename) {
        png_image a;
        memset(&a, 0, sizeof(a));
        a.format = PNG_FORMAT_RGBA;
        a.height = (png_uint_32) _height;
        a.version =  PNG_IMAGE_VERSION;
        a.width = (png_uint_32) _width;
        png_image_write_to_file(&a, filename, 0, _data, (png_int_32) _stride, nullptr);
        png_image_free(&a);
    }

    void image::crop(ptrdiff_t x, ptrdiff_t y, ptrdiff_t w, ptrdiff_t h) {
        _data += x + y * _stride;
        _width = w;
        _height = h;
    }
    
    void image::halve() {
        ptrdiff_t w = _width / 2;
        ptrdiff_t h = _height / 2;
        pixel* d = _allocation;
        for (ptrdiff_t i = 0; i != h; ++i)
            for (ptrdiff_t j = 0; j != w; ++j)
                *d++ = (  *(_data + i * 2 * _stride + j * 2)
                      + *(_data + (i * 2 + 1) * _stride + j * 2)
                      + *(_data + i * 2 * _stride + j * 2 + 1)
                      + *(_data + (i * 2 + 1) * _stride + j * 2 + 1)) / 4;
        _data = _allocation;
        _width = w;
        _height = h;
        _stride = w;
    }
    
    pixel& image::operator()(ptrdiff_t i, ptrdiff_t j) {
        return *(_data + i + j * _stride);
    }
    
    void image::bevel() {
        for (ptrdiff_t j = 0; j != _height; ++j) {
            double a = sin(j * M_PI / _height);
            a *= a;
            for (ptrdiff_t i = 0; i != _width; ++i) {
                double b = sin(i * M_PI / _width);
                b *= b * a;
                pixel& p = operator()(i, j);
                p.rgb = p.rgb * b;
                p.a = 0;
            }
        }
    }
    
    void image::draw_rect(ptrdiff_t x, ptrdiff_t y, ptrdiff_t width, ptrdiff_t height, pixel c) {
        for (auto j = y; j != y + height; ++j)
            for (auto i = 0; i != x + width; ++i)
                operator()(i, j) = c;
    }
    
    void image::clear(pixel c) {
        draw_rect(0, 0, _width, _height, c);
    }
    
    image image::with_size(ptrdiff_t width, ptrdiff_t height) {
        image a;
        a._allocation = static_cast<pixel*>(malloc(sizeof(pixel) * width * height));
        a._data = a._allocation;
        a._height = height;
        a._stride = width;
        a._width = width;
        return a;
    }
    
    

    
}
