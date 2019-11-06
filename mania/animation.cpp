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
        image a = from_png_and_multiply_alpha(v + z);
        // BUG: We have just taken an opaque image and premultiplied alpha,
        // achieving notthing at great expense
        
        i64 i_min = a.columns();
        i64 i_max = 0;
        i64 j_min = a.rows();
        i64 j_max = 0;
        
        
        // We now infer the ALPHA channel from the BLUE channel
        
        // The top left pixel is assumed to be pure background
        double f = 1.0 / from_sRGB(a(0,0).b / 255.0);
        
        for (i64 i = 0; i != a.rows(); ++i)
            for (i64 j = 0; j != a.columns(); ++j) {
                // Check that this really is an opaque image
                assert(a(i, j).a = 255);
                // Map to linear color space and multiply by factor
                float alpha = 1.0f - from_sRGB(a(i, j).b / 255.0f) * f;
                assert(0 <= alpha);
                assert(alpha <= 1.0);
                // Zero out blue channel
                a(i, j).b = 0;
                // Set alpha channel
                a(i, j).a = round(alpha * 255.0);
                if (a(i, j).a) {
                    i_min = std::min(i, i_min);
                    i_max = std::max(i, i_max);
                    j_min = std::min(j, j_min);
                    j_max = std::max(j, j_max);
                }
                // For the red and green channels, antialiasing in proportion
                // to coverage has already done the work of premultiplying
                // alpha.  Thus their (linearized) value should be less than or
                // equal to the alpha.
                assert(from_sRGB(a(i, j).r / 255.0) <= alpha);
                assert(from_sRGB(a(i, j).g / 255.0) <= alpha);
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
