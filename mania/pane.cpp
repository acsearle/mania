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
struct box {
    
    T* _ptr;
    
    box() : _ptr(nullptr) {}
    
    box(box const&) = delete;
    box(box&& x) : _ptr(std::exchange(x._ptr, nullptr)) {}
    
    ~box() { delete _ptr; }
    
    box& operator=(box const&) = delete;
    box& operator=(box&& x);
    
    template<typename... Args>
    static box from(Args&&... args) {
        box a;
        a._ptr = new T{std::forward<Args>(args)...};
        return a;
    }
    
    T* operator->() const {
        assert(_ptr);
        return _ptr;
    }
    
}; // struct box

template<typename T>
box<T>& box<T>::operator=(box<T>&& x) {
    box y{std::move(x)};
    using std::swap;
    swap(_ptr, y._ptr);
}


// nested views for input (and drawing?)

// mouse location is passed with every event?
// events needs to occur at a defined time in game loop

// we have either a global mouse location or pass it with every event?
// keystrokes need to be captured by a selected thing

// on click register as keyboard handler ? which is a global stack
// on move register as mouse handler?  which is a global?

struct draw_context;

struct pane {
    
    rect<i64> ext;
    
    virtual ~pane() = default;
    
    virtual bool mouse_down(int button) { return false; }
    virtual bool mouse_up(int button) { return false; }
    
    virtual bool scroll(vec<i64, 2> delta) { return false; }
    
    virtual bool mouse_move(vec<i64, 2> x) { return false; }
    
    virtual void draw(draw_context*) {}
    
    bool within(vec<i64, 2> x) {
        return (ext.a.x <= x.x) && (ext.a.y <= x.y) && (x.x < ext.b.x) && (x.y < ext.b.y);
    }
    
}; // class pane

struct pane_collection : pane {
    
    vector<box<pane>> sub;
    
    virtual bool mouse_move(vec<i64, 2> x) override {
        if (!within(x))
            return false;
        for (auto& p : sub)
            if (p->mouse_move(x))
                return true;
        return false;
    }
    
    virtual void draw(draw_context* c) override {
        for (auto& p : sub)
            p->draw(c);
    }
    
};


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
