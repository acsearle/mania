//
//  elements.hpp
//  mania
//
//  Created by Antony Searle on 25/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef elements_hpp
#define elements_hpp

#include "common.hpp"

namespace manic {
    
enum elements : u64 {
    hydrogen = 1ull << 0,
    carbon = 1ull << 1,
    oxygen = 1ull << 2,
    silicon = 1ull << 3,
    sulfur = 1ull << 4,
    calcium = 1ull << 5,
    iron = 1ull << 6,
    copper = 1ull << 7,
    nitrogen = 1ull << 8,
};

enum materials : u64 {
    water = hydrogen | oxygen,
    silica = silicon | oxygen,
    limestone = calcium | carbon | oxygen,
    earth = carbon | hydrogen | oxygen | silicon,
    air = oxygen | nitrogen,
    coal = carbon | sulfur,
    
};

} // namespace manic

#endif /* elements_hpp */
