#ifndef INT_H_
#define INT_H_

#include <cstdint>

// Unsigned integers
typedef unsigned char  u8;  // 1 byte long
typedef unsigned short u16; // 2 bytes long
typedef unsigned int   u32; // 4 bytes long
typedef unsigned long  u64; // 8 bytes long

// Signed integers
typedef signed char  s8;  // 1 byte long
typedef signed short s16; // 2 bytes long
typedef signed int   s32; // 4 bytes long
typedef signed long  s64; // 8 bytes long

/** Endian detection, returns true if the *
 *  the system is little-endian.          */
inline static bool _LITTLE_ENDIAN(){
    u16 v = 1;

    return *((u8 *) &v) == 1;
}

/** Flips the endian of a value. */
template<typename T>
static void _FLIP_ENDIAN(T* value){
    u8 *bytes = (u8 *) value;

    for(size_t i = 0; i < sizeof(T) / 2; ++i){
        u8 one = bytes[i];
        u8 two = bytes[sizeof(T) - 1 - i];

        bytes[i] = two;
        bytes[sizeof(T) - 1 - i] = one;
    }
}

#endif
