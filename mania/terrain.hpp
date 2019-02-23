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

    matrix<double> terrain(ptrdiff_t i,
                           ptrdiff_t j,
                           ptrdiff_t rows,
                           ptrdiff_t columns,
                           uint64_t seed = 0);

} // namespace manic

#endif /* terrain_hpp */
