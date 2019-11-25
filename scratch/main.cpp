//
//  main.cpp
//
//  Created by Antony Searle on 15/12/18.
//  Copyright © 2018 Antony Searle. All rights reserved.
//

#include "projections.hpp"

int main(int argc, char** argv) {
    main_projections(argc, argv);
}


/*
#include <iostream>

#include "image.hpp"
#include "debug.hpp"
#include "projections.hpp"

namespace manic {

void foo() {
    
    image a = from_png_and_multiply_alpha("/Users/acsearle/Downloads/owen-crop.png");

    pixel z[] = {
        { 0, 0, 0, 255 },
        { 85, 85, 85, 255 },
        { 170, 170, 170, 255 },
        { 170, 85, 85, 255 },
        { 85, 170, 85, 255 },
        { 85, 85, 170, 255 },
        { 170, 170, 85, 255 }
    };
    
    vector_view<pixel> y(z, z + 7);
    
    for (vector_view<pixel> b : a)
        for (pixel& c : b) {
            
            //c.r = (c.r < 128) ? 0 : 255;
            //c.g = (c.g < 128) ? 0 : 255;
            //c.b = (c.b < 128) ? 0 : 255;
            c = *std::min_element(y.begin(), y.end(), [=](pixel u, pixel v) {
                return distance(c, u) < distance(c, v);
            });
        }
    
    
    to_png(a, "/Users/acsearle/Downloads/owen-posterize.png");
}



}

int main(int argc, char** argv) {
    manic::foo();
}

*/


/*
template<typename...> struct typelist {};

template<typename>
struct introspect_method {};

template<typename R, typename T, typename... Args>
struct introspect_method<R (T::*)(Args...)> {
    using return_t = R;
    using object_t = T;
    using args_t = typelist<Args...>;
};

#define INTROSPECT(T, F) introspect_method<decltype(&T::F)>;

struct widget {
    int foo();
    void bar(int);
};

int main(int argc, char** argv) {
    std::cout << INTROSPECT(widget, foo)::return_t << std::endl;
}
*/

/*
struct widget {
    int foo();
    void bar(int);
};

template<typename>
struct analyzer {
};

template<typename A, typename B, typename... Args>
struct analyzer<A (B::*)(Args...)> {
    using return_t = A;
};

int main(int argc, char** argv) {

    std::cout << std::is_void_v<decltype(std::declval<widget>().foo())> << std::endl;
    std::cout << std::is_void_v<decltype(std::declval<widget>().bar(7))> << std::endl;

    return 0;
}
*/
/*
namespace manic {

void foo() {
    
    auto a = from_png_and_multiply_alpha("/Users/acsearle/Documents/pov/silo00.png");
    auto b = a(0,0);
    DUMP(vec4(b));
    
    DUMP(pow(b.r / 255.0, 2.2));
    
    DUMP(0.04045 * 255.0);
        
    DUMP(pow(0.5, 1.0/2.2) * 255);
    
    auto sRGB = [](double u) {
        if (u <= 0.04045) {
            return u / 12.92;
        } else {
            return pow((u + 0.055) / 1.055, 2.4);
        }
    };
    
    DUMP(sRGB(b.r / 255.0));
    
    auto isRGB = [](double u) {
        if (u < 0.0031308) {
            return 12.92 * u;
        } else {
            return 1.055 * pow(u, 1.0/2.4) - 0.055;
        }
    };
    
    DUMP(isRGB(0.5) * 255.0);

    
    auto n = 1024;
    image a(n, n);
    
    pixel purple(255, 0, 255, 255);
    
    auto m = 64;
    for (int i = 0; i != n; i += m)
        for (int j = 0; j != n; j += 2) {
            a(i, j) = purple;
            a(j, i) = purple;
        }
    
    // to_png(a, "/Users/acsearle/Downloads/textures/scaffold.png");
    auto s = _string_from_file("/Users/acsearle/Downloads/textures/symbols.json");
    auto b = json::from(s.data(), s.data() + s.size());
    b._ptr->debug();
    printf("\n");
    
    for (int i = 0; i != b["names"].size(); ++i) {
        json const& c = b["names"][i];
        for (int j = 0; j != c.size(); ++j) {
            printf("%s ", c[j].as_string());
        }
        printf("\n");
    }
 
} // namespace foo


//int main_projections(int argc, char** argv);



int main(int argc, char** argv) {
    //manic::foo();
    //main_projections( argc,  argv);
    
    
}
*/

/*
#include "thing.hpp"

int main( int argc, char** argv) {
    using namespace manic;
    
    world w;
    
    w.simulate();
    
}
*/

/*
#include <cassert>
#include <iostream>

#include <ft2build.h>
#include FT_FREETYPE_H

#define DUMP(X) do { std::cout << #X << " -> " << (X) << std::endl; } while(false)

#include "bit.hpp"

#include "hash.hpp"

void foo() {
    
    FT_Library ft;
    FT_Error e = FT_Init_FreeType(&ft);
    assert(!e);
    
    FT_Face face;
    e = FT_New_Face(ft,
                    "/Users/acsearle/Downloads/textures/Futura Medium Condensed.otf",
                    0,
                    &face);
    assert(!e);
    
    DUMP(face->num_glyphs);
    DUMP(face->num_fixed_sizes);
    DUMP(face->available_sizes);
    
    FT_Set_Pixel_Sizes(face, 0, 32);
    
    FT_Load_Char(face, 'O', FT_LOAD_RENDER);
    
    auto& b = face->glyph->bitmap;
    
    DUMP(face->glyph->bitmap_left);
    DUMP(face->glyph->bitmap_top);
    DUMP(face->glyph->advance.x);
    for (int i = 0; i != b.rows; ++i) {
        for (int j = 0; j != b.width; ++j)
            std::cout <<  (int) b.buffer[i * b.pitch + j];
        std::cout << std::endl;
    }


}

int main(int argc, char** argv) {
    //foo();
    
    std::cout << std::bitset<64>(manic::morton2(12345678)) << std::endl;
    std::cout << manic::morton2_reverse(manic::morton2(12345678)) << std::endl;

    return 0;
}

 */
