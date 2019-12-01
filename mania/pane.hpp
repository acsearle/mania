//
//  pane.hpp
//  mania
//
//  Created by Antony Searle on 20/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef pane_hpp
#define pane_hpp


#include "pane.hpp"
#include "vector.hpp"
#include "rect.hpp"
#include "common.hpp"
#include "string.hpp"

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




struct draw_proxy {

    virtual ~draw_proxy() = default;
    
    virtual void draw_frame(rect<f32>) = 0;
    virtual void draw_text(rect<f32>, string_view v) = 0;
    
};

struct event_proxy {
    
    virtual ~event_proxy() = default;
    
    virtual vec2 mouse() const = 0;
    
    
};

// nested views for input (and drawing?)

// mouse location is passed with every event?
// events needs to occur at a defined time in game loop

// we have either a global mouse location or pass it with every event?
// keystrokes need to be captured by a selected thing

// on click register as keyboard handler ? which is a global stack
// on move register as mouse handler?  which is a global?


struct pane {
    
    rect<f32> _ext;
    
    explicit pane(rect<f32> x) : _ext(x) {}
    
    virtual ~pane() = default;
    
    virtual bool mouse_move(event_proxy*) { return false; }

    virtual bool mouse_down(u64 button, event_proxy*) { return false; }
    virtual bool mouse_up(u64 button, event_proxy*) { return false; }

    virtual bool key_down(u32, event_proxy*) { return false; }
    virtual bool key_up(u32, event_proxy*) { return false; }

    virtual bool scrolled(vec<f32, 2> delta, event_proxy*) { return false; }
    
    virtual void resize(rect<f32>) {}
    virtual void draw(draw_proxy*) {}
        
    bool within(vec<f32, 2> x) {
        return (_ext.a.x <= x.x) && (_ext.a.y <= x.y) && (x.x < _ext.b.x) && (x.y < _ext.b.y);
    }
    
}; // class pane

struct pane_collection : pane {
    
    using pane::pane;
    
    vector<box<pane>> sub;
    
    virtual bool mouse_move(event_proxy* e) override {
        if (!within(e->mouse()))
            return false;
        for (auto& p : sub)
            if (p->mouse_move(e))
                return true;
        return false;
    }
    
    virtual void draw(draw_proxy* c) override {
        for (auto& p : sub)
            p->draw(c);
    }
    
};

struct pane_text : pane {

    
    string _string;
    
    pane_text(rect<f32> x, string_view s) : pane(x), _string(s) {}
    
    virtual void draw(draw_proxy* c) override {
        assert(c);
        c->draw_frame(_ext);
        c->draw_text(_ext, _string);
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

#endif /* pane_hpp */
