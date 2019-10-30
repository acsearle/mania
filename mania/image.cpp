//
//  image.cpp
//  mania
//
//  Created by Antony Searle on 16/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//


#include "image.hpp"

#include <numeric>

#include <png.h>

#include "debug.hpp"


namespace manic {
    
    void multiply_alpha(image& img) {
        for (auto&& row : img)
            for (auto&& px : row)
                px.rgb = px.rgb * px.a / 255;
    }
    
    void divide_alpha(image& img) {
        for (auto&& row : img)
            for (auto&& px : row)
                if (px.a)
                    px.rgb = px.rgb * 255 / px.a;
    }
    
    image from_png(string_view v) {
        png_image a;
        memset(&a, 0, sizeof(a));
        a.version = PNG_IMAGE_VERSION;
        if (!png_image_begin_read_from_file(&a, string(v).c_str())) {
            printf("png_image_begin_read_from_file -> \"%s\"\n", a.message);
            abort();
        }
        a.format = PNG_FORMAT_RGBA;
        image c(a.height, a.width);
        if (!png_image_finish_read(&a, nullptr, c.data(), (png_int_32) c.stride() * sizeof(pixel), nullptr)) {
            printf("png_image_finish_read -> \"%s\"\n", a.message);
            abort();
        }
        png_image_free(&a);
        multiply_alpha(c);        
        return c;
    }
    
    void to_png(const image& img, const char* filename) {
        
        image img2(img);
        divide_alpha(img2);
        
        png_image a;
        memset(&a, 0, sizeof(a));
        a.format = PNG_FORMAT_RGBA;
        a.height = (png_uint_32) img2.rows();
        a.version =  PNG_IMAGE_VERSION;
        a.width = (png_uint_32) img2.columns();
        png_image_write_to_file(&a, filename, 0, img2.data(), (png_int_32) img2.stride() * sizeof(pixel), nullptr);
        std::cout << a.message << std::endl;
        png_image_free(&a);
    }
        
    /*
    void halve(image& img) {
        ptrdiff_t w = img.columns() / 2;
        ptrdiff_t h = img.rows() / 2;
        pixel* d = img._store.data();
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
    }*/
    
        /*
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
    }*/
    
    void draw_rect(image& img, ptrdiff_t x, ptrdiff_t y, ptrdiff_t width, ptrdiff_t height, pixel c) {
        for (auto j = y; j != y + height; ++j)
            for (auto i = 0; i != x + width; ++i)
                img(i, j) = c;
    }
    
    /*
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
    */
    
    void blur(matrix_view<pixel> a, const_matrix_view<pixel> b) {
        std::vector<double> c(5, 0.0);
        double* d = c.data() + 2;
        for (ptrdiff_t k = -2; k != 3; ++k)
            c[k + 2] = exp(-0.5 * k * k);
        double e = std::accumulate(c.begin(), c.end(), 0.0);
        for (ptrdiff_t i = 0; i != b.rows(); ++i)
            for (ptrdiff_t j = 0; j != b.columns(); ++j) {
                gl::vec<double, 4> f = 0.0;
                for (ptrdiff_t k = 0; k != 5; ++k)
                    f += b(i, j + k) * d[k];
                a(i, j) = f / e;
            }
    }

bool is_blank(const_matrix_view<pixel> v) {
    for (i64 i = 0; i != v.rows(); ++i) {
        for (i64 j = 0; j != v.columns(); ++j) {
            if (v(i, j).a)
                return false;
        }
    }
    return true;
}

gl::vec<i64, 2> prune(matrix_view<pixel>& v) {
    gl::vec<i64, 2> o(0, 0);
    while (v.rows() && is_blank(v.sub(0, 0, 1, v.columns()))) {
        ++o.y;
        v = v.sub(1, 0, v.rows() - 1, v.columns());
    }
    while (v.rows() && is_blank(v.sub(v.rows() - 1, 0, 1, v.columns())))
        v = v.sub(0, 0, v.rows() - 1, v.columns());
    while (v.columns() && is_blank(v.sub(0, 0, v.rows(), 1))) {
        ++o.x;
        v = v.sub(0, 1, v.rows(), v.columns() - 1);
    }
    while (v.columns() && is_blank(v.sub(0, v.columns() - 1, v.rows(), 1)))
        v = v.sub(0, 0, v.rows(), v.columns() - 1);
    return o;
}

void dilate(image& a) {
    DUMP(a.rows());
    image b(a.rows() + 4, a.columns() + 4);
    b.sub(2, 2, a.rows(), a.columns()) = a;
    swap(a, b);
    double m[3][3] = {{0.6,0.9,0.6},{0.9,1.0,0.9},{0.6,0.9,0.6}};
    b = a.sub(1, 1, a.rows() - 2, a.columns() - 2);
    for (ptrdiff_t i = 0; i != b.rows(); ++i)
        for (ptrdiff_t j = 0; j != b.columns(); ++j) {
            double c = 0;
            for (ptrdiff_t k = 0; k != 3; ++k)
                for (ptrdiff_t l = 0; l != 3; ++l) {
                    c = std::max(c, a(i + k, j + l).a * m[k][l]);
                }
            b(i, j).a = c;
        }
    swap(a, b);
    DUMP(a.rows());
}

pixel compose(pixel a, pixel b) {
    pixel c;
    auto o = 255 - b.a;
    c.r = (a.r * o + b.r * 255) / 255;
    c.g = (a.g * o + b.g * 255) / 255;
    c.b = (a.b * o + b.b * 255) / 255;
    c.a = (a.a * o + b.a * 255) / 255;
    return c;
}

void compose(matrix_view<pixel> background, const_matrix_view<pixel> foreground) {
    for (ptrdiff_t i = 0; i != background.rows(); ++i)
        for (ptrdiff_t j = 0; j != background.columns(); ++j)
            background(i, j) = compose(background(i, j), foreground(i, j));
}

}

