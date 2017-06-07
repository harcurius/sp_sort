
#include "platform.c"

#define UNICODE
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "opengl.c"

typedef const char* (WINAPI WGLGETEXTENSIONSSTRINGARB)(HDC);
typedef int  (WINAPI WGLSWAPINTERVALEXT_T) (int interval);
static WGLGETEXTENSIONSSTRINGARB *wglGetExtensionsStringARB;
static WGLSWAPINTERVALEXT_T *wglSwapIntervalEXT;

struct
{
    int Width;
    int Height;
}WinDim;

static b32 GlobalRunning;
static WINDOWPLACEMENT GlobalWindowPlacement;

static
PLATFORM_ALLOCATE(Win32_Allocate)
{
    void *Memory = VirtualAlloc(0, Size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    return(Memory);
}

static
PLATFORM_DEALLOCATE(Win32_Deallocate)
{
    if(Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

static
PLATFORM_REALLOCATE(Win32_Reallocate)
{
    void *NewMemory = Win32_Allocate(Size);
    MemoryCopy(NewMemory, OldMemory, Size);
    Win32_Deallocate(OldMemory);
    return(NewMemory);
}

static void*
Win32_GetGLFunction(char *Name)
{
    static HMODULE OpenGLModule = 0;
    if(!OpenGLModule)
    {
        OpenGLModule = LoadLibraryA("OpenGL32.dll");
    }
    void *Pointer = (void*)wglGetProcAddress(Name);
    if(Pointer == 0 || Pointer == (void*)0x2 || Pointer == (void*)0x3 || Pointer == (void*)-1)
    {
        if(OpenGLModule)
        {
            Pointer = (void*)GetProcAddress(OpenGLModule, Name);
        }
    }
    return(Pointer);
}

static HGLRC
CreateRenderer(HDC DeviceContext)
{
    HGLRC Result = 0;
    PIXELFORMATDESCRIPTOR PixelFormat = {
        .nSize = sizeof(PIXELFORMATDESCRIPTOR),
        .nVersion = 1,
        .dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        .iPixelType = PFD_TYPE_RGBA,
        .cColorBits = 32,
        .iLayerType = PFD_MAIN_PLANE
    };
    int PixelFormatIndex = ChoosePixelFormat(DeviceContext, &PixelFormat);
    if(PixelFormatIndex)
    {
        if(SetPixelFormat(DeviceContext, PixelFormatIndex, &PixelFormat))
        {
            Result = wglCreateContext(DeviceContext);
            wglMakeCurrent(DeviceContext, Result);
            wglGetExtensionsStringARB = Win32_GetGLFunction("wglGetExtensionsStringARB");
            if(wglGetExtensionsStringARB)
            {
                char *WglExtensions = (char*)wglGetExtensionsStringARB(DeviceContext);
                if(WglExtensions)
                {
                    char *Scan = WglExtensions;
                    char CurrentExtension[512];
                    u32 CurrentExtensionLength = 0;
                    do
                    {
                        if(*Scan == ' ')
                        {
                            CurrentExtension[CurrentExtensionLength] = 0;
                            CurrentExtensionLength = 0;
                            if(!strcmp(CurrentExtension, "WGL_EXT_swap_control"))
                            {
                                wglSwapIntervalEXT = Win32_GetGLFunction("wglSwapIntervalEXT");
                                break;
                            }
                        }
                        else
                        {
                            if(CurrentExtensionLength < ArrayCount(CurrentExtension))
                            {
                                CurrentExtension[CurrentExtensionLength++] = *Scan;
                            }
                        }
                    } while(*Scan++);
                }
                if(wglSwapIntervalEXT)
                {
                    wglSwapIntervalEXT(1);
                }
            }
        }
    }
    return(Result);
}

static void
Win32_ResizeWindow(int Width, int Height)
{
    if(Height <= 0)
    {
        Height = 1;
    }
    WinDim.Width = Width;
    WinDim.Height = Height;
}

static void
Win32_ToggleFullscreen(HWND Window)
{
    // http://blogs.msdn.com/b/oldnewthing/archive/2010/04/12/9994016.aspx
    
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if(Style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo = { sizeof(MonitorInfo)};
        if(GetWindowPlacement(Window, &GlobalWindowPlacement) &&
           GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    }
    else
    {
        SetWindowLong(Window, GWL_STYLE, Style | (WS_OVERLAPPEDWINDOW/* ^ (WS_THICKFRAME | WS_MAXIMIZEBOX)*/));
        SetWindowPlacement(Window, &GlobalWindowPlacement);
        SetWindowPos(Window, 0, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

LRESULT CALLBACK
Win32_MainWindowCallback(HWND windowHandle, UINT message,
                         WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_CLOSE:
        case WM_DESTROY:
        {
            GlobalRunning = false;
        }break;
        case WM_SIZE:
        {
            Win32_ResizeWindow(LOWORD(lParam), HIWORD(lParam));
        }break;
    }
    return (DefWindowProc(windowHandle, message, wParam, lParam));
}

static void
Win32_ProcessKeyMessage(input_key *NewState, b32 IsPressed)
{
    if(NewState->Pressed != IsPressed)
    {
        NewState->Pressed = IsPressed;
        ++NewState->Transition;
    }
}

static void
Win32_ProcessPendingMessages(HWND WindowHandle, input *Input)
{
    for(u32 KeyIndex = 0;
        KeyIndex < ArrayCount(Input->Keys);
        ++KeyIndex)
    {
        Input->Keys[KeyIndex].Transition = 0;
    }
    MSG Message;
    while(PeekMessage(&Message, 0,0,0, PM_REMOVE))
        //while(GetMessage(&Message, WindowHandle,0,0))
    {
        switch(Message.message)
        {
            case WM_QUIT:
            {
                GlobalRunning = false;
            }break;
            case WM_SYSKEYDOWN:
            case WM_KEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYUP:
            {
                u32 VKCode = (u32)Message.wParam;
                b32 WasPressed = ((Message.lParam & (1 << 30)) != 0);
                b32 IsPressed = ((Message.lParam & (1 << 31)) == 0);
                if(WasPressed != IsPressed)
                {
                    if(VKCode == VK_ESCAPE)
                    {
                        GlobalRunning = false;
                    }
                    else if(VKCode == VK_LEFT)
                    {
                        Win32_ProcessKeyMessage(&Input->Keys[Input_Left], IsPressed);
                    }
                    else if(VKCode == VK_RIGHT)
                    {
                        Win32_ProcessKeyMessage(&Input->Keys[Input_Right], IsPressed);
                    }
                    if(VKCode >= ' ' && VKCode <= 'Z')
                    {
                        u32 test = VKCode;
                        Win32_ProcessKeyMessage(&Input->Keys[VKCode], IsPressed);
                    }
                    if(IsPressed)
                    {
                        int AltKeyWasPressed = (Message.lParam & (1 << 29));
                        if((VKCode == VK_F4) && AltKeyWasPressed)
                        {
                            GlobalRunning = false;
                        }
                        if((VKCode == VK_RETURN) && AltKeyWasPressed)
                        {
                            Win32_ToggleFullscreen(Message.hwnd);
                        }
                    }
                }
            }break;
            default:
            {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }break;
        }
    }    
}

int CALLBACK
WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
    PlatformAllocate = Win32_Allocate;
    PlatformDeallocate = Win32_Deallocate;
    PlatformReallocate = Win32_Reallocate;
    UpdateAndRender = UpdateAndRenderFunc;
    WinDim.Width = 800;
    WinDim.Height = 600;
    
    WNDCLASSEXW WindowClass = {0};
    WindowClass.cbSize = sizeof(WindowClass);
    WindowClass.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    WindowClass.lpfnWndProc = Win32_MainWindowCallback;
    WindowClass.hInstance = hInstance;
    WindowClass.lpszClassName = L"sp_sort";
    WindowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
    WindowClass.hCursor = LoadCursor(0, IDC_ARROW);
    WindowClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
    if(RegisterClassExW(&WindowClass))
    {
        DWORD ExtendedStyle = 0;
        DWORD WindowStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
        
        RECT WindowRect = {.left = 0, .right = (LONG)WinDim.Width, .top = 0, .bottom = (LONG)WinDim.Height};
        AdjustWindowRectEx(&WindowRect, WindowStyle, 0, ExtendedStyle);
        
        DWORD WindowWidth = WindowRect.right - WindowRect.left;
        DWORD WindowHeight = WindowRect.bottom - WindowRect.top;
        
        HWND WindowHandle = CreateWindowExW(ExtendedStyle,
                                            WindowClass.lpszClassName,
                                            WindowClass.lpszClassName,
                                            WindowStyle,
                                            CW_USEDEFAULT, 
                                            CW_USEDEFAULT, 
                                            WindowWidth,
                                            WindowHeight,
                                            0, 0, hInstance, 0); 
        if(WindowHandle)
        {
            HDC DeviceContext = GetDC(WindowHandle);
            HGLRC OpenGLContex = CreateRenderer(DeviceContext);
            if(OpenGLContex)
            {
                input InputStruct = {0};
                input *Input = &InputStruct;
                GlobalRunning = true;
                while(GlobalRunning)
                {
                    Win32_ProcessPendingMessages(WindowHandle, Input);
                    BeginRender(WinDim.Width, WinDim.Height);
                    UpdateAndRender(WinDim.Width, WinDim.Height, Input);
                    SwapBuffers(DeviceContext);
                }
            }
        }
    }
    return(0);
}
