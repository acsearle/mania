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
    
short build_font(atlas2<unsigned long>& font_atlas, table3<unsigned long, float>& advances);

}

#endif /* text_hpp */
