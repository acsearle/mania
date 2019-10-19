//
//  relocate.hpp
//  mania
//
//  Created by Antony Searle on 26/9/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef relocate_hpp
#define relocate_hpp

#include <stdio.h>

namespace manic {

// Relocatable types are those for which
//
//     new (dest) Relocatable(std::move(*src));
//     src->~Relocatable();
//
// is equivalent to
//
//     std::memcpy(dest, src, sizeof(Relocatable));
//
// This is true of all basic and movable STL types.  Generally, classes which
// are not relocatable store or share their own address.
//
// References imply an object exists at the referenced location, whereas raw
// pointers provide no such guarantee; thus we use pointers as the interface to
// relocate.

template<typename Relocatable>
void relocate(Relocatable* dest, Relocatable const* src) {
    std::memcpy(dest, src, sizeof(Relocatable));
}

}

#endif /* relocate_hpp */
