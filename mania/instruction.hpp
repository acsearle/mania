//
//  instruction.hpp
//  mania
//
//  Created by Antony Searle on 3/11/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef instruction_hpp
#define instruction_hpp

#include <cassert>

#include "common.hpp"

namespace manic::instruction {

// desiderata for carving up the 64-bit words that occupy memory cells and
// registers:
//
// * occupied flag is orthogonal to everything
//   * occupied flag must be preserved; it's useful to change numbers and
//     opcodes under entities, but we can't just overwrite the flag or we
//     corrupt mutual exclusion.  This means all math on the cell has to be
//     appropriately masked.
//   * meaningless in registers?
//   * an entity can drive over a number, occupying the cell.  Therefore the
//     property of occupation is orthogonal.  We can only write a physical
//     object into an unoccupied cell, casuing it to become occupied.  We
//     can SWAP a physical object into an occupied cell ONLY if it is holding
//     a value, ghost or opcode (non-physical) thing
//
// * physical flag implies occupied but not vice versa
//   * operations on physical values must be very restricted.  Basically they
//     must be conserved.  This means no duplicating or overwriting.  The
//     representation chosen for physical things determines other restrictions;
//     for example, total-number-of-set-bits-at-a-position must be conserved
//     for the atomic representation.
//   * physical objects obstruct MCUs from entering them.  This works well with
//     diagonal access.  It also means we don't have to deal with opcodes under
//     physical objects, the object just overwrites it.
//
// * numbers must include zero, negative and positive numbers.  This implies
//   that slicing up the space of values should be done with high bits, and
//   that there can't be a single tag for numbers, e.g. they will begin with 00
//   and 11.  numbers can be freely created, destroyed, mutated.  Important to
//   preserve zero, one, minus one as their natural representations.  Zero does
//   double duty as a number, as as the absence of opcode, the absence of
//   physical material, etc. etc.
//       x00xx -> number
//       x01xx -> physical object
//       x10xx -> opcode
//       x11xx -> number
//
// * opcodes are a kind of thing.  some have an associated address, which
//   should be low bits represented the same way as direction, so we can rotate
//   them.  some have an associated register, which should also be low bits,
//   but rotation is a worse metaphor for them.
//   * unclear if immediate values are a good idea.  they complicate the
//     interface greatly, when we could just read from neighbouring cell.  on
//     the other hand, CLEAR is convenient, as is loading directions directly
//     into the last two bits of D.  These can be special cases, again for D
//     using the low two bits
//   * easy to change the address / literal associated with opcode, easy to
//     copy opcodes, hard to accidentally mutate the opcodes themselves; this
//     implies offsetting used opcodes into the middle of their valid range,
//     implies offsetting canonical address codes and direction codes into
//     the middle of their valid range too.  Then adding and subtracting small
//     amounts does't carry up into affecting opcodes themselves
//   * carefully group opcodes so that they are paired with their inverses etc
//     and provide easy access to an opcode mutator constant, a large value
//     that can be added, xored etc. with opcodes to turn opcodes into related
//     opcodes easily
//     * arithmetic opposites like ADD and SUB, INC and DEC
//     * logical opposites like EQ and NE
//       * does < pair with > or >= "!<"?  maybe both on different axes
//   * multiple opcodes for same operation; for example, INC and DEC are natural
//     pairs, but COMPLEMENT is its own inverse
//   * are opcodes just obscure numbers?  Tempting but no.
//
// * the codes for physical objects should encode their properties in some way;
//   ideas for this include
//   * bits represent the presence of atoms, i.e. iron oxides Fe_x O_x are
//     represented as (1 << (26 - 1)) | (1 << (8 - 1))
//     * chemical processing is then expressible as operations like
//       CONSERVATIVE_AND
//     * this does not allow distinguishing different chemical formulae, or
//       mixed materials;
//     * everything biological is C_x H_x O_x
//   * bits tag the presence of some quality, such as "extruded" or "gaseous";
//     processing is then representable as operations like (Fe | machined)
//   * just enumerate different things and allow no bit manipulations;
//     processing is then a table lookup of known recipes
//   * subfields encode quantity (IRON | (200 << QUANITY_SHIFT)
//   * subfields encode quality / tech level (CIRCUIT | (4 << QUALITY_SHIFT))
//
// * we need ghost things; we need to be able to set up filters that say is
//   item equal to e.g. circuit board, without needing to use a real circuit
//   board (which we may have none of yet!).  Ghost things can be manipulated
//   like numbers, which (like opcodes) means the representation becomes
//   visible to the user, and should be comprehensible
//   * All the operations available on numbers have to be available on ghosts
//     too, so representation should make it easy to check if number or ghost
//     or opcode and to preserve ghostliness?
//   * Comparisons don't distinguish between physical and ghost objects.  So
//     we can say ACCUMULATOR == ghost_COAL
//   * How do we make ghosts from materials we drive by?  Perhaps LOAD and STORE
//     traffic in ghosts, only SWAP can get physical materials in and out of
//     the accumulator.  Makes sense because LOAD and STORE can't satisfy their
//     postcondition otherwise.
//   * GHOSTS may result from operations involving physical things, like
//         ACCUMULATOR = 1 -> 1
//         ACCUMULATOR += IRON -> ghost_COBALT
//   * Are ghost opcodes a thing?  No.
//
// * Is it always clear when numbers, ghost and opcodes interact what the type
//   of the output is?  ghost plus opcode seems meaningless.
//
// * Can registers other than accumulator hold physical object?  Can direction
//   register hold physical object?  Is a physical object oriented with last
//   two bits like opcode?
//   * Can you hold multiple physical objects by putting them in multiple
//     registers?  No, or we are not a "truck" carrying "[bulk material]".  And
//     we don't want to check all registers, so they must go into a particular
//     register.
//     * ACCUMULATOR is the obvious choice, but this means we can't "think"
//       while "carrying".
//       * Consistent (everything loaded into / unloaded from ACCUMULATOR)
//       * Keeps the number of registers and the number of opcodes small.
//       * Implementation of operations will be more complex as they have to
//         check for physical values and then do nothing.
//     * B[UCKET?] or C is another choice.  Physical values can only get loaded
//       into B.
//       * This means we can "think" while we are "carrying"
//       * We can't do comparisons between B and *addr without more opcodes.
//         This is perhaps the most common kind of "thinking": if I am
//         carrying [iron ore] turn onto the smelter road.
//       * If we keep physical objects out of accumulator, math becomes simpler
//         perhaps
//     * DIRECTION is ruled out; can't see any advantage here; coal always goes
//       west???
//
// * What is the difference between number zero, opcode zero, item zero and
//   ghost zero?  They all compare equal, but have different answers for tags.
//   Do we need to check we never write out a physical zero?  What if we
//   decrement a ghost or opcode to zero, then increment it back up, does it
//   become a number?  Yes, that seems okay.
//
// * ghosts and items have to be comparable, but it's better if other
//   heterogeneous combinations aren't comparable.  Does this mean we mask
//   out only some of the tag bits for equality comparison?


// allocation of word:
// 63: obstruction flag
//
// type tag:
//     physical
//     ghost
//     opcode
//     number

// ghosts and physicals must be paired with equal range

// This is nice, it means everything has the same 60-bit space available to it,
// but it also means that the codes are split, we have to select by masking

//     x000 -> number
//     x001 -> opcode
//     x010 -> opcode
//     x011 -> physical
//     x100 -> physical
//     x101 -> ghost
//     x110 -> ghost
//     x111 -> number

// The OCCUPIED bit has already screwed us out of natural negative numbers,
// maybe just accept that fact?

//     x00 -> number
//     x01 -> opcode
//     x02 -> item, ghostly
//     x03 -> item, material

// We can regain flag interpretations if we give up some of the space
// OPCODE ITEM PHYSICAL
//
//     000 -> number
//     001 ->
//     010 -> ghost
//     011 -> material
//     100 -> opcode
//     101 ->
//     110 ->
//     111 ->

// CONSERVED ITEM OPCODE
//
//    000 -> number
//    001 -> opcode
//    010 -> ghost
//    011 -> ? ghostly opcode item
//    100 -> ? constant
//    101 -> ? constant opcode
//    110 -> physical thing
//    111 -> ? physical opcode

// One bit for questions we want to ask:
// Is it an opcode?
// Is it conserved?
// Is it an item or a ghost?
// Math never touches these bits.

// We need to generate ghosts from physical items, but otherwise the tag never
// transforms

// Use bit fields?  No, too implementation defined :(



constexpr u64    OCCUPIED_FLAG = 0x8000'0000'0000'0000ull;
constexpr u64 INSTRUCTION_FLAG = 0x4000'0000'0000'0000ull;
constexpr u64        ITEM_FLAG = 0x2000'0000'0000'0000ull;
constexpr u64   CONSERVED_FLAG = 0x1000'0000'0000'0000ull;

constexpr bool is_occupied(u64 x) { return x & OCCUPIED_FLAG; }
constexpr bool is_instruction(u64 x) { return x & INSTRUCTION_FLAG; }
constexpr bool is_item(u64 x) { return x & ITEM_FLAG; }
constexpr bool is_conserved(u64 x) { return x & CONSERVED_FLAG; }

constexpr bool is_ghost(u64 x) { return is_item(x) && !is_conserved(x); }

constexpr bool is_vacant(u64 x) { return !is_occupied(x); }

inline void occupy(u64& x) { x |=  OCCUPIED_FLAG; }
inline void vacate(u64& x) { x &=~ OCCUPIED_FLAG; }

constexpr u64        TAG_MASK = 0x7000'0000'0000'0000ull;
constexpr u64      NUMBER_TAG = 0x0000'0000'0000'0000ull;
constexpr u64 INSTRUCTION_TAG = INSTRUCTION_FLAG;
constexpr u64       GHOST_TAG = ITEM_FLAG;
constexpr u64    MATERIAL_TAG = OCCUPIED_FLAG | ITEM_FLAG | CONSERVED_FLAG;

constexpr u64 ghost_of(u64 x) { return x &~ (OCCUPIED_FLAG | CONSERVED_FLAG); }

constexpr u64 VALUE_MASK = 0x0FFF'FFFF'FFFF'FFFFull;
constexpr u64 FLAGS_MASK = 0xF000'0000'0000'0000ull;

constexpr u64 value_of(u64 x) { return x & VALUE_MASK; }

constexpr i64 signed_of(u64 x) {
    x &= VALUE_MASK;
    if (x & 0x0800'0000'0000'0000) {
        x &= 0xF000'0000'0000'0000;
    }
    return static_cast<i64>(x);
}


// Opcode structure definitions

constexpr u64 MICROSTATE_SHIFT = 58;
constexpr u64 MICROSTATE_MASK = 0x0C00'0000'0000'0000ull;

constexpr u64 OPCODE_SHIFT = 32;
constexpr u64 OPCODE_MASK  = 0x03FF'FFFF'0000'0000ull;
constexpr u64 OPCODE_BASIS = 0x0000'0001'0000'0000ull;

constexpr u64 ADDRESS_SHIFT = 0;
constexpr u64 ADDRESS_MASK = 0x0007ull;

constexpr u64 REGISTER_FLAG = 0x0004ull;

// To compare values, we want to include some flag and exclude others
// * OCCUPIED_FLAG is excluded so we can compare things while another entity runs over them
// * INSTRUCTION_FLAG is included so instructions are not comparable to numbers, ghosts or items
// * ITEM_FLAG is included so items and ghosts are not comparable to instructions or numbers
// * CONSERVED_FLAG is excluded so ghosts are comparable to items
// Signed comparisons become problematic?
constexpr u64 EQUALITY_MASK = ~(OCCUPIED_FLAG | CONSERVED_FLAG);

constexpr u64 opcode_of(u64 x) {
    assert(is_instruction(x));
    return (x & OPCODE_MASK);
}

// enums are quite awful!

constexpr u64 _opcode_enum_maker(u64 x) {
    return x << OPCODE_SHIFT;
}

enum opcode_enum : u64 {
    noop = _opcode_enum_maker( 0ull ),
    load = _opcode_enum_maker( 1ull ),
    store = _opcode_enum_maker( 2ull ),
    add = _opcode_enum_maker( 3ull ),
    sub = _opcode_enum_maker( 4ull ),
    bitwise_and = _opcode_enum_maker( 5ull ),
    bitwise_or = _opcode_enum_maker( 6ull ),
    bitwise_xor = _opcode_enum_maker( 7ull ),
    decrement = _opcode_enum_maker( 8ull ),
    decrement_saturate = _opcode_enum_maker( 9ull ),
    increment = _opcode_enum_maker( 10ull ),
    increment_saturate = _opcode_enum_maker( 11ull ),
    flip_increment = _opcode_enum_maker( 12ull ),
    flip_decrement = _opcode_enum_maker( 13ull ),
    swap = _opcode_enum_maker( 14ull ),
    kill = _opcode_enum_maker( 15ull ),
    fork = _opcode_enum_maker( 16ull ),
    conservative_or = _opcode_enum_maker( 17ull ),
    conservative_and = _opcode_enum_maker( 18ull ),
    less_than = _opcode_enum_maker( 19ull ),
    equal_to = _opcode_enum_maker( 20ull ),
    clear = _opcode_enum_maker( 21ull ),
    compare = _opcode_enum_maker( 22ull ),
    and_complement_of = _opcode_enum_maker( 23ull ),
    dump = _opcode_enum_maker( 24ull ),
    halt = _opcode_enum_maker( 25ull ),
    barrier = _opcode_enum_maker( 26ull ),
    mutex = _opcode_enum_maker( 27ull ),
    greater_than = _opcode_enum_maker( 28ull ),
    less_than_or_equal_to = _opcode_enum_maker( 29ull ),
    greater_than_or_equal_to = _opcode_enum_maker( 30ull ),
    not_equal_to = _opcode_enum_maker( 31ull ),
    complement = _opcode_enum_maker( 32ull ),
    negate = _opcode_enum_maker( 33ull ),
    shift_left = _opcode_enum_maker( 34ull ),
    shift_right = _opcode_enum_maker( 35ull ),
    turn_back = _opcode_enum_maker( 36ull ), // replace with add D immediate, set D immediate?
    turn_north = _opcode_enum_maker( 37ull ),
    turn_east = _opcode_enum_maker( 38ull ),
    turn_south = _opcode_enum_maker( 39ull ),
    turn_west = _opcode_enum_maker( 40ull ),
    turn_cw = _opcode_enum_maker( 41ull ),
    turn_cccw = _opcode_enum_maker( 42ull ),
};

constexpr u64 _opcode_enum_size = 42;

enum address_enum : u64 {
    northeast = 0,
    southeast,
    southwest,
    northwest,
    register_a,
    register_b,
    register_c,
    register_d,
};

// state enum needs to nicely mesh with the opcode bits.  If the top two
// bits of the u64 are OCCUPIED_FLAG and INSTRUCTION_FLAG, neither are
// needed?
enum state_enum : u64 {
    entering = 0x0000'0000'0000'0000ull,
    waiting  = 0x0400'0000'0000'0000ull,
    exiting  = 0x0800'0000'0000'0000ull,
    newborn  = 0x0A00'0000'0000'0000ull,
};

inline u64 opcode(opcode_enum op, address_enum ad = northeast) {
    assert(op == (op & OPCODE_MASK));
    assert(ad == (ad & ADDRESS_MASK));
    return INSTRUCTION_FLAG | op | ad;
}

} // namespace manic::instruction

#endif /* instruction_hpp */
