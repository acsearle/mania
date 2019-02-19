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
#include "matrix.hpp"

namespace manic {
    
    using std::exchange;
    using std::move;
    using std::forward;

    struct f8 {
        unsigned char _value;
        explicit f8(int x) {
            _value = std::clamp(x, 0, 255);
        }
    public:
        explicit f8(double x) : _value(std::clamp(std::round(x * 255), 0.0, 255.0)) {}
        operator double() const { return _value / 255.0; }
        f8& operator+=(f8 x) { _value += x._value; return *this; }
        f8& operator-=(f8 x) { _value -= x._value; return *this; }
        f8 operator/(f8 x) { return f8(_value * 255 / x._value); }
        f8 operator*(f8 x) { return f8(_value * x._value / 255); }
        f8& operator/=(f8 x) { _value = _value * 255 / x._value; return *this; }
        f8& operator*=(f8 x) { _value = _value * x._value / 255; return *this; }
        f8 operator+(f8 x) { return f8(_value + x._value); }
        f8 operator-(f8 x) { return f8(_value - x._value); }
    };
    
    

    

    using pixel = gl::vec<unsigned char, 4>;

    /*
    template<typename T>
    class column_vector_view {
        struct column_vector_iterator {
            
        };
    };
     */
    
    using image = matrix<pixel>;
    
    //class image {
        
        // libpng and OpenGL both utilize row-major storage for images.
        // Unfortunately this implies that the coordinate system for images is
        // (row, column) = (y, x)
        
    //public:
        
        /*pixel* _allocation;
        pixel* _data;
    
        ptrdiff_t _width;
        ptrdiff_t _height;
        ptrdiff_t _stride;
        
        image() : _allocation(nullptr), _data(nullptr) {}
        image(image const&) = delete;
        image(image&& r)
        : _allocation(exchange(r._allocation, nullptr))
        , _data(exchange(r._data, nullptr))
        , _width(exchange(r._width, 0))
        , _height(exchange(r._height, 0))
        , _stride(exchange(r._stride, 0)) {
        }
        ~image() { free(_allocation); }
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
         */
        
        image from_png(const char*);
        //static image with_size(ptrdiff_t width, ptrdiff_t height);
        void to_png(image, const char*);
        
        void multiply_alpha(image& a);
        void divide_alpha(image& a);
        
        //void crop(ptrdiff_t x, ptrdiff_t y, ptrdiff_t w, ptrdiff_t h);
        
        void halve(image&);
        
        void bevel(image&);
        
        //pixel& operator()(ptrdiff_t i, ptrdiff_t j);
        
        //void draw_rect(ptrdiff_t x, ptrdiff_t y, ptrdiff_t width, ptrdiff_t height, pixel c);
        
        //void clear(pixel c);
        

    //};

}


#endif /* image_hpp */
