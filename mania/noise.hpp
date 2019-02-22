//
//  noise.hpp
//  mania
//
//  Created by Antony Searle on 25/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#ifndef noise_hpp
#define noise_hpp

#include <cstdio>

#include "hash.hpp"
#include "vec.hpp"

namespace manic {
    
    // deterministic multidimensional noise grid
    
    /*
    template<std::size_t N>
    uint64_t noise(uint64_t seed, gl::vec<uint64_t, N> x) {
        for (size_t i = 0; i != N; ++i)
            seed = x[i] = hash(seed ^ x[i]);
        return seed;
    }
     */
    
    /*
    uint64_t noise(uint64_t seed, ptrdiff_t i, ptrdiff_t j) {
        seed ^= hash(i);
        seed ^= hash(j);
        return seed;
    }
     */
    
    // filtered octave interpolation over grid
    // recurse on dimension
    // compute gaussian filters at two scales
    // recompose filters
    // subtract low frequency to produce band-limited spherically symmetric noise
    
    // on grid?
    
    
    
}

#endif /* noise_hpp */
