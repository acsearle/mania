//
//  terrain.hpp
//  mania
//
//  Created by Antony Searle on 22/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef terrain_hpp
#define terrain_hpp

#include "matrix.hpp"

namespace manic {

// A high quality (and relatively expensive) terrain generator.  The world is
// determined by a seed; any two overlapping regions requested in any order
// will be consistent (up to minor floating point errors?).
// Unlike traditional Perlin noise, this terrain is both isotropic and
// approximately scale-free over a requested range of spatial frequencies.

// Using a hash function we generate deterministic uncorrelated and uniformly
// distributed values on a 2d grid.
//
// We apply one-dimensional Gaussian filters to the columns then the rows, to
// efficiently perform a 2d Gaussian filter.  This band-limits the noise and
// renders it isotropic.  However, the noise is still dominated by high
// spatial frequencies.

// To add lower spatial frequencies, we perform the same operation at multiple
// scales and add the upscaled results together.  Each scale further permutes
// the seed, so patterns do not repeat.

// We can optimize the process by using the same filter to perform the
// upscaling.

// The depth of the octave "stack" will determine the scale at which the
// largest features occur.  For small requests (<32), cost is approximately linear
// with number of ocatves.  For large requests (>32) cost is scales with the
// request area.

// The output will not 


matrix<double> terrain(ptrdiff_t i,
                       ptrdiff_t j,
                       ptrdiff_t rows,
                       ptrdiff_t columns,
                       uint64_t seed = 0);

} // namespace manic

#endif /* terrain_hpp */
