//
//  animation.hpp
//  mania
//
//  Created by Antony Searle on 2/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef animation_hpp
#define animation_hpp

#include "atlas.hpp"

namespace manic {

vector<sprite> load_animation(atlas& atl, string_view v, vec2 delta);

} // namespace manic

#endif /* animation_hpp */
