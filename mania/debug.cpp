//
//  debug.cpp
//  mania
//
//  Created by Antony Searle on 5/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include <mach/mach_time.h>

#include "debug.hpp"

namespace manic {

timer::timer(char const* context)
: _begin(mach_absolute_time())
, _context(context) {
}

timer::~timer() {
    printf("%s: %gms\n", _context, (mach_absolute_time() - _begin) * 1e-6);
}



} // namespace manic
