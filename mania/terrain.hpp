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
// Unlike traditional Perlin noise, this noise is both isotropic and
// approximately scale-free over a requested range of spatial frequencies.

matrix<double> terrain(ptrdiff_t i,
                       ptrdiff_t j,
                       ptrdiff_t rows,
                       ptrdiff_t columns,
                       uint64_t seed = 0);

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

// The output will be Gaussian, and centered at zero (but only when averaged
// over regions larger than the largest feature size).  The scale is implicitly
// determined by the filtering -- TODO: work it out; also, provide explicit
// control for the number and relative size of the octaves?

// With some modifications, we can manufacture tiles of various properties with
// this same basic idea:
//   * Using a mask (or modulus) on the coordiante inputs to the hash function
//     we can make the terrain repeat (the mask/modulus must be adjusted at
//     each ocatve.
//   * By switching between different hash functions in different regions of
//     the image we can make a set of tiles with matching edges and different
//     interiors, with the edges affecting the interior at scale-appropriate
//     distances  Or alternatively, we can only generate nonzero values inside
//     some regions and then sum together the results to make such tiles
//
//         A A A A   A A A A A A A A
//         A B B A   A B B B B B B A
//         A B B A + A B B B B B B A + ...
//         A A A A   A B B B B B B A
//                   A B B B B B B A
//                   A B B B B B B A
//                   A B B B B B B A
//                   A A A A A A A A
//
//         A A B B
//         A C C B <- Gives us Wang tiles with the possibility of extra variants
//         D C C E
//         D D E E
//
//     The size of the border will depend on how many orders of continuity we
//     want to provide and if any mipmapping consistency is required.  For
//     example if we want the local derivative to be consistent at boundaries
//     we need one more pixel of boundary at each level.
//         Above some scale the boundaries dominate, as they must, and all
//     tiles become the same.
//   * If we only generate noise within some region, and let zeros stand
//     outside it, we are making some square tiles with soft edges that will
//     sum up nicely with any neighbours.  Offset these and we get the wang
//     tiles, except
//
//         A A B B
//         A A B B
//         C C D D
//         C C D D
//
//     Note that this differs from simply smoothly blending two patterns
//     together, because the blend region is frequency aware in some sense.
//     Low frequencies will reach deep into the neighbour tile, high
//     frequencies will only affect the edge

} // namespace manic

#endif /* terrain_hpp */
