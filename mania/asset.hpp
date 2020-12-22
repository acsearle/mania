//
//  asset.hpp
//  mania
//
//  Created by Antony Searle on 28/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef asset_hpp
#define asset_hpp

#include "atlas.hpp"
#include "table3.hpp"

namespace manic {

sprite load_image(string_view image_name, atlas& atl);
table3<string, sprite> load_asset(string_view, atlas& atl);
    string path_for_resource(string_view name, string_view ext);

} // namespace manic

#endif /* asset_hpp */
