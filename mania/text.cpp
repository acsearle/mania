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
    
    FT_Set_Pixel_Sizes(face, 0, 24);
    
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
        u.discard_and_resize(v.rows(), v.columns());
        for (ptrdiff_t i = 0; i != v.rows(); ++i)
            for (ptrdiff_t j = 0; j != v.columns(); ++j) {
                u8 a = v(i, j);
                u(i, j) = pixel(a, a, a, a); // white, premultiplied alpha
            }
        
        //dilate(u);
        
        sprite s = atl.place(u,
                             vec2(-face->glyph->bitmap_left,
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
