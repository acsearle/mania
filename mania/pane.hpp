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
    
    virtual ~pane() = default;
    
    virtual bool mouse_move(rect<f32> extent, event_proxy*) { return false; }

    virtual bool mouse_down(rect<f32>, u64 button, event_proxy*) { return false; }
    virtual bool mouse_up(rect<f32>, u64 button, event_proxy*) { return false; }

    virtual bool key_down(u32, event_proxy*) { return false; }
    virtual bool key_up(u32, event_proxy*) { return false; }

    virtual bool scrolled(vec<f32, 2> delta, event_proxy*) { return false; }
    
    virtual void draw(rect<f32> extent, draw_proxy*) {}
    
    virtual vec2 bound() { return vec2(0, 0); }
            
}; // class pane

struct pane_collection : pane {
    
    using pane::pane;
    
    vector<std::pair<rect<f32>, box<pane>>> sub;
    
    virtual bool mouse_move(rect<f32> extent, event_proxy* e) override {
        if (!extent.contains(e->mouse()))
            return false;
        for (auto&& [r, p] : sub)
            if (p->mouse_move(r, e))
                return true;
        return false;
    }
    
    virtual void draw(rect<f32> extent, draw_proxy* c) override {
        c->draw_sprite(extent.mid(), _background);
        for (auto&& [r, p] : sub)
            p->draw(r, c);
    }
    
    virtual vec2 bound() override {

        vec2 x(0, 0);
        for (auto&& [r, p] : sub) {
            vec2 y = r.size();
            x.x = std::max(x.x, y.x);
            x.y = std::max(x.y, y.y);
        }
        return x;
        
    }
        
};

struct pane_text : pane {

    
    string _string;
    
    pane_text(string_view s) : _string(s) {}
    
    virtual void draw(rect<f32> extent, draw_proxy* c) override;
    
    virtual vec2 bound() override;
    
};


struct pane_palette : pane {
    
    matrix<box<pane>> _stuff;
    vec2 _stride;
    
    virtual void draw(rect<f32> extent, draw_proxy* c) override {
        c->draw_frame(extent);
        for (isize i = 0; i != _stuff.rows(); ++i)
            for (isize j = 0; j != _stuff.columns(); ++j)
                _stuff(i, j)->draw(rect<f32>{extent.a, extent.a + _stride} + vec2{i, j} * _stride, c);
                
    }
    
    virtual bool mouse_move(rect<f32> extent, event_proxy* e) override {
        auto m = e->mouse();
        if (!extent.contains(m))
            return false;
        auto x = (m - extent.a) / _stride;
        return _stuff(vec<isize, 2>{x})->mouse_move(extent, e);
    }
    
};

struct pane_opcode : pane {
    u64 _opcode;
    
    pane_opcode(u64 code) : _opcode(code) {}
    
    virtual void draw(rect<f32> extent, draw_proxy* c) override {

    }

};

struct pane_button : pane {
    
    box<pane> _appearance;
    std::function<void()> _action;
    
    template<typename FF>
    pane_button(pane* p, FF&& ff)
    : _appearance(p)
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
    
}; // struct pane_button


struct pane_stack : pane {
    
    std::vector<box<pane>> _panes;
    
    virtual void draw(rect<f32> extent, draw_proxy* c) override {
        assert(_panes.size());
        _panes.back()->draw(extent, c);
    }

    virtual vec2 bound() override {
        assert(_panes.size());
        return _panes.back()->bound();
    }
    
    void push(box<pane>&& p) {
        _panes.push_back(std::move(p));
    }
    
    box<pane> pop() {
        assert(_panes.size());
        box<pane> p{std::move(_panes.back())};
        _panes.pop_back();
        return p;
    }

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

} // namespace manic

#endif /* pane_hpp */
