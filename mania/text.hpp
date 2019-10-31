//
//  text.hpp
//  mania
//
//  Created by Antony Searle on 6/3/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef text_hpp
#define text_hpp

#include "atlas.hpp"
#include "table3.hpp"

namespace manic {

struct font {
    
    struct glyph {
        sprite sprite_;
        float advance;
    };
    
    table3<u32, glyph> charmap;
    
    float ascender;
    float descender;
    float height;
    
};
    
// std::pair<table3<u32, std::pair<sprite, float>>, float> build_font(atlas&);
font build_font(atlas&);

}

#endif /* text_hpp */
