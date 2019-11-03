//
//  thing.hpp
//  mania
//
//  Created by Antony Searle on 22/10/19.
//  Copyright © 2019 Antony Searle. All rights reserved.
//

#ifndef thing_hpp
#define thing_hpp

#include "matrix.hpp"
#include "space.hpp"
#include "vector.hpp"

#include "rleq.hpp"

namespace manic {

struct entity {
    
    u64 type; // make the type explicit rather than screw with vtbl id
    
    // MCU has a 2d location in a plane of memory cells, instead of a function
    // pointer into 1d memory
    
    i64 x;
    i64 y;
    
    // MCU has a 'microstate' that allows it to perform operations over multiple
    // cycles.  The most common is being obstructed, where it must wait for
    // the cell ahead to clear before it can move on.  It can also be used to
    // wait for conditions to be met (input cell nonzero, output cell zero)
    // that can be used to construct locks, barriers, wait_load, wait_store.
    // Never use the microstate to smuggle information between cells; this is
    // what the registers are for, and nonzero values have different meanings
    // in different cells / instructions anyway.  The microstate is always
    // zero when entering and leaving a new cell.  Without the microstate, the
    // MCU would always attempt to execute the whole instructon each cycle while
    // blocked.  This is not always desired.
    
    u64 s; // 0: travelling; 1: obstructed; other: instruction defined
    
    // Operations may wait for some conditions to be met, mutate their own state
    // and that of the world, and then wait for more conditions to be met before
    // moving on.  For example, a LOCK instruction waits until a cell is zero,
    // then writes one to it, then proceeds.  A BARRIER instruction decrements
    // a cell, then waits until the cell is zero, then proceeds.
    
    // If MCUs are physical, the operations must also reserve and release cells
    // as they travel through them.  On execution, if state is zero, release
    // the "from" cell indicated by D, perform instruction, then if not
    // waiting, acquire the "to" cell indicated by D.
    
    // MCU has multiple registers.  These let it carry state as it moves around
    //     A:  The accumulator register.  Instructions involving two locations
    //         usually include accumulator as one of them.  For example,
    //         addition is perfromed by adding a value from a memory cell or
    //         register to the accumulator, A = A + *addr
    //     B:  General-purpose register
    //     C:  General-purpose register
    //     D:  The direction register.  The last two bits control the direction
    //         the MCU is travelling (0123 -> NESW).  Conditional instructions
    //         like "<" write their output to the direction register, as in
    //         right-turn-if-less-than D += (A < *addr)
    
    u64 a; // accumulator
    u64 b;
    u64 c;
    u64 d; // direction
    
    // Each tick, the MCU reads its instruction from the cell it is "at".  The
    // instruction may address only the diagonally adjacent cells for read write
    // etc.
    //
    // Code and data are mingled together, but diagonal access means it is easy
    // to keep them locally separate on two complementary grids.
    
    u64 i; // identity, serial number
    
    static entity* make() {
        entity* p = (entity*) calloc(1, sizeof(entity));
        p->s = /* instruction::newborn*/ 0x0A00'0000'0000'0000ull;
        return p;
    }
    
};

inline std::ostream& operator<<(std::ostream& s, const manic::entity& x) {
    return (s << std::hex << "entity{x=" << x.x << ",y=" << x.y << ",a=" << x.a << ",d=" << x.d << "}");
}

struct world {
    
    // State of world
    
    // The world is a 2D grid of memory cells.  Entities are "above" this plane,
    // terrain is "below" (and mostly cosmetic).
    // The interpretation of the value is complex.  It may either be a value
    // or an opcode.  One bit is reserved to indicate if an MCU may enter the
    // cell; MCUs lock and unlock cells using this mechanism as they move
    // around the board, preventing collisions (like Factorio, deadlock is
    // possible if there are N MCUs in a loop of N nodes; unlike Factorio, the
    // MCUs have unit extent and thus cannot deadlock #-shaped intersections)
    
    space<u64> _board;
    
    // MCUs take turns to act on the board.  A queue is the obvious data
    // structure.  However, we want to spread the computational load across
    // each frame, and we want MCUs to move at a predictable speed.
    // * If we exec all MCUs in one frame, load spike
    // * If we dispatch to another CPU, we have to clone data
    // * If we process some one MCU each frame, they get slower as there are
    //   more (even if CPU is not loaded)
    // * If we process some fraction each frame, adding more MCUs makes the
    //   existing ones jump forward a bit
    //
    // Idea: each MCU gets a turn every N (=64) ticks.  There are 64 queues,
    // each MCU lives in one, and each tick executes a whole queue in order.
    // A potential problem is if the queues become unbalanced; balancing them
    // constrains what we can guarantee about the relative order of forked
    // MCUs?
    
    vector<entity*> _entities[64];
    
    world();
    
    u64 counter = 0;
    
    void tick();
    
