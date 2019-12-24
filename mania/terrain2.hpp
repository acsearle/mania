//
//  terrain2.hpp
//  mania
//
//  Created by Antony Searle on 6/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef terrain2_hpp
#define terrain2_hpp

#include "space2.hpp"

namespace manic {

struct _terrain_generator {
    
    u64 _seed;
    
    explicit _terrain_generator(u64 seed)
    : _seed(seed) {
    }
    
    struct _terrain_maker {

        vec<i64, 2> _xy;
        u64 _seed;

        matrix<u8> operator()() const;
        
    };
        
    _terrain_maker operator()(vec<i64, 2> xy) const {
        return _terrain_maker{xy, _seed};
    }
    
};

template<typename Serializer>
void serialize(_terrain_generator const& x, Serializer& s) {
    serialize(x._seed, s);
}

template<typename Deserializer>
_terrain_generator deserialize(placeholder<_terrain_generator>, Deserializer& d) {
    return _terrain_generator{deserialize<u64>(d)};
}

using terrain2 = space2<_terrain_generator>;

} // namespace manic

#endif /* terrain2_hpp */
