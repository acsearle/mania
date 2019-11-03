//
//  elements.hpp
//  mania
//
//  Created by Antony Searle on 25/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef elements_hpp
#define elements_hpp

#include "instruction.hpp"

namespace manic {
    
namespace element {
    
constexpr u64 _make_elements_enum(u64 i) {
    using namespace instruction;
    return MATERIAL_TAG | ((1ull << (i)) & VALUE_MASK);
}

enum elements_enum : u64 {
    
    // more than we can pack

    hydrogen = _make_elements_enum(0),
    carbon = _make_elements_enum(1),
    oxygen = _make_elements_enum(2),
    sulfur = _make_elements_enum(3),
    iron = _make_elements_enum(4),
    silicon = _make_elements_enum(5),

};

enum materials_enum : u64 {
    
    kerosene = hydrogen | carbon,
    water = hydrogen | oxygen,
    carbon_dioxide = carbon | oxygen,
    hematite = iron | oxygen,
    magnetite = iron | oxygen | silicon,
    
};
    
} // namespace element

} // namespace manic

#endif /* elements_hpp */
