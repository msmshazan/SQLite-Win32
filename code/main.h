#if !defined(MAIN_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Mohamed Shazan $
   $Notice: All Rights Reserved. $
   ======================================================================== */
#ifdef __cplusplus
extern "C"
#endif
{

#define local_persist static
#define internal static
#define global_variable static
    
#include <windows.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <float.h>
#include <time.h>
#include <limits.h>
#include <assert.h>
#include <sqlite3.h>
    
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef float real32;
typedef double real64;

typedef int8 s8;
typedef int8 s08;
typedef int16 s16;
typedef int32 s32;
typedef int64 s64;
typedef int32 s32;
typedef bool32 b32;

typedef uint8 u8;
typedef uint8 u08;
typedef uint16 u16;
typedef uint32 u32;
typedef uint64 u64;

typedef real32 r32;
typedef real64 r64;

typedef intptr_t intptr;
typedef uintptr_t uintptr;

typedef size_t memory_index;

#define Real32Maximum FLT_MAX

struct win32_offscreen_buffer {
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height; 
    int Pitch;
    int BytesPerPixel;
};

struct win32_window_dimension {
    int Width;
    int Height;
};


}

#define MAIN_H
#endif
