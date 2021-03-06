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
#include "string.hpp"

namespace manic {

// Images are in the preferred format for OpenGL texture upload, alpha-
// premultiplied colors in sRGB color space.
//
// libpng loads sRGB with non-premultiplied alpha.  This is not particularly
// useful for any purpose?, so we are OK to always premultiply the alpha.
// Lots of work for nothing though if it has trivial or absent alpha.
//
// Linear RGB with premultipled alpha is required for correct filtering.
//
// We need another image format, linear RGB represented as f32, for filtering
// and compositing?
//
// Emmissives are their own texture layer so the fact they can't be represented
// in non-premultiplied-alpha pngs is irrelevant.

using pixel = vec<u8, 4>;
using image = matrix<pixel>;

using imagef = matrix<vec4>;

image from_png_and_multiply_alpha(string_view);
void to_png(image const&, char const*);

void multiply_alpha(image& a);

inline f32 from_sRGB(f32 u) {
    return ((u <= 0.04045f)
            ? (u / 12.92f)
            : (std::powf((u + 0.055f) / 1.055f, 2.4f)));
}

extern f32* _from_sRGB_table;

inline f32 from_sRBG(u8 u) {
    return _from_sRGB_table[u];
}

inline vec4 from_sRGB_(pixel p) {
    return vec4(_from_sRGB_table[p.r],
                _from_sRGB_table[p.g],
                _from_sRGB_table[p.b],
                p.a / 255.0f);
}

inline f32 to_sRGB(f32 u) {
    return ((u <= 0.0031308f)
            ? (u * 12.92f)
            : (1.055f * powf(u, 1.0f / 2.4f) - 0.055f));
}

inline vec4 to_sRGB(vec4 v) {
    return vec4(to_sRGB(v.r),
                to_sRGB(v.g),
                to_sRGB(v.b),
                v.a * 255.0f);
}

extern u8 (*_multiply_alpha_table)[256];

inline pixel multiply_alpha(pixel x) {
    return pixel{
        _multiply_alpha_table[x.a][x.r],
        _multiply_alpha_table[x.a][x.g],
        _multiply_alpha_table[x.a][x.g],
        x.a
    };
}

extern u8 (*_divide_alpha_table)[256];

inline pixel divide_alpha(pixel x) {
    return pixel{
        _multiply_alpha_table[x.a][x.r],
        _multiply_alpha_table[x.a][x.g],
        _multiply_alpha_table[x.a][x.g],
        x.a
    };
}


} // namespace manic

#endif /* image_hpp */
