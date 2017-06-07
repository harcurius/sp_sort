#include "platform.c"
#include <SDL.h>
#include <GL/gl.h>
#include <malloc.h>
#include "opengl.c"
#include <stdio.h>

struct
{
    int Width;
    int Height;
}WinDim;

static b32 GlobalRunning;

static
PLATFORM_ALLOCATE(Linux_Allocate)
{
    void *Memory = malloc(Size);
    return(Memory);
}

static
PLATFORM_DEALLOCATE(Linux_Deallocate)
{
    if(Memory)
    {
        free(Memory);
    }
}

static
PLATFORM_REALLOCATE(Linux_Reallocate)
{
    void *NewMemory = realloc(OldMemory, Size);
    return(NewMemory);
}

static void
Linux_ResizeWindow(int Width, int Height)
{
    if(Height <= 0)
    {
        Height = 1;
    }
    WinDim.Width = Width;
    WinDim.Height = Height;
}

static void
Linux_ToggleFullscreen(SDL_Window *Window)
{
    // TODO(Sander): Better way to do this???
    static b32 IsFullscreen = false;
    u32 Flag = (IsFullscreen ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
    SDL_SetWindowFullscreen(Window, Flag);
	IsFullscreen = !IsFullscreen;
}

static SDL_GLContext
Linux_CreateRenderer(SDL_Window *Window)
{
	SDL_GLContext GLContext = SDL_GL_CreateContext(Window);
	if(SDL_GL_SetSwapInterval(1))
	{
		printf("SDL_GL_SetSwapInterval failed: %s\n", SDL_GetError());
	}
	return(GLContext);
}

static void
Linux_ProcessKeyMessage(input_key *NewState, b32 IsPressed)
{
    if(NewState->Pressed != IsPressed)
    {
        NewState->Pressed = IsPressed;
        ++NewState->Transition;
    }
}

static void
Linux_ProcessPendingMessages(SDL_Window *Window, input *Input)
{
    for(u32 KeyIndex = 0;
        KeyIndex < ArrayCount(Input->Keys);
        ++KeyIndex)
    {
        Input->Keys[KeyIndex].Transition = 0;
    }
    SDL_Event Event;
    while(SDL_PollEvent(&Event))
    {
        if(Event.type == SDL_QUIT)
        {
            GlobalRunning = 0;
        }
        else if(Event.type == SDL_WINDOWEVENT)
        {
            if(Event.window.event == SDL_WINDOWEVENT_RESIZED)
            {
                Linux_ResizeWindow(Event.window.data1, Event.window.data2);
            }
        }
        else if(Event.type == SDL_KEYDOWN ||
                Event.type == SDL_KEYUP)
        {
            u32 KeyCode = Event.key.keysym.sym;
            b32 WasPressed = false;
            if(Event.key.state == SDL_RELEASED ||
               Event.key.repeat != 0)
            {
                WasPressed = true;
            }
            b32 IsPressed = (Event.key.state == SDL_PRESSED);
            if(WasPressed != IsPressed)
            {
                if(KeyCode == SDLK_ESCAPE)
                {
                    GlobalRunning = false;
                }
                else if(KeyCode == SDLK_LEFT)
                {
					Linux_ProcessKeyMessage(&Input->Keys[Input_Left], IsPressed);
                }
				
                else if(KeyCode == SDLK_RIGHT)
                {
					Linux_ProcessKeyMessage(&Input->Keys[Input_Right], IsPressed);
                }
                if(KeyCode >= SDLK_a && KeyCode <= SDLK_z)
                {
                    u32 KeyIndex = KeyCode - ('a' - 'A');
					Linux_ProcessKeyMessage(&Input->Keys[KeyIndex], IsPressed);
                }
                if(IsPressed)
                {
                    u8 *Keys = (u8*)SDL_GetKeyboardState(0);
					int AltKeyWasPressed = (Keys[SDL_SCANCODE_LALT] || Keys[SDL_SCANCODE_RALT]);
					if((KeyCode == SDLK_F4) && AltKeyWasPressed)
                    {
                        GlobalRunning = false;
                    }
                    if((KeyCode == SDLK_RETURN) && AltKeyWasPressed)
                    {
                        Linux_ToggleFullscreen(Window);
                    }
                }
            }
        }
    }
}

int main(int argc, char **argv)
{
    PlatformAllocate = Linux_Allocate;
    PlatformDeallocate = Linux_Deallocate;
    PlatformReallocate = Linux_Reallocate;
    UpdateAndRender = UpdateAndRenderFunc;
    WinDim.Width = 800;
    WinDim.Height = 600;
    
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window *Window = SDL_CreateWindow("sp_sort", SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          WinDim.Width,WinDim.Height, SDL_WINDOW_OPENGL);
    if(Window)
    {
        SDL_GLContext GLContext = Linux_CreateRenderer(Window);
        if(GLContext)
        {
			input InputStruct = {0};
            input *Input = &InputStruct;
            GlobalRunning = true;
            while(GlobalRunning)
            {
                Linux_ProcessPendingMessages(Window, Input);
                
                BeginRender(WinDim.Width, WinDim.Height);UpdateAndRender(WinDim.Width, WinDim.Height, Input);
                
                SDL_GL_SwapWindow(Window);
            }
        }
    }
    SDL_Quit();
    return(0);
}