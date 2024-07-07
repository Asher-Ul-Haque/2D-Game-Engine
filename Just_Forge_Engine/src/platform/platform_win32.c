#if FORGE_PLATFORM_WINDOWS
#include "platform/platform.h"

#include "core/logger.h"
#include "core/input.h"
#include "core/event.h"

#include "dataStructures/list.h"

#include <windows.h>
#include <windowsx.h>  // param input extraction
#include <stdlib.h>

// For surface creation
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_win32.h>
#include "renderer/vulkan/vulkan_types.h"

// - - - Platform state 
typedef struct platformState
{
    HINSTANCE hInstance; //HISTANCE = HANDLE OF INSTANCE
    HWND hwnd; //HWND = HANDLE OF WINDOW
    VkSurfaceKHR surface;
} platformState;

//Clock
double clockFrequency;
LARGE_INTEGER startTime;

static platformState* statePtr;


// - - - | Platform Functions | - - -


// - - - State Functions - - -

LRESULT CALLBACK windowsProcessMessage(HWND HANDLE_WINDOW, unsigned int MESSAGE, WPARAM WINDOW_PARAMETER, LPARAM LONG_PARAMETER);

// - - - Clock Setup
void clockSetup()
{
    LARGE_INTEGER frequency;
    QueryPerformanceFrequency(&frequency);
    clockFrequency = 1.0 / (double)frequency.QuadPart;
    QueryPerformanceCounter(&startTime);
}

// - - - Initialize the platform
bool platformSystemInitialize(unsigned long long* MEMORY_REQUIREMENT, void* STATE, const char* APPLICATION, int X, int Y, int WIDTH, int HEIGHT)
{
    *MEMORY_REQUIREMENT = sizeof(platformState);
    if (STATE == 0)
    {
        return true;
    }
    statePtr = STATE;
    statePtr->hInstance = GetModuleHandleA(0); //Get the handle to the application running this code
    
    //Setup and register the window class
    HICON icon = LoadIcon(statePtr->hInstance, IDI_APPLICATION);
    WNDCLASSA windowClass;

    //Zero out the memory of the window class
    memset(&windowClass, 0, sizeof(windowClass));
    windowClass.style = CS_DBLCLKS; //Double click messages
    windowClass.lpfnWndProc = windowsProcessMessage; //The message processing function
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = statePtr->hInstance;
    windowClass.hIcon = icon;
    windowClass.hCursor = LoadCursor(NULL, IDC_ARROW); //Default cursor,
    windowClass.hbrBackground = NULL; //No background
    windowClass.lpszClassName = "JustForgeWindowClass";

    if (!RegisterClassA(&windowClass))
    {
        MessageBoxA(0, "Failed to register window class", "Error", MB_ICONERROR);
        return false;
    }

    //Create the window
    unsigned int clientWidth = WIDTH;
    unsigned int clientHeight = HEIGHT;
    unsigned int clientX = X;
    unsigned int clientY = Y;

    unsigned int windowWidth = clientWidth;
    unsigned int windowHeight = clientHeight;
    unsigned int windowX = clientX;
    unsigned int windowY = clientY;

    unsigned int windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME;
    unsigned int windowStyleEx = WS_EX_APPWINDOW;

    //Get border and title bar size
    RECT borderRect = { 0, 0, 0, 0 };
    AdjustWindowRectEx(&borderRect, windowStyle, 0, windowStyleEx);
    windowX += borderRect.left;
    windowY += borderRect.top;

    //Increase the window size to get the client size
    windowWidth += borderRect.right - borderRect.left;
    windowHeight += borderRect.bottom - borderRect.top;

    HWND handle = CreateWindowExA(
        windowStyleEx,
        windowClass.lpszClassName, //ForgeWindowClass
        APPLICATION,
        windowStyle,
        windowX,
        windowY,
        windowWidth,
        windowHeight,
        0,
        0,
        state->hInstance,
        0);

    if (handle == 0)
    {
        MessageBoxA(0, "Failed to create window", "Error", MB_ICONERROR);
        FORGE_LOG_FATAL("Failed to create window!");
        return false;
    }
    else 
    {
        state->hwnd = handle;
    }

    //Show the window
    bool shouldActivate = 1; // TODO: if the windows should not accept input, set this to 0
    int showWindowCommandFlags = shouldActivate ? SW_SHOW : SW_SHOWNOACTIVATE;
    //If the window is initially minimized, use SW_MINIMIZE : SW_SHOWMINNOACTIVE
    //If the window is initially maximized, use SW_MAXIMIZE : SW_SHOWMAXIMIZED
    ShowWindow(state->hwnd, showWindowCommandFlags);

    clockSetup();

    FORGE_LOG_INFO("Platform system initialized");
    return true;
}

