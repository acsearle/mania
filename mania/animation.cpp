//
//  animation.cpp
//  mania
//
//  Created by Antony Searle on 2/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "animation.hpp"
#include "debug.hpp"

namespace manic {

vector<sprite> load_animation(atlas& atl, string_view v, vec2 delta) {
    
    vector<sprite> result;
    
    for (int k = 0; k != 32; ++k) {
        char z[256];
        sprintf(z, "%02i.png", k);
        image a = from_png(v + z);
        
        i64 i_min = a.columns();
        i64 i_max = 0;
        i64 j_min = a.rows();
        i64 j_max = 0;
        
        
        float b = 255.0f / a(0,0).b;
        for (i64 i = 0; i != a.rows(); ++i)
            for (i64 j = 0; j != a.columns(); ++j) {
                // Normalize blue channel, copy to alpha channel, zero blue channel
                float c = a(i, j).b * b;
                a(i, j).b = 0;
                assert(a(i, j).a = 255);
                a(i, j).a = 255.0 - std::round(c);
                
                // do we need to premultiply alpha??
                a(i, j).r = a(i, j).r * a(i, j).a / 255.0f;
                a(i, j).g = a(i, j).g * a(i, j).a / 255.0f;

                if (a(i, j).a) {
                    i_min = std::min(i, i_min);
                    i_max = std::max(i, i_max);
                    j_min = std::min(j, j_min);
                    j_max = std::max(j, j_max);
                }
            }
        
        ++j_max;
        ++i_max;
        
        auto crop = a.sub(i_min, j_min, i_max - i_min, j_max - j_min);
        
        sprite s = atl.place(crop);
        
        s.a.position.x += j_min + k * delta.x;
        s.a.position.y += i_min + k * delta.y;
        s.b.position.x += j_min + k * delta.x;
        s.b.position.y += i_min + k * delta.y;;

        result.push_back(s);
    
    }
        
    
    return result;
    
}

}
