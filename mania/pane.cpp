//
//  pane.cpp
//  mania
//
//  Created by Antony Searle on 20/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "pane.hpp"
#include "vector.hpp"
#include "rect.hpp"
#include "common.hpp"


namespace manic {

template<typename T>
class box {
    
    T* _ptr;
    
    box() : _ptr(nullptr) {}
    
    box(box const&) = delete;
    box(box&& x) : _ptr(std::exchange(x._ptr, nullptr)) {}
    
    ~box() { delete _ptr; }
    
    box& operator=(box const&) = delete;
    box& operator=(box&& x) {
        box y{std::move(x)};
        using std::swap;
        swap(_ptr, y._ptr);
    }

    template<typename... Args>
    static box from(Args&&... args) {
        box a;
        a._ptr = new T{std::forward<Args>(args)...};
        return a;
    }
    
};

class pane {
    
    // nested views for input (and drawing?)
    
    // mouse location is passed with every event?
    // events needs to occur at a defined time in game loop
    
    rect<i64> ext;
    vector<box<pane>> sub;
    
    virtual ~pane() = default;
    
    virtual void mouse_down(int button) {}
    virtual void mouse_up(int button) {}

    virtual void scroll(vec<i64, 2> delta);
    
    virtual pane* mouse_move(vec<i64, 2> x);
    
    virtual void draw();
    
}; // class pane

// types of panes
// minimap
// game world
// line of text
// grid of squares
//     instruction picker (row of squares?)
//          subinstruction picker (popup column of squares?)
// list of panes
// table of panes

} // namespace manic