// - - - Shutdown the platform
void platformSystemShutdown(void *STATE)
{
    if (statePtr && statePtr->hwnd)
    {
        DestroyWindow(statePtr->hwnd);
        statePtr->hwnd = 0;
        FORGE_LOG_INFO("Platform system shutdown");
    }
}

// - - - Give messages
bool platformGiveMessages()
{
    if (statePtr)
    {
        MSG message;
        while (PeekMessageA(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessageA(&message);
        }
    }

    return true;
}


// - - - Memory Function - - -

void* platformAllocateMemory(unsigned long long SIZE, bool ALIGNED)
{
    return malloc(SIZE);
}

void platformFreeMemory(void* MEMORY, bool ALIGNED)
{
    free(MEMORY);
}

void* platformZeroMemory(void* MEMORY, unsigned long long SIZE)
{
   return memset(MEMORY, 0, SIZE);
}

void* platformCopyMemory(void* DESTINATION, const void* SOURCE, unsigned long long SIZE)
{
    return memcpy(DESTINATION, SOURCE, SIZE);
}

void* platformSetMemory(void* DESTINATION, int VALUE, unsigned long long SIZE)
{
    return memset(DESTINATION, VALUE, SIZE);
}


// - - - Writing Functions - - -


void platformWriteConsole(const char* MESSAGE, unsigned char COLOR)
{
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    static unsigned char levels[6] = { 64, 4, 6, 2, 1, 8 };
    //@Travis Vroman found these colors, I have no idea how
    SetConsoleTextAttribute(consoleHandle, levels[COLOR]);
    OutputDebugStringA(MESSAGE);
    unsigned long long length = strlen(MESSAGE);
    LPDWORD numberWritten = 0;
    WriteConsoleA(consoleHandle, MESSAGE, (DWORD) length, numberWritten, 0);
}

void platformWriteConsoleError(const char* MESSAGE, unsigned char COLOR)
{
    HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
    static unsigned char levels[6] = { 64, 4, 6, 2, 1, 8 };
    //@Travis Vroman found these colors, I have no idea how
    SetConsoleTextAttribute(consoleHandle, levels[COLOR]);
    OutputDebugStringA(MESSAGE);
    unsigned long long length = strlen(MESSAGE);
    LPDWORD numberWritten = 0;
    WriteConsoleA(consoleHandle, MESSAGE, (DWORD) length, numberWritten, 0);
}


// - - - Time and Sleep Functions - - -

double platformGetTime()
{
    if (!clockFrequency)
    {
        clockSetup();
    }

    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);

    return (double)now.QuadPart * clockFrequency;
}

void platformSleep(unsigned long long MILLISECONDS)
{
    Sleep(MILLISECONDS);
}


// - - - | Window Message Procedure | - - -


