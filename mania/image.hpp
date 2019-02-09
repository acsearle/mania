//
//  image.hpp
//  mania
//
//  Created by Antony Searle on 16/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef image_hpp
#define image_hpp

#include <cstdlib>
#include <utility>

#include "vec.hpp"

namespace manic {

    using std::exchange;
    using std::move;
    using std::forward;

    using pixel = gl::vec<unsigned char, 4>;
    
    class image {
        
    public:
        
        pixel* _data;
    
        ptrdiff_t _width;
        ptrdiff_t _height;
        ptrdiff_t _stride;
        
        image() : _data(nullptr) {}
        image(image const&) = delete;
        image(image&& r)
        : _data(exchange(r._data, nullptr))
        , _width(exchange(r._width, 0))
        , _height(exchange(r._height, 0))
        , _stride(exchange(r._stride, 0)) {
        }
        ~image() { free(_data); }
        image& operator=(image const&) = delete;
        image& operator=(image&& r) {
            image(move(r)).swap(*this);
            return *this;
        }
        void swap(image& r) {
            using std::swap;
            swap(_data, r._data);
            swap(_width, r._width);
            swap(_height, r._height);
            swap(_stride, r._stride);
        }
        
        static image from_png(const char*);
        
        void multiply_alpha();

    };

}


#endif /* image_hpp */
