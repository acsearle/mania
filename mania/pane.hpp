//
//  pane.hpp
//  mania
//
//  Created by Antony Searle on 20/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef pane_hpp
#define pane_hpp

#include <functional>

#include "pane.hpp"
#include "vector.hpp"
#include "rect.hpp"
#include "common.hpp"
#include "string.hpp"
#include "matrix.hpp"
#include "box.hpp"
#include "draw_proxy.hpp"

namespace manic {

namespace pane {

struct base;

} // namespace pane

namespace event {

struct base {
    
    virtual ~base() = default;
    virtual bool visit(pane::base*) = 0;
    
    vec<f32, 2> mouse() const;

    
};

struct key_up : base {
    u32 c;
    explicit key_up(u32 cc) : c(cc) {}
    virtual bool visit(pane::base*);
};

struct key_down : base {
    u32 c;
    explicit key_down(u32 cc) : c(cc) {}
    virtual bool visit(pane::base*);
};

struct mouse_down : base {
    u64 button;
    explicit mouse_down(u64 b) : button(b) {}
    virtual bool visit(pane::base*);
};

struct mouse_up : base {
    u64 button;
    explicit mouse_up(u64 b) : button(b) {}
    virtual bool visit(pane::base*);
};

struct mouse_moved : base {
    explicit mouse_moved(vec2) {}
    virtual bool visit(pane::base*);
};

struct scrolled : base {
    vec2 delta;
    explicit scrolled(vec2 d) : delta(d) {};
    virtual bool visit(pane::base*);
};
    
} // namespace event


namespace pane {

// nested views for input (and drawing?)

// mouse location is passed with every event?
// events needs to occur at a defined time in game loop

// we have either a global mouse location or pass it with every event?
// keystrokes need to be captured by a selected thing

// on click register as keyboard handler ? which is a global stack
// on move register as mouse handler?  which is a global?


struct base {
    
    virtual ~base() = default;
    
    // each pane needs to handle events
    // some panes need to forward all events to their subpanes, the same way,
    //     regardless of event type -> single virtual function
    // some panes need to handle only one event -> multiple virtual functions,
    //     defaulted to did-not-handle (return false)
    //
    // tools: virtual function calls
    //        pointer-to-virtual-member function
    //        switch on discriminant
    //        higher: visitor, multimethod
    //
    // panes are a classic OOP case, with an interface of multiple functions
    // and default behaviours
    //
    // events could be inheritance hierachy or discriminated union
    //
    // modified visitor might be a nice fit:
    //
    // virtual bool pane::base::handle(event::base& e) {
    //     e.visit(*this);
    // }
    //
    // virtual bool pane::collection::handle(event::base& e) {
    //      for (p : sub) {
    //          if (in_extents(?) && p->handle(e))
    //              return true;
    //      }
    //      return false;
    // }
    //
    // virtual bool event::mouse::visit(pane::base& p) {
    //     p->handle(*this);
    // }
    //
    // virtual bool pane::button::handle(event::mouse& e) {
    //     f();
    // }
        
    // contra:
    //
    // too much bouncing around vs. classic thing
    // too many types
    //
    // we want to handle key events differently to mouse events
    

    virtual bool handle_event(rect<f32> extent, event::base* e) {
        return e->visit(this);
    }
    
    virtual bool handle_event(rect<f32>, event::mouse_moved*) { return false; }
    virtual bool handle_event(rect<f32>, event::key_down*) { return false; }
    virtual bool handle_event(rect<f32>, event::mouse_up*) { return false; }
    virtual bool handle_event(rect<f32>, event::mouse_down*) { return false; }
    virtual bool handle_event(rect<f32>, event::scrolled*) { return false; }

    
    virtual void draw(rect<f32> extent, draw_proxy*) {}
    
    virtual vec2 bound() { return vec2(0, 0); }
            
}; // class pane::base

struct collection : base {
    
    using base::base;
    
    vector<std::pair<rect<f32>, box<base>>> sub;
        
    virtual void draw(rect<f32> extent, draw_proxy* c) override;
    
    virtual vec2 bound() override {

        vec2 x(0, 0);
        for (auto&& [r, p] : sub) {
            vec2 y = r.size();
            x.x = std::max(x.x, y.x);
            x.y = std::max(x.y, y.y);
        }
        return x;
        
    }
    
    /*
    virtual bool mouse_move(rect<f32> extent, event_proxy* e) override {
        if (!extent.contains(e->mouse()))
            return false;
        for (auto&& [r, p] : sub)
            if (p->mouse_move(r, e))
                return true;
        return false;
    }

    virtual bool mouse_down(rect<f32> extent, u64 button, event_proxy* e) override {
        if (!extent.contains(e->mouse()))
            return false;
        for (auto&& [r, p] : sub)
            if (p->mouse_down(r, button, e))
                return true;
        return false;
    }
     */
    
