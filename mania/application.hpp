//
//  application.hpp
//  mania
//
//  Created by Antony Searle on 30/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef application_hpp
#define application_hpp

#include "string.hpp"
#include "table3.hpp"
#include "vec.hpp"
#include "pane.hpp"

namespace manic {

struct event_proxy {};

struct application {
    
    // Entry point and firewall for platform-independent code.
    
    // maybe this is NSWindow-like, pane is NSView-like?
    
    pane::base* _pane;
    usize _width, _height;
    
    
    struct application_event_proxy : event_proxy {
        vec<double, 2> _mouse_window;
        table3<u32, bool> _keyboard_state;
        u64 _mouse_state;

        virtual vec2 mouse() const {
            return _mouse_window;
        }
        
    } _event_proxy ;
    
    
    application() = default;
    application(const application&) = delete;
    application(application&&) = delete;
    virtual ~application() = default;
    application& operator=(const application&) = delete;
    application& operator=(application&&) = delete;
    
    static application& get(); // hopefully the only singleton?
    
    virtual void resize(usize width, usize height);
    virtual void draw();
    
    virtual void key_down(u32);
    virtual void key_up(u32);

    virtual void mouse_down(manic::u64);
    virtual void mouse_up(manic::u64);


    void mouse_moved(double x, double y);
    
    virtual void scrolled(double x, double y);

}; // struct application

} // namespace manic

#endif /* application_hpp */
