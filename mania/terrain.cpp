//
//  terrain.cpp
//  mania
//
//  Created by Antony Searle on 22/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "terrain.hpp"

#include "hash.hpp"
#include "vector.hpp"

namespace manic {
    
    double _terrain_noise(uint64_t seed, ptrdiff_t i, ptrdiff_t j) {
        seed = hash(hash(seed ^ i) ^ j);
        return static_cast<int64_t>(seed) * pow(2.0, -63);
    }
    
    void _terrain_perturb(uint64_t seed, ptrdiff_t i, ptrdiff_t j, matrix_view<double> a) {
        for (ptrdiff_t s = 0; s != a.rows(); ++s)
            for (ptrdiff_t t = 0; t != a.columns(); ++t)
                a(s, t) += _terrain_noise(seed, i + s, j + t);
    }
    
    void _terrain_recurse(ptrdiff_t i,
                          ptrdiff_t j,
                          ptrdiff_t rows,
                          ptrdiff_t columns,
                          const_vector_view<double> filter,
                          ptrdiff_t depth,
                          matrix<double>& a,
                          matrix<double>& b,
                          uint64_t seed) {
        
        if (depth == 7) {
            
            a.discard_and_resize(rows, columns);
            a = 0.0;
            return;
            
        }
        
        // Expand our working size to account for filter losses
        i -= filter.size() / 2;
        j -= filter.size() / 2;
        rows += filter.size();
        columns += filter.size();
        
        std::cout << i << ", " << j << ", " << rows << ", " << columns << std::endl;
        
        // Round coordinates to next octave
        ptrdiff_t i2 = i >> 1; // rounding down
        ptrdiff_t j2 = j >> 1;
        ptrdiff_t rows2 = ((i + rows + 1) >> 1) - i2; // rounding up
        ptrdiff_t columns2 = ((j + columns + 1) >> 1) - j2;
        
        // Get terrain from next level
        _terrain_recurse(i2, j2, rows2, columns2, filter, depth + 1, a, b, seed);
        assert(a.rows() == rows2);
        assert(a.columns() == columns2);
        
        // Add noise
        _terrain_perturb(hash(seed ^ depth), i2, j2, a);
        // seed has already been hashed so it won't have coincidences with depth
        
        // Double size
        b.discard_and_resize(a.rows() * 2, a.columns() * 2);
        b = 0.0;
        explode(b, a);
        
        // Account for rounding in higher coordinates
        b.crop(i - i2 * 2, j - j2 * 2, rows, columns);
        
        // Low-pass filter by ping-pong
        a.discard_and_resize(b.rows(), b.columns() - filter.size());
        a = 0.0;
        assert(a._invariant());
        assert(b._invariant());
        filter_rows(a, b, filter);
        b.discard_and_resize(a.rows() - filter.size(), a.columns());
        b = 0.0;
        filter_columns(b, a, filter);
        
        // Prepare output
        swap(a, b);
        
        std::cout << "rms: " << rms(a) << std::endl;
        
    }
    
    matrix<double> terrain(ptrdiff_t i,
                           ptrdiff_t j,
                           ptrdiff_t rows,
                           ptrdiff_t columns,
                           uint64_t seed) {
        
        // Deterministically generate terrain of given region
        
        vector<double> filter(16);
        for (ptrdiff_t i = 0; i != 16; ++i) {
            filter[i] = exp(-sqr(i - 7.5) / 8.0);
        }
        filter *= sqrt(8.0) / sum(filter);
        
        matrix<double> a;
        matrix<double> b;
        
        _terrain_recurse(i, j, rows, columns, filter, 0, a, b, hash(seed));
        // we defensively hashed the seed
        
        return a;
    }
    
} // namespace manic
