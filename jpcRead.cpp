#include <fstream>
#include <iostream>
#include <vector>
#include <iomanip>
#include <algorithm>

using namespace std;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef float    f32;
typedef double   f64;
typedef vector<u8> data_t;

typedef vector<pair<int, int>> idOffs;
int mem_size;
vector<pair<string, int>> textures;
vector<pair<int, int>> bspOffsets;

// [start, end] inclusive
const idOffs ADDED_SWORD_EFFECT = {
    {0x56f, 0x575}, // Base
    {0x757, 0x759}, // TMS and MS hero
    {0xA84, 0xA86}, // Goddess Hero
    {0xA87, 0xA89}, // Long/White Hero
};

const idOffs FULL_CHARGE = {
    {0x568, 0x56B}, // Base
    {0x752, 0x756}, // TMS and MS Hero
    {0xA7A, 0xA7E}, // Goddess Hero
    {0xA7F, 0xA83}, // Long/White Hero
};

const idOffs BLINKING_SWORDS = {
    {0x576, 0x57A}, // Base
    {0x75A, 0x75E}, // TMS and MS Hero
    {0xA8A, 0xA8E}, // Goddess Hero
    {0xA8F, 0xA93}, // Long/White Hero
};

FULL_CHARGE
{0x56f, 0x571 }
{0x757, 0x759}
{0xA87 ,0xA89}
{0xA84, 0xA86}

0x562, 0x563 
0x751
0x55F

const idOffs STAB_TRAILS = {
    {0x543, 0x547}, // Base
    {0x749, 0x74A}, // TMS or Hero Added
    {0x636, 0x63A}, // Lightning
};

const idOffs SPIN_ATTACKS = {
    {0x0E0, 0x0E1}, // Down Up Spin
    {0x0E2, 0x0E3}, // Up Down Spin
    {0x0E4, 0x0E5}, // horizontal Spin
    {0x73A, 0x73B}, // TMS / Heromode
    {0x8AE, 0x8AF}, // Lightning
};

const idOffs COLLISION_EFFECTS = {
    {0x541, 0x542}, // Base
    {0x748, 0x748}, // TMS / Hero
    {0x89D, 0x8A0}, // Lightning
};

const idOffs UNK_PARTICLES = {
    {0x559, 0x55B}, {0x55C, 0x55F}, {0x560, 0x560}, // Base
    {0x74C, 0x74E},  // Tms / Hero
    {0x633, 0x635},  // Lightning
};

