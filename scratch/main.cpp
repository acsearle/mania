//
//  main.cpp
//
//  Created by Antony Searle on 15/12/18.
//  Copyright © 2018 Antony Searle. All rights reserved.
//

#include <iostream>

struct pin {
    pin() = default;
    pin(const pin&) = delete;
    pin(pin&&) = delete;
};

pin get() {
    return pin{};
}

int main( int argc, char** argv) {
    pin p = get();
    printf("ello\n");
    return 0;
}


/*
#include <cassert>
#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H

#define DUMP(X) do { std::cout << #X << " -> " << (X) << std::endl; } while(false)

#include "bit.hpp"

#include "hash.hpp"

void foo() {
    
    FT_Library ft;
    FT_Error e = FT_Init_FreeType(&ft);
    assert(!e);
    
    FT_Face face;
    e = FT_New_Face(ft,
                    "/Users/acsearle/Downloads/textures/Futura Medium Condensed.otf",
                    0,
                    &face);
    assert(!e);
    
    DUMP(face->num_glyphs);
    DUMP(face->num_fixed_sizes);
    DUMP(face->available_sizes);
    
    FT_Set_Pixel_Sizes(face, 0, 32);
    
    FT_Load_Char(face, 'O', FT_LOAD_RENDER);
    
    auto& b = face->glyph->bitmap;
    
    DUMP(face->glyph->bitmap_left);
    DUMP(face->glyph->bitmap_top);
    DUMP(face->glyph->advance.x);
    for (int i = 0; i != b.rows; ++i) {
        for (int j = 0; j != b.width; ++j)
            std::cout <<  (int) b.buffer[i * b.pitch + j];
        std::cout << std::endl;
    }


}

int main(int argc, char** argv) {
    //foo();
    
    std::cout << std::bitset<64>(manic::morton2(12345678)) << std::endl;
    std::cout << manic::morton2_reverse(manic::morton2(12345678)) << std::endl;

    return 0;
}

 */
