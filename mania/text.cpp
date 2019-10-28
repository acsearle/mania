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


struct font {
    
    FT_Library _library;
    FT_Face _face;
    
    explicit font(char const* filename) {
        FT_Error e;
        e = FT_Init_FreeType(&_library);
        assert(!e);
        e = FT_New_Face(_library, filename, 0, &_face);
        assert(!e);
        FT_Set_Pixel_Sizes(_face, 0, 64);
    }
    
    ~font() {
        FT_Done_Face(_face);
        FT_Done_FreeType(_library);
    }
    
    i32 kerning(u32 left, u32 right) {
        FT_Error e;
        FT_Vector k;
        e = FT_Get_Kerning(_face,
                           FT_Get_Char_Index(_face, left),
                           FT_Get_Char_Index(_face, right),
                           FT_KERNING_DEFAULT,
                           &k);
        assert(!e);
        assert(!(k.x & 0x3F));
        return (i32) (k.x >> 6);
    }
    
    i32 advance(u32 character) {
        FT_Error e;
        e = FT_Load_Char(_face, character, FT_LOAD_DEFAULT);
        assert(!e);
        return (i32) (_face->glyph->advance.x >> 6);
    }
    
};



// short build_font(atlas2<unsigned long>& font_atlas, table3<unsigned long, float>& advances) {
std::pair<table3<u32, std::pair<sprite, float>>, float> build_font(atlas& atl) {
    
    std::pair<table3<u32, std::pair<sprite, float>>, float> result;
    
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
                             gl::vec2(-face->glyph->bitmap_left,
                                      +face->glyph->bitmap_top));
        
        float advance = face->glyph->advance.x * 0.015625f;
        result.first.insert((u32) charcode, std::make_pair(s, advance));
        
        charcode = FT_Get_Next_Char(face, charcode, &gindex);
    }
    
    result.second = face->size->metrics.height * 0.015625f;
    
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    
    return result;
}

}
