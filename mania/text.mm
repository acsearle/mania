//
//  text.cpp
//  mania
//
//  Created by Antony Searle on 6/3/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"

#include <ft2build.h>
#include FT_FREETYPE_H

#pragma clang diagnostic pop


#include "debug.hpp"
#include "text.hpp"

#include "atlas.hpp"
#include "image.hpp"
#include "vec.hpp"

namespace manic {
    
    string path_for_resource(string_view name, string_view ext);


// Apply a simple pixel-scale drop-shadow to existing artwork

image apply_shadow(const_matrix_view<std::uint8_t> x) {
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
    matrix<std::uint8_t> b(x.rows() + 4, x.columns() + 4);
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
            u8 d = std::round(to_sRGB(x(i, j) / 255.0) * 255.0); // <-- replace with table
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
                    path_for_resource("Futura Medium Condensed", "otf").c_str(),
                    0,
                    &face);
    assert(!e);
    
    FT_Set_Pixel_Sizes(face, 0, 32);
    
    FT_UInt gindex = 0;
    FT_ULong charcode = FT_Get_First_Char(face, &gindex);
    
    image u;
    
    while (gindex) {
        
        // DUMP(charcode);
        
        FT_Load_Glyph(face, gindex, FT_LOAD_RENDER);
        
        auto v = const_matrix_view<std::uint8_t>(face->glyph->bitmap.buffer,
                                            face->glyph->bitmap.width,
                                            face->glyph->bitmap.pitch,
                                            face->glyph->bitmap.rows);
        image u = apply_shadow(v);
        
        sprite s = atl.place(u,
                             vec2(-face->glyph->bitmap_left + 1,
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
