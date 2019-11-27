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
#include "debug.hpp"

namespace manic {

double _terrain_noise(uint64_t seed, ptrdiff_t i, ptrdiff_t j) {
    // generate uniform between [-1.0, +1.0)
    seed = hash(hash(seed ^ i) ^ j);
    auto x = static_cast<int64_t>(seed) * pow(2.0, -63);
    //std::cout << "sample " << x << std::endl;
    return x;
}

void _terrain_perturb(uint64_t seed, ptrdiff_t i, ptrdiff_t j, matrix_view<double> a) {
    for (ptrdiff_t s = 0; s != a.rows(); ++s)
        for (ptrdiff_t t = 0; t != a.columns(); ++t)
            a(s, t) += _terrain_noise(seed, i + s, j + t);
}

// TODO: When we are filtering the exploded values, 5/8ths of the values we
// encounter are zeros!  We can do better if we make a combined explode-filter-
// into-transpose operation we call twice.

// TODO: For tiling purposes we sometimes want to only generate nonzero values
// in some region (but still filter them into an expanded region).

// TODO: Provide explicit control over the amplitude and seed of each octave.
//       We don't always want "scale free" (if that's what default settings yield)

// TODO: Compute the variance of the output (or normalize it).

void _terrain_recurse(ptrdiff_t i,
                      ptrdiff_t j,
                      ptrdiff_t rows,
                      ptrdiff_t columns,
                      const_vector_view<double> filter,
                      ptrdiff_t depth,
                      matrix<double>& a,
                      matrix<double>& b,
                      uint64_t seed) {
    
    if (!depth) {
        a.discard_and_resize(rows, columns);
        a = 0.0;
        return;
        
    }
    
    assert(!(filter.size() & 1));
    // Expand our working size to account for filter shrinkage
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
    _terrain_recurse(i2, j2, rows2, columns2, filter, depth - 1, a, b, hash(seed));
    assert(a.rows() == rows2);
    assert(a.columns() == columns2);

    // Add noise
    _terrain_perturb(seed, i2, j2, a);
    
    std::cout << "variance of samples before filter " << variance(a) << std::endl;
    
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
    
    //std::cout << "min: " << min(a) << std::endl;
    //std::cout << "max: " << max(a) << std::endl;
    //std::cout << "rms: " << rms(a) << std::endl;
    //std::cout << "mean:" << mean(a) << std::endl;
    
    //std::cout << "minmax: " << min(a) << ", " << max(a) << std::endl;
    std::cout << "variance after filter:" << variance(a) << std::endl;

}

matrix<double> terrain(ptrdiff_t i,
                       ptrdiff_t j,
                       ptrdiff_t rows,
                       ptrdiff_t columns,
                       uint64_t seed) {
    
    timer _("terrain");
    
    // Deterministically generate terrain of given region
    
    vector<double> filter(16);
    for (ptrdiff_t i = 0; i != 16; ++i) {
        filter[i] = exp(-sqr(i - 7.5) / 8.0);
    }
    filter *= sqrt(4.0 * 2.0) / sum(filter);
    // The filter scale:
    //     sqrt: the filter is applied twice
    //      4.0: expanding the image 2x2 reduces amplitude by 4
    //      2.0: expanding the image 2x2 reduces the slopes by 2
    //           (this is implicitly a choice about the shape of the power
    //           spectrum)
    
    
    {
        // Variance of the uniform distribution from a to b is (b - a)^2 / 12
        // When the inputs are [-1, 1] from filtering the expanded noise,
        double v = 0.0;
        for (int i = 0 ; i != filter.size(); i += 2) {
            for (int j = 0 ; j != filter.size(); j += 2) {
                v += (pow(filter[i] * filter[j] * 2.0, 2) / 12.0);
            }
        }
        // Result of filter is approximately normally distributed as a sum of
        // multiple uniform distributions.
        DUMP(v);
        DUMP(sqrt(v));
    }
    
    
    
    
    matrix<double> a;
    matrix<double> b;
    
    _terrain_recurse(i, j, rows, columns, filter, 1, a, b, hash(seed));
    // we defensively hashed the seed
    
    /*
    std::vector<double> d;
    for (auto row : a)
        for (auto x : row)
            d.push_back(x);
    
    std::sort(d.begin(), d.end());
    std::cout << d.front() << std::endl;
    std::cout << d[d.size() / 4] << std::endl;
    std::cout << d[d.size() / 2] << std::endl;
    std::cout << d[d.size() * 3 / 4] << std::endl;
    std::cout << d.back() << std::endl;
*/
    
    return a;
}

} // namespace manic
