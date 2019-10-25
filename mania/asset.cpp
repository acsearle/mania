//
//  asset.cpp
//  mania
//
//  Created by Antony Searle on 24/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include <string>
#include <string_view>

#include "asset.hpp"
#include "image.hpp"
#include "json.hpp"

namespace manic {

    void load_asset(std::string_view s) {
        std::string t(s);
        image a = from_png((t + ".png").c_str());
        FILE* fp = fopen((t + ".json").c_str(), "rb");
        auto z = _string_from_file(fp);
        fclose(fp);
        json b = json::from(z);
        
        ptrdiff_t c = b["tile_size"].as_i64();
        json const& d = b["names"];
        for (size_t i = 0; i != d.size(); ++i) {
            for (size_t j = 0; j != d.size(); ++j) {
                matrix_view<pixel> e = a.sub(i * c, j * c, c, c);
            }
        }
        
        
        
        
    }

}
