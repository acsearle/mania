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
    
    image image::from_png(const char * filename) {
        png_image a;
        memset(&a, 0, sizeof(a));
        a.version = PNG_IMAGE_VERSION;
        png_image_begin_read_from_file(&a, filename);
        a.format = PNG_FORMAT_RGBA;
        auto b = malloc(a.width * a.height * 4);
        png_image_finish_read(&a, nullptr, b, a.width * 4, nullptr);
        png_image_free(&a);
        
        image c;
        c._data = (pixel*) b;
        c._width = a.width;
        c._height = a.height;
        c._stride = a.width;
        c.multiply_alpha();
        return c;
    }
    
    void image::multiply_alpha() {
        for (auto a = _data; a != _data + _height * _stride; a += _stride) {
            for (auto b = a; b != a + _width; ++b) {
                b->rgb = b->rgb * b->a / 255;
            }
        }
    }
    
}
