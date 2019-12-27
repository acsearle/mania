//
//  draw_proxy.hpp
//  mania
//
//  Created by Antony Searle on 26/12/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef draw_proxy_hpp
#define draw_proxy_hpp

#include "rect.hpp"
#include "string_view.hpp"

namespace manic {


struct draw_proxy {

    virtual ~draw_proxy() = default;
    
    // is this a singleton, or should it be a member of application_concrete or something?
    static draw_proxy& get();

    // before drawing anything, setup viewport and view transform to pixel-perfect
    virtual void presize(vec2 ext) = 0;
    // splat the accumulated sprites
    virtual void commit() = 0;
    
    // good for UI panes but not game pane?
    virtual void draw_frame(rect<f32>) = 0;
    
    // good for all panes
    virtual void draw_text(rect<f32>, string_view v) = 0;
    virtual rect<f32> bound_text(string_view v) = 0;
    
    // too general
    virtual void draw_asset(vec2, string_view v) = 0;
    
    // overly-specific interfaces, we need to remove them somehow
    virtual void draw_terrain(vec2, int i) = 0;
    virtual void draw_animation_h(vec2, int i) = 0;
    virtual void draw_animation_v(vec2, int i) = 0;
    // while the draw_proxy owns the atlas, maybe particular classes of pane
    // own the vector<sprites> of specific stuff inside the atlas.
    // On construction of pane, loads the pane-specific sprites and gets their
    // handle.  Then these methods become in terms of sprites.

    // todo: select what layer we are drawing in, then resolve multiple draw
    // calls.
    

    
        
}; // struct draw_proxy



} // namespace manic
#endif /* draw_proxy_hpp */