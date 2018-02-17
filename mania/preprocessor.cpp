//
//  preprocessor.cpp
//  mania
//
//  Created by Antony Searle on 4/1/18.
//  Copyright © 2018 Antony Searle. All rights reserved.
//

// #include "preprocessor.hpp"

#define STRINGIZE(x) STRINGIZE_1(x)
#define STRINGIZE_1(x) # x

#define CONCATENATE(X, Y) CONCATENATE_2(X, Y)
#define CONCATENATE_2(X, Y) X ## Y

#define HEAD(a, ...) a
#define TAIL(a, ...) (__VA_ARGS__)

#define PEEL(...) __VA_ARGS__

#define GET(i, ...) CONCATENATE(GET_, i)(__VA_ARGS__)
#define GET_0(a, ...) a
#define GET_1(a, ...) GET_0(__VA_ARGS__)
#define GET_2(a, ...) GET_1(__VA_ARGS__)
#define GET_3(a, ...) GET_2(__VA_ARGS__)
#define GET_4(a, ...) GET_3(__VA_ARGS__)
#define GET_5(a, ...) GET_4(__VA_ARGS__)
#define GET_6(a, ...) GET_5(__VA_ARGS__)
#define GET_7(a, ...) GET_6(__VA_ARGS__)
#define GET_8(a, ...) GET_7(__VA_ARGS__)

#define COUNT(...) GET_8(__VA_ARGS__, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define INC(X) CONCATENATE(INC_, X)
#define INC_0 1
#define INC_1 2
#define INC_2 3
#define INC_3 4
#define INC_4 5
#define INC_5 6
#define INC_6 7
#define INC_7 8

#define DEC(X) CONCATENATE(DEC_, X)
#define DEC_1 0
#define DEC_2 1
#define DEC_3 2
#define DEC_4 3
#define DEC_5 4
#define DEC_6 5
#define DEC_7 6
#define DEC_8 7

//STRINGIZE(123)

//COUNT(a, b, c)

#define JOIN(A, B) (PEEL A, PEEL B)

#define AB(A, B) A B

//AB(PEEL, JOIN((a, b), (c, d)))

// INVOKE(f, x, y) -> f(x, y)
#define INVOKE(F, ...) F(__VA_ARGS__)

// MAP(f, (x, y)) (f(x), f(y))
#define MAP(F,L) (INVOKE(CONCATENATE(MAP_, COUNT L), F, PEEL L))

#define MAP_0(F)
#define MAP_1(F, A) F(A)
#define MAP_2(F, A, ...) F(A), MAP_1(F, __VA_ARGS__)
#define MAP_3(F, A, ...) F(A), MAP_2(F, __VA_ARGS__)
#define MAP_4(F, A, ...) F(A), MAP_3(F, __VA_ARGS__)

//MAP(f,(a,b,c))

#define ITERATE(...)

//INC(4)

#define BOOL(X) CONCATENATE(BOOL_, X)
#define BOOL_0 0
#define BOOL_1 1
#define BOOL_2 1
#define BOOL_3 1
#define BOOL_4 1
#define BOOL_5 1
#define BOOL_6 1
#define BOOL_7 1
#define BOOL_8 1

#define IF(X, T, F) CONCATENATE(IF_, BOOL(X))(T, F)
#define IF_0(T, F) F
#define IF_1(T, F) T

#define COMMA() ,
#define EMPTY()

//IF(1, COMMA,EMPTY)()






