LRESULT CALLBACK windowsProcessMessage(HWND HANDLE_WINDOW, unsigned int MESSAGE, WPARAM WINDOW_PARAMETER, LPARAM LONG_PARAMETER)
{
    switch (MESSAGE)
    {
        case WM_ERASEBKGND:
            return 1;
            //Yo Windows, Erasing is upon us.

        case WM_CLOSE:
            eventContext data = {};
            eventTrigger(EVENT_CODE_APPLICATION_QUIT, 0, data);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;

        case WM_SIZE:
            RECT r;
            GetClientRect(HANDLE_WINDOW, &r);
            unsigned int width = r.right - r.left;
            unsigned int height = r.bottom - r.top;
            
            // Fire the event. The application layer should pick this up, but not handle it
            // as it shouldn be visible to other parts of the application.
            eventContext context;
            context.data.u16[0] = (unsigned short)width;
            context.data.u16[1] = (unsigned short)height;
            eventTrigger(EVENT_CODE_WINDOW_RESIZE, 0, context);
            break;

        case WM_KEYDOWN:

        case WM_SYSKEYDOWN:

        case WM_KEYUP:

        case WM_SYSKEYUP:
            bool pressed = (MESSAGE == WM_KEYDOWN || MESSAGE == WM_SYSKEYDOWN);
            keys key = (unsigned short) WINDOW_PARAMETER;
            inputProcessKey(key, pressed);
            break;

        case WM_MOUSEMOVE:
            int xPosition = GET_X_LPARAM(LONG_PARAMETER);
            int yPosition = GET_Y_LPARAM(LONG_PARAMETER);
            inputProcessMouseMovement(xPosition, yPosition);
            break;

        case WM_MOUSEWHEEL:
            int zDelta = GET_WHEEL_DELTA_WPARAM(WINDOW_PARAMETER);
            if (zDelta != 0)
            {
                zDelta = (zDelta < 0) ? -1 : 1;
                // Scrolling up or down is 1 or -1
                inputProcessMouseWheel(zDelta);
            }
            break;

        case WM_LBUTTONDOWN:

        case WM_MBUTTONDOWN:

        case WM_RBUTTONDOWN:

        case WM_LBUTTONUP:

        case WM_MBUTTONUP:

        case WM_RBUTTONUP:
            pressed = (MESSAGE == WM_LBUTTONDOWN || MESSAGE == WM_MBUTTONDOWN || MESSAGE == WM_RBUTTONDOWN);
            buttons mouseButton = BUTTON_MAX_BUTTONS;
            switch(MESSAGE)
            {
                case WM_LBUTTONDOWN:
                case WM_LBUTTONUP:
                    mouseButton = BUTTON_LEFT;
                    break;
                case WM_MBUTTONDOWN:
                case WM_MBUTTONUP:
                    mouseButton = BUTTON_MIDDLE;
                    break;
                case WM_RBUTTONDOWN:
                case WM_RBUTTONUP:
                    mouseButton = BUTTON_RIGHT;
                    break;
            }
            if (mouseButton != BUTTON_MAX_BUTTONS)
            {
                inputProcessButton(mouseButton, pressed);
            }
            break;
    }

    return DefWindowProcA(HANDLE_WINDOW, MESSAGE, WINDOW_PARAMETER, LONG_PARAMETER);
}

// - - - Vulkan Functions - - -

void platformGetRequiredExtensions(const char*** EXTENSIONS)
{
    listAppend(*EXTENSIONS, &"VK_KHR_win32_surface");
}

// - - - Vulkan Surface Creation
bool platformCreateSurface(vulkanContext* CONTEXT)
{
    if (!statePtr)
    {
        FORGE_LOG_WARNING("Platform system not initialized");
        return false;
    }
    VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR};
    surfaceCreateInfo.hinstance = statePtr->hInstance;
    surfaceCreateInfo.hwnd = statePtr->hwnd;

    VkResult result = vkCreateWin32SurfaceKHR(CONTEXT->instance, &surfaceCreateInfo, 0, &statePtr->surface);
    if (result != VK_SUCCESS)
    {
        FORGE_LOG_FATAL("Failed to create surface");
        return false;
    }

    CONTEXT->surface = statePtr->surface;
    return true;
}

#endif //FORGE_PLATFORM_WINDOWS
