//
//  atlas.cpp
//  mania
//
//  Created by Antony Searle on 27/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "atlas.hpp"

namespace manic {

table3<std::string, sprite> load_asset(std::string_view asset_name, atlas& atl) {
    
    auto clean_image = [](image& a) {
        for (i64 i = a.rows() - 1; i--;)
            for (i64 j = 0; j != a.columns(); ++j)
                if ((i & 63) && (j & 63))
                    a(i + 1, j).a = std::max(a(i + 1, j).a, a(i, j).a);
    };
    
    
    std::string t(asset_name);
    image a = from_png((t + ".png").c_str());
    clean_image(a);
    
    auto z = _string_from_file(t + ".json");
    json b = json::from(z);
    
    table3<std::string, sprite> result;
        
    ptrdiff_t c = b["tile_size"].as_i64();
    json const& d = b["names"];
    for (size_t i = 0; i != d.size(); ++i) {
        json const& e = d[i];
        for (size_t j = 0; j != e.size(); ++j) {
            result.insert(e[j].as_string(),
                          atl.place(a.sub(i * c, j * c, c, c),
                                    gl::vec2(c / 2.0f, c / 2.0f)));
        }
        
    }
    return result;
}




} // namespace manic
