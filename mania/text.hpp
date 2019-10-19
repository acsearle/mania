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

namespace manic {
    
void build_font(atlas2<char>& font_atlas, table3<char, float>& advances);

}

#endif /* text_hpp */
