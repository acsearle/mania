//
//  bytes.hpp
//  mania
//
//  Created by Antony Searle on 4/3/20.
//  Copyright © 2020 Antony Searle. All rights reserved.
//

#ifndef bytes_hpp
#define bytes_hpp

#include <cstdlib> // malloc
#include <cstring> // memcpy

#include "common.hpp"

namespace manic {

// block-transfer-oriented contiguous byte queue

struct bytes {
    
    byte* _begin;
    byte* _end;
    byte* _allocation;
    byte* _capacity;
    
    bool _invariant() {
        return ((_allocation <= _begin)
                && (_begin <= _end)
                && (_end <= _capacity));
    }
    
    isize size() const {
        return _end - _begin;
    }
    
    bytes()
    : _begin(nullptr)
    , _end(_begin)
    , _allocation(nullptr)
    , _capacity(nullptr) {
    }
    
    bytes(bytes const& x) {
        _capacity = _end = (_begin = _allocation = (byte*) std::malloc(x.size())) + size();
        std::memcpy(_begin, x._begin, x.size());
    }
    
    //bytes(bytes&& x)
    //: bytes(std::ex)
    
    
    
    

}; // struct bytes

} // namespace manic

#endif /* bytes_hpp */
