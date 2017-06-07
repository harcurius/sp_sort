#include <stdint.h>

#define true 1
#define false 0

#define ArrayCount(arr) (sizeof(arr) / sizeof(arr[0]))

#define Assert(expr) if(!expr){int *i = 0; *i = 0;}

typedef unsigned int u32;
typedef int b32;
typedef unsigned char b8;
typedef unsigned char u8;

typedef uintptr_t umm;

typedef struct v4
{
    float r,g,b,a;
}v4;

typedef struct input_key
{
    b8 Pressed;
    u8 Transition;
}input_key;

typedef struct input
{
    input_key Keys[256];
}input;

enum InputKeys
{
    Input_Left,
    Input_Right,
};
#define KeyDown(Key, Input) (Input->Keys[Key].Pressed && Input->Keys[Key].Transition)
#define KeyHeld(Key, Input)  (Input->Keys[Key].Pressed && !Input->Keys[Key].Transition)
#define KeyUp(Key, Input)    (!Input->Keys[Key].Pressed && Input->Keys[Key].Transition)

#define PLATFORM_ALLOCATE(name) void* name(umm Size)
typedef PLATFORM_ALLOCATE(platform_allocate);

#define PLATFORM_DEALLOCATE(name) void name(void *Memory)
typedef PLATFORM_DEALLOCATE(platform_deallocate);

#define PLATFORM_REALLOCATE(name) void* name(void *OldMemory, umm Size)
typedef PLATFORM_REALLOCATE(platform_reallocate);

#define UPDATE_AND_RENDER(name) void name(int Width, int Height, input *Input)
typedef UPDATE_AND_RENDER(update_and_render);

static platform_allocate *PlatformAllocate;
static platform_deallocate *PlatformDeallocate;
static platform_reallocate *PlatformReallocate;
static update_and_render *UpdateAndRender;

#include "sp_shared.c"
#include "sp_sort.c"