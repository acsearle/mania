//
//  pane.cpp
//  mania
//
//  Created by Antony Searle on 26/12/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#include "application.hpp"
#include "pane.hpp"
#include "renderer.hpp"

namespace manic {

pane::base* make_game();

application& application::get() {
    // static game x;
    static application* x = [](){
        application* a = new application;
        a->_pane = make_game();
        // auto q = new pane_text("Not a button");
        /*
        auto q = new pane::collection;
        rect<f32> r{{100,100},{200,200}};
        q->sub.emplace_back(r, box<pane::button>::from(box<pane::text>::from("New"), [=](){
            a->_pane = make_game();
        }));
        q->sub.emplace_back(r, box<pane::button>::from(box<pane::text>::from("Continue"), nullptr));
        q->sub.emplace_back(r, box<pane::button>::from(box<pane::text>::from("Load"), nullptr));
        q->sub.emplace_back(r, box<pane::button>::from(box<pane::text>::from("Settings"), nullptr));
        q->sub.emplace_back(r, box<pane::button>::from(box<pane::text>::from("Exit"), nullptr));
        
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
        */
        return a;
    }();
    return *x;
}

vec2 pane::text::bound() {
    return draw_proxy::get().bound_text(this->_string).size();
}

void pane::text::draw(rect<f32> extent, draw_proxy* c) {
    assert(c);
    //c->draw_frame(extent);
    auto b = c->bound_text(_string);
    // c->draw_text(extent, _string);
    c->draw_text(extent - b.a, _string);
}

void pane::collection::draw(rect<f32> extent, draw_proxy* c) {
    
    // todo:
    // * load images into their own textures
    // * enable filtering
    // * pan and zoom slowly ( < 60 pixels / s)
    // * fade between images
    // * like apple image screensaver
    c->draw_sprite(extent.mid(), _background);
    
    for (auto&& [r, p] : sub)
        p->draw(r, c);
}

vec2 event::base::mouse() const {
    return vec2{}; // FIXME
}

bool event::mouse_up::visit(pane::base* p) {
    return p->handle_event(rect<f32>{{},{}}, this); // FIXME
}

bool event::mouse_down::visit(pane::base* p) {
    return p->handle_event(rect<f32>{{},{}}, this); // FIXME
}

bool event::key_up::visit(pane::base* p) {
    return p->handle_event(rect<f32>{{},{}}, this); // FIXME
}

bool event::key_down::visit(pane::base* p) {
    return p->handle_event(rect<f32>{{},{}}, this); // FIXME
}

bool event::scrolled::visit(pane::base* p) {
    return p->handle_event(rect<f32>{{},{}}, this); // FIXME
}

bool event::mouse_moved::visit(pane::base* p) {
    return p->handle_event(rect<f32>{{},{}}, this); // FIXME
}




}
