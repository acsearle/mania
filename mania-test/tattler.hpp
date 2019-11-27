//
//  tattler.hpp
//  mania-test
//
//  Created by Antony Searle on 27/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef tattler_hpp
#define tattler_hpp

#include "common.hpp"

namespace manic {

struct tattler {
    static isize _live;
    tattler() {
        ++_live;
    }
    tattler(const tattler&) {
        ++_live;
    }
    ~tattler() {
        --_live;
    }
};

} // namespace manic

#endif /* tattler_hpp */
