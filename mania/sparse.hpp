//
//  sparse.hpp
//  mania
//
//  Created by Antony Searle on 23/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef sparse_h
#define sparse_h

namespace manic {
    
    template<typename T>
    class sparse {
        
        T _value;
        
        sparse* _left;
        sparse* _right;
        sparse* _up;
        sparse* _down;
        sparse* _next;
        sparse* _prev;
        
        ~sparse() {
            if (_left)
                _left->_right = nullptr;
            if (_right)
                _right->_left = nullptr;
            if (_up)
                _up->_down = nullptr;
            if (_down)
                _down->_up = nullptr;
            if (_next)
                _next->_prev = _prev;
            if (_prev)
                _prev->_next = _next;
        }

        
    };
    


#endif /* sparse_h */
