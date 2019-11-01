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

namespace manic {

struct application {
    
    table3<u32, bool> _keyboard_state;
    u64 _mouse_state;
    vec<double, 2> _mouse_window;
    
    application() = default;
    application(const application&) = delete;
    application(application&&) = delete;
    virtual ~application() = default;
    application& operator=(const application&) = delete;
    application& operator=(application&&) = delete;
    
    static application& get();
    
    virtual void resize(usize width, usize height) = 0;
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