    void exec(entity&);
    
    
}; // struct world


namespace instruction {

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



const u64    OCCUPIED_FLAG = 0x8000'0000'0000'0000ull;
const u64 INSTRUCTION_FLAG = 0x4000'0000'0000'0000ull;
const u64        ITEM_FLAG = 0x2000'0000'0000'0000ull;
const u64   CONSERVED_FLAG = 0x1000'0000'0000'0000ull;

constexpr bool is_occupied(u64 x) { return x & OCCUPIED_FLAG; }
constexpr bool is_instruction(u64 x) { return x & INSTRUCTION_FLAG; }
constexpr bool is_item(u64 x) { return x & ITEM_FLAG; }
constexpr bool is_conserved(u64 x) { return x & CONSERVED_FLAG; }

constexpr bool is_vacant(u64 x) { return !is_occupied(x); }

inline void occupy(u64& x) { x |=  OCCUPIED_FLAG; }
inline void vacate(u64& x) { x &=~ OCCUPIED_FLAG; }

const u64        TAG_MASK = 0x7000'0000'0000'0000ull;
const u64      NUMBER_TAG = 0x0000'0000'0000'0000ull;
const u64 INSTRUCTION_TAG = INSTRUCTION_FLAG;
const u64       GHOST_TAG = ITEM_FLAG;
const u64    MATERIAL_TAG = OCCUPIED_FLAG | ITEM_FLAG | CONSERVED_FLAG;

constexpr u64 ghost_of(u64 x) { return x &~ (OCCUPIED_FLAG | CONSERVED_FLAG); }

const u64 VALUE_MASK = 0x0FFF'FFFF'FFFF'FFFFull;

constexpr u64 value_of(u64 x) { return x & VALUE_MASK; }

// Opcode structure definitions

const u64 MICROSTATE_SHIFT = 58;
const u64 MICROSTATE_MASK = 0x0C00'0000'0000'0000ull;

constexpr u64 OPCODE_SHIFT = 32;
const u64 OPCODE_MASK  = 0x03FF'FFFF'0000'0000ull;
const u64 OPCODE_BASIS = 0x0000'0001'0000'0000ull;

const u64 ADDRESS_SHIFT = 0;
const u64 ADDRESS_MASK = 0x0007ull;

const u64 REGISTER_FLAG = 0x0004ull;

constexpr u64 opcode_of(u64 x) {
    assert(is_instruction(x));
    return (x & OPCODE_MASK);
}
enum opcode_enum : u64 {
    noop = 0ull << OPCODE_SHIFT,
    load = 1ull << OPCODE_SHIFT,
    store = 3ull << OPCODE_SHIFT,
    add = 4ull << OPCODE_SHIFT,
    sub = 5ull << OPCODE_SHIFT,
    bitwise_and = 6ull << OPCODE_SHIFT,
    bitwise_or = 7ull << OPCODE_SHIFT,
    bitwise_xor = 8ull << OPCODE_SHIFT,
    decrement = 9ull << OPCODE_SHIFT,
    decrement_saturate = 10ull << OPCODE_SHIFT,
    increment = 11ull << OPCODE_SHIFT,
    increment_saturate = 12ull << OPCODE_SHIFT,
    flip_increment = 13ull << OPCODE_SHIFT,
    flip_decrement = 14ull << OPCODE_SHIFT,
    swap = 15ull << OPCODE_SHIFT,
    kill = 16ull << OPCODE_SHIFT,
    fork = 17ull << OPCODE_SHIFT,
    conservative_or = 18ull << OPCODE_SHIFT,
    conservative_and = 19ull << OPCODE_SHIFT,
    less_than = 20ull << OPCODE_SHIFT,
    equal_to = 21ull << OPCODE_SHIFT,
    clear = 22ull << OPCODE_SHIFT,
    compare = 23ull << OPCODE_SHIFT,
    and_complement_of = 24ull << OPCODE_SHIFT,
    dump = 25ull << OPCODE_SHIFT,
    halt = 26ull << OPCODE_SHIFT,
    barrier = 27ull << OPCODE_SHIFT,
    mutex = 28ull << OPCODE_SHIFT,
    greater_than = 29ull << OPCODE_SHIFT,
    less_than_or_equal_to = 30ull << OPCODE_SHIFT,
    greater_than_or_equal_to = 31ull << OPCODE_SHIFT,
    not_equal_to = 32ull << OPCODE_SHIFT,
    complement = 33ull << OPCODE_SHIFT,
    negate = 34ull << OPCODE_SHIFT,
    shift_left = 35ull << OPCODE_SHIFT,
    shift_right = 36ull << OPCODE_SHIFT,
};

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

const u64 STATE_MASK = 0xC000'0000;

inline u64 opcode(opcode_enum op, address_enum ad = northeast) {
    assert(op == (op & OPCODE_MASK));
    assert(ad == (ad & ADDRESS_MASK));
    return INSTRUCTION_FLAG | op | ad;
}

} // namespace instruction

} // namespace manic

#endif /* thing_hpp */
