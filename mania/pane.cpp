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
        auto q = new pane_text("Not a button");
        a->_pane = q;
        return a;
    }();
    return *x;
}

}
