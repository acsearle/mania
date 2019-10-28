//
//  asset.hpp
//  mania
//
//  Created by Antony Searle on 28/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef asset_hpp
#define asset_hpp

#include <string_view>

#include "atlas.hpp"
#include "table3.hpp"

namespace manic {

table3<std::string, sprite> load_asset(std::string_view, atlas& atl);

} // namespace manic

#endif /* asset_hpp */
