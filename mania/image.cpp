//
//  image.cpp
//  mania
//
//  Created by Antony Searle on 16/7/17.
//  Copyright © 2017 Antony Searle. All rights reserved.
//

#include "image.hpp"

#include <vector>
#include <png.h>

#include "vec.hpp"

template<typename Iterator>
struct span {
    Iterator _begin, _end;
    auto size() { _end - _begin; }
    decltype(auto) operator[](std::ptrdiff_t i) const { return _begin[i]; }
};

namespace gl {
    
    using rgba = gl::vec<unsigned char, 4>;
    
};

struct image {
    std::vector<gl::rgba> _data;
    std::size_t width, height;
};


void png_thing() {
    
    /*
    const char* file_name = "thing.png";
    
    FILE *fp = fopen(file_name, "rb");
    if (!fp)
    {
        //return (ERROR);
    }
    
    if (fread(header, 1, number, fp) != number)
    {
        //return (ERROR);
    }
    
    is_png = !png_sig_cmp(header, 0, number);
    if (!is_png)
    {
        //return (NOT_PNG);
    }
     */
    
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    png_infop info_ptr = png_create_info_struct(png_ptr);
    
    FILE* fp = fopen("/Users/acsearle/Downloads/basn6a08.png", "rb");
    
    png_init_io(png_ptr, fp);
    
    png_read_png(png_ptr, info_ptr, 0, nullptr);
    
    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);
    
    auto width = png_get_image_width(png_ptr, info_ptr);
    auto height = png_get_image_height(png_ptr, info_ptr);
    
    auto bit_depth        = png_get_bit_depth(png_ptr,
                                         info_ptr);
    
    auto color_type       = png_get_color_type(png_ptr,
                                          info_ptr);
    
    auto interlace_type   = png_get_interlace_type(png_ptr,
                                              info_ptr);
    
    auto compression_type = png_get_compression_type(png_ptr,
                                                info_ptr);
    
    auto filter_method    = png_get_filter_type(png_ptr,
                                           info_ptr);
    
    auto channels = png_get_channels(png_ptr, info_ptr);
    /*
    channels       - number of channels of info for the
        color type (valid values are 1 (GRAY,
                                        PALETTE), 2 (GRAY_ALPHA), 3 (RGB),
                    4 (RGB_ALPHA or RGB + filler byte))
      */
    
    auto rowbytes = png_get_rowbytes(png_ptr, info_ptr);
    
    //rowbytes       - number of bytes needed to hold a row
    
    auto signature = png_get_signature(png_ptr, info_ptr);
    
    /*
    signature      - holds the signature read from the
    file (if any).  The data is kept in
    the same offset it would be if the
        whole signature were read (i.e. if an
                                   application had already read in 4
                                   bytes of signature before starting
                                   libpng, the remaining 4 bytes would
                                   be in signature[4] through signature[7]
                                   (see png_set_sig_bytes())).
    */
    
    auto row = (unsigned char*) *row_pointers;
    
    for (int i = 2 * 32; i != 4 * 32; ++i)
        std::cout << (int) row[i] << " ";
    
    
    fclose(fp);
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    
    
    
}
