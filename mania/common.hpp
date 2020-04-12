//
//  common.hpp
//  mania
//
//  Created by Antony Searle on 24/2/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef common_hpp
#define common_hpp

#include <cstddef>
#include <cstdint>
#include <cmath>

namespace manic {

using i64 = std::int64_t;
using u64 = std::uint64_t;

using i32 = std::int32_t;
using u32 = std::uint32_t;

using i16 = std::int16_t;
using u16 = std::uint16_t;

using i8 = std::int8_t;
using u8 = std::uint8_t;

using f64 = std::double_t;
using f32 = std::float_t;

static_assert(sizeof(f64)== 8);
static_assert(sizeof(f32) == 4);

using usize = std::uintptr_t;
using isize = std::intptr_t;

using byte = std::byte;

static_assert(sizeof(usize) == sizeof(std::size_t));
static_assert(sizeof(isize) == sizeof(std::ptrdiff_t));

#define F(X) inline X operator"" _##X (unsigned long long x) { return ( X ) x; }

F(u8)
F(u16)
F(u32)
F(u64)
F(usize)

F(i8)
F(i16)
F(i32)
F(i64)
F(isize)

#undef F

} // namespace manic

#endif /* common_h */