    virtual bool handle_event(rect<f32> extent, event::base* e) override {
        for (auto&& [r, p] : sub) {
            if (r.contains(e->mouse()) && p->handle_event(r, e))
                return true;
        }
        return false;
    }
        
};

struct text : base {

    
    string _string;
    
    text(string_view s) : _string(s) {}
    
    virtual void draw(rect<f32> extent, draw_proxy* c) override;
    
    virtual vec2 bound() override;
    
};

struct paragraph : base {

    std::vector<string> _lines;

    virtual void draw(rect<f32> extent, draw_proxy* c) override;
    
    virtual vec2 bound() override;

    void reflow(rect<f32> extent) {
        
    }
};


struct palette : base {
    
    matrix<box<base>> _stuff;
    vec2 _stride;
    
    virtual void draw(rect<f32> extent, draw_proxy* c) override {
        c->draw_frame(extent);
        for (isize i = 0; i != _stuff.rows(); ++i)
            for (isize j = 0; j != _stuff.columns(); ++j)
                _stuff(i, j)->draw(rect<f32>{extent.a, extent.a + _stride} + vec2{i, j} * _stride, c);
                
    }
    
    virtual bool handle_event(rect<f32> extent, event::base* e) override {
        auto m = e->mouse();
        if (!extent.contains(m))
            return false;
        auto x = (m - extent.a) / _stride;
        // return _stuff(vec<isize, 2>{x})->mouse_move(extent, e);
        return _stuff(vec<isize, 2>{x})->handle_event(extent, e);
    }
    
};

struct opcode : base {
    u64 _opcode;
    
    opcode(u64 code) : _opcode(code) {}
    
    virtual void draw(rect<f32> extent, draw_proxy* c) override {

    }

};

struct button : base {
    
    box<base> _appearance;
    std::function<void()> _action;
    
    template<typename FF>
    button(box<base> p, FF&& ff)
    : _appearance(std::move(p))
    , _action(std::forward<FF>(ff)) {
    }
    
    virtual void draw(rect<f32> extent, draw_proxy* c) override {
        c->draw_frame(extent);
        extent.a += 4;
        extent.b -= 4;
        _appearance->draw(extent, c);
    }
    
    virtual vec2 bound() override {
        return _appearance->bound() + 8;
    }
    
    virtual bool handle_event(rect<f32> extent, event::mouse_down* e) override {
        if (!_action)
            return false;
        _action();
        return true;
    }

}; // struct pane_button


struct stack : base {
    
    std::vector<box<base>> _panes;
    
    virtual void draw(rect<f32> extent, draw_proxy* c) override {
        assert(_panes.size());
        _panes.back()->draw(extent, c);
    }

    virtual vec2 bound() override {
        assert(_panes.size());
        return _panes.back()->bound();
    }
    
    void push(box<base>&& p) {
        _panes.push_back(std::move(p));
    }
    
    box<base> pop() {
        assert(_panes.size());
        box<base> p{std::move(_panes.back())};
        _panes.pop_back();
        return p;
    }

};


struct horizontal_splitter : collection {
    
    void layout(rect<f32> extent) {

        for (auto&& x : this->sub) {
            auto c = x.second->bound();
            x.first = {extent.a, extent.a + c};
            extent.a.x += c.x;
        }
        
    }
    
};

struct vertical_splitter : collection {

    void layout(rect<f32> extent) {

        for (auto&& x : this->sub) {
            auto c = x.second->bound();
            x.first = {extent.a, extent.a + c};
            extent.a.y += c.y;
        }
        
    }

};

struct compositor : base {
    
    std::vector<box<base>> _panes;
    
    virtual void draw(rect<f32> extent, draw_proxy* c) override {
        for (auto&& x : _panes)
            x->draw(extent, c);
    }
    
    virtual vec2 bound() override {
        vec2 b = { 0, 0 };
        for (auto&& x : _panes) {
            auto c = x->bound();
            b.x = std::max(b.x, c.x);
            b.y = std::max(b.y, c.y);
        }
        return b;
    }

};

struct mouseable : base {
    
    // holds base class of mouseable things?
    // is the base class of mouseable things?
    
    // "singleton" mouse also has to hold it, so not pane
    
};

// types of panes
// minimap
// game world
// line of text
// grid of squares
//     instruction picker (row of squares?)
//          subinstruction picker (popup column of squares?)
// list of panes (h, v)
// table of panes
// slider, checkbox
// button

} // namespace pane

} // namespace manic

#endif /* pane_hpp */
