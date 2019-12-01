//
//  terrain2.cpp
//  mania
//
//  Created by Antony Searle on 6/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "terrain.hpp"
#include "terrain2.hpp"

namespace manic {

matrix<u8> _terrain_generator::_terrain_maker::operator()() const {
    
    constexpr i64 N = 16;
    constexpr i64 MASK = N - 1;
    
    matrix<double> a = terrain(_xy.x & ~MASK, _xy.y & ~MASK, N, N, _seed);
    
    matrix<u8> b(N, N);
    for (i64 i = 0; i != N; ++i)
        for (i64 j = 0; j != N; ++j)
            b(i, j) = std::clamp<double>(a(i, j) * 256 + 128, 0, 255);
            
    return b;
    
}

} // namespace manic
