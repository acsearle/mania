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
    
    
    
    
    void build_font(atlas2<char>& font_atlas, table3<char, float>& advances) {
        
        FT_Library ft;
        FT_Error e = FT_Init_FreeType(&ft);
        assert(!e);
        
        FT_Face face;
        e = FT_New_Face(ft,
                        "/Users/acsearle/Downloads/textures/Futura Medium Condensed.otf",
                        0,
                        &face);
        assert(!e);
        
        FT_Set_Pixel_Sizes(face, 0, 128);
        
        for (FT_ULong c = 32; c != 127; ++c) {
        
            FT_Load_Char(face, c, FT_LOAD_RENDER);
            
            font_atlas.push(c,
                            const_matrix_view<GLubyte>(face->glyph->bitmap.buffer,
                                                       face->glyph->bitmap.width,
                                                       face->glyph->bitmap.pitch,
                                                       face->glyph->bitmap.rows),
                            face->glyph->bitmap_left,
                            face->glyph->bitmap_top);
            
            advances.insert(c, face->glyph->advance.x * 0.015625f);
            
        }
        
        FT_Done_Face(face);
        FT_Done_FreeType(ft);
        
        DUMP(advances[' ']);
        
    }
    
}
