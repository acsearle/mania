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

    
    
    short build_font(atlas2<unsigned long>& font_atlas, table3<unsigned long, float>& advances) {
        
        FT_Library ft;
        FT_Error e = FT_Init_FreeType(&ft);
        assert(!e);
        
        FT_Face face;
        e = FT_New_Face(ft,
                        "/Users/acsearle/Downloads/textures/Futura Medium Condensed.otf",
                        0,
                        &face);
        assert(!e);
        
        FT_Set_Pixel_Sizes(face, 0, 64);
        // FT_Set_Char_Size(face, 0, 24 * 64, 220, 220);
        
        FT_UInt gindex = 0;
        FT_ULong charcode = FT_Get_First_Char(face, &gindex);
        
        while (gindex) {
            
            DUMP(charcode);
            
            FT_Load_Glyph(face, gindex, FT_LOAD_RENDER);
            
            font_atlas.push(charcode,
                            const_matrix_view<GLubyte>(face->glyph->bitmap.buffer,
                                                       face->glyph->bitmap.width,
                                                       face->glyph->bitmap.pitch,
                                                       face->glyph->bitmap.rows),
                            face->glyph->bitmap_left,
                            face->glyph->bitmap_top);
            
            advances.insert(charcode, face->glyph->advance.x * 0.015625f);
            
            charcode = FT_Get_Next_Char(face, charcode, &gindex);
        }
        
        auto h = face->size->metrics.height;
        
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        
        //DUMP(advances[' ']);
        DUMP(h);
        
        DUMP(font_atlas._used.size());
        
        return h * 0.015625f;
        
    }
    
}
