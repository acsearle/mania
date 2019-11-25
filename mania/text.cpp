//
//  text.cpp
//  mania
//
//  Created by Antony Searle on 6/3/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include <ft2build.h>
#include FT_FREETYPE_H

#include "debug.hpp"
#include "text.hpp"

#include "atlas.hpp"
#include "image.hpp"
#include "vec.hpp"

namespace manic {

// Apply a simple pixel-scale drop-shadow to existing artwork

image apply_shadow(const_matrix_view<GLubyte> x) {
    // We get an alpha map.
    //
    // We want the following drop-shadow:
    //
    // 0 F 0    1 2 1    1 F 1
    // 0 0 0 -> 2 4 2 -> 2 4 2
    // 0 0 0    1 2 1    1 2 1
    
    
    double k[3][3] = {
        { 0.0625, 0.125, 0.0625 },
        { 0.125, 0.25, 0.125 },
        { 0.0625, 0.125, 0.0625 }};

    
    matrix<double> a(x.rows() + 2, x.columns() + 2);
    matrix<GLubyte> b(x.rows() + 4, x.columns() + 4);
    b.sub(2, 2, x.rows(), x.columns()) = x;

    // Compute offset filter
    for (i64 i = 0; i != a.rows(); ++i)
        for (i64 j = 0; j != a.columns(); ++j)
            for (i64 u = 0; u != 3; ++u)
                for (i64 v = 0; v != 3; ++v)
                    a(i, j) += k[u][v] * b(i + u, j + v);

    // Blend with offset glyph alpha
    for (i64 i = 0; i != x.rows(); ++i)
        for (i64 j = 0; j != x.columns(); ++j)
            (a(i, j + 1) *= (1.0 - (x(i, j) / 255.0))) += x(i, j);
    
    // Copy alpha into final result
    image c(a.rows(), a.columns());
    for (i64 i = 0; i != c.rows(); ++i)
        for (i64 j = 0; j != c.columns(); ++j)
            c(i, j).a = round(a(i, j));
    
    // Infer color for final result
    for (i64 i = 0; i != x.rows(); ++i)
        for (i64 j = 0; j != x.columns(); ++j) {
            u8 d = std::round(to_sRGB(x(i, j) / 255.0) * 255.0);
            c(i, j + 1).rgb = {d, d, d};
        }
            
          
    return c;
    
}



// short build_font(atlas2<unsigned long>& font_atlas, table3<unsigned long, float>& advances) {
font build_font(atlas& atl) {
    
    font result;
    
    FT_Library ft;
    FT_Error e = FT_Init_FreeType(&ft);
    assert(!e);
    
    FT_Face face;
    e = FT_New_Face(ft,
                    "/Users/acsearle/Downloads/textures/Futura Medium Condensed.otf",
                    0,
                    &face);
    assert(!e);
    
    FT_Set_Pixel_Sizes(face, 0, 100);
    
    FT_UInt gindex = 0;
    FT_ULong charcode = FT_Get_First_Char(face, &gindex);
    
    image u;
    
    while (gindex) {
        
        // DUMP(charcode);
        
        FT_Load_Glyph(face, gindex, FT_LOAD_RENDER);
        
        auto v = const_matrix_view<GLubyte>(face->glyph->bitmap.buffer,
                                            face->glyph->bitmap.width,
                                            face->glyph->bitmap.pitch,
                                            face->glyph->bitmap.rows);
        
        // Unfortunately we can't tell OpenGL to load one channel into all
        // channels so we have to expand in memory
        // TODO: investigate texture swizzling
        /*
        u.discard_and_resize(v.rows(), v.columns());
        for (ptrdiff_t i = 0; i != v.rows(); ++i)
            for (ptrdiff_t j = 0; j != v.columns(); ++j) {
                u8 a = v(i, j);
                u8 b = std::round(to_sRGB(a / 255.0) * 255.0);
                u(i, j) = pixel(b, b, b, a); // white, premultiplied alpha
            }
         */
        
        //dilate(u);
        
        //matrix<GLubyte> h(v);
        //h = 0;
        //if (h.rows() && h.columns())
        //    h(0,0) = 255;
        
        image u = apply_shadow(v);
        
        sprite s = atl.place(u,
                             vec2(-face->glyph->bitmap_left - 1,
                                      +face->glyph->bitmap_top));
        
        float advance = face->glyph->advance.x * 0.015625f;
        result.charmap.insert((u32) charcode, font::glyph{s, advance});
        
        charcode = FT_Get_Next_Char(face, charcode, &gindex);
    }
    
    result.height = face->size->metrics.height * 0.015625f;
    result.ascender = face->size->metrics.ascender * 0.015625f;
    result.descender = face->size->metrics.descender * 0.015625f;

    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    return result;
}

}
