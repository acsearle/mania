//
//  asset.cpp
//  mania
//
//  Created by Antony Searle on 28/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "asset.hpp"
#include "debug.hpp"
#include "json.hpp"

namespace manic {

sprite load_image(string_view image_name, atlas& atl) {
    image a = from_png_and_multiply_alpha(image_name + ".png");
    vec2 origin = { a.rows() / 2, a.columns() / 2 };
    return atl.place(a, origin);
}

table3<string, sprite> load_asset(string_view asset_name, atlas& atl) {
    
    auto clean_image = [](image& a) {
        for (i64 i = a.rows() - 1; i--;)
            for (i64 j = 0; j != a.columns(); ++j)
                if ((i & 63) && (j & 63))
                    a(i + 1, j).a = std::max(a(i + 1, j).a, a(i, j).a);
    };
    
    image a = from_png_and_multiply_alpha(asset_name + ".png");
    clean_image(a);
    
    auto z = _string_from_file(asset_name + ".json");
    json b = json::from(z);

    ptrdiff_t c = b["tile_size"].as_i64();

    table3<string, sprite> result;
    json const& d = b["names"];
    for (size_t i = 0; i != d.size(); ++i) {
        json const& e = d[i];
        for (size_t j = 0; j != e.size(); ++j) {
            if (!e[j].as_string().empty()) {
            result.insert(e[j].as_string(),
                          atl.place(a.sub(i * c, j * c, c, c),
                                    vec2(0, 0)));
            }
        }
        
    }
    return result;
}

} // namespace manic
