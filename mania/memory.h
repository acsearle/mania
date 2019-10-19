//
//  memory.h
//  mania
//
//  Created by Antony Searle on 15/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef memory_h
#define memory_h

namespace manic {

inline void memswap(void* p, void* q, std::size_t count) {
    auto a = static_cast<unsigned char*>(p);
    auto b = static_cast<unsigned char*>(q);
    auto c = a + count;
    for (; a != c; ++a, ++b) {
        unsigned char t{*a};
        *a = *b;
        *b = t;
    }
}

} // namespace manic

#endif /* memory_h */
