//
//  chunk.cpp
//  mania
//
//  Created by Antony Searle on 20/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "chunk.hpp"

namespace manic {
    
    chunk::chunk(ptrdiff_t i_, ptrdiff_t j_)
    : _i(i_)
    , _j(j_)
    , _tiles(_rows + 1, _columns + 1) {
        matrix<double> a = terrain(i_ * _rows, j_ * _columns, _rows + 1, _columns + 1);
        
        for (ptrdiff_t i = 0; i != a.rows(); ++i)
            for (ptrdiff_t j = 0; j != a.columns(); ++j)
                _tiles(i, j) = a(i, j) > 0;
        
        _tiles.print();
        
        
    }
    

}
