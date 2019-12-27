//
//  pane.cpp
//  mania
//
//  Created by Antony Searle on 26/12/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "application.hpp"
#include "pane.hpp"

namespace manic {

application& application::get() {
    // static game x;
    static application* x = [](){
        application* a = new application;
        // a->_pane = new game();
        // auto q = new pane_text("Not a button");
        auto q = new pane_collection;
        rect<f32> r{{100,100},{200,200}};
        q->sub.emplace_back(r, new pane_button(new pane_text("New"), nullptr));
        q->sub.emplace_back(r, new pane_button(new pane_text("Continue"), nullptr));
        q->sub.emplace_back(r, new pane_button(new pane_text("Load"), nullptr));
        q->sub.emplace_back(r, new pane_button(new pane_text("Settings"), nullptr));
        q->sub.emplace_back(r, new pane_button(new pane_text("Exit"), nullptr));
        
        // auto-layout
        vec2 b{0,0};
        for (auto&& x : q->sub) {
            auto c = x.second->bound();
            b.x = std::max(b.x, c.x);
            b.y = std::max(b.y, c.y);
        }
        int y = b.y;
        for (auto&& x : q->sub) {
            x.first = {{0,0},{b}};
            x.first += vec2{b.x, y};
            y += b.y * 1.5;
        }
        a->_pane = q;
        
        
        return a;
    }();
    return *x;
}

vec2 pane_text::bound() {
    return draw_proxy::get().bound_text(this->_string).size();
}

void pane_text::draw(rect<f32> extent, draw_proxy* c) {
    assert(c);
    //c->draw_frame(extent);
    auto b = c->bound_text(_string);
    // c->draw_text(extent, _string);
    c->draw_text(extent - b.a, _string);
}


}
