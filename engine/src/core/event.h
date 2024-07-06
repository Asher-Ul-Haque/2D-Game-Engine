#pragma once
#include "defines.h"


// - - - | Event | - - -

// - - -  Event Codes  - - -

//System code from 0 to 255
typedef enum systemEventCode
{
    EVENT_CODE_APPLICATION_QUIT = 0x01, 
    /*
      Shut down the application on the next frame
      Context : None required
    */
    EVENT_CODE_KEY_PRESS = 0x02,
    /*
      A key has been pressed
      Context : unsigned short key code = CONTEXT.data.u16[0] : Key code
    */
    EVENT_CODE_KEY_RELEASE = 0x03,
    /*
      A key has been released
      Context : unsigned short key code = CONTEXT.data.u16[0] : Key code
    */
    EVENT_CODE_BUTTON_PRESS = 0x04,
    /*
      A button has been pressed
      Context : unsigned short button code = CONTEXT.data.u16[0] : Button code
    */
    EVENT_CODE_BUTTON_RELEASE = 0x05,
    /*
      A button has been released
      Context : unsigned short button code = CONTEXT.data.u16[0] : Button code
    */
    EVENT_CODE_MOUSE_MOVE = 0x06,
    /*
      The mouse moved
      Context: unsigned short x = CONTEXT.data.u16[0] : X position
               unsigned short y = CONTEXT.data.u16[1] : Y position
    */
    EVENT_CODE_MOUSE_WHEEL = 0x07,
    /*
      The mouse wheel moved. -1 is down, 1 is up
      Context: unsigned short delta = CONTEXT.data.u18[0] : Wheel delta
    */
    EVENT_CODE_RESIZE = 0x08,
    /*
      The window has been resized
      Context: unsigned short width = CONTEXT.data.u16[0] : New width
               unsigned short height = CONTEXT.data.u16[1] : New height
    */
    MAX_SYSTEM_EVENT_CODE = 0xFF
} systemEventCode;

// - - - Context
typedef struct eventContext
{
    union // 16 bytes of data
    {
        // 2x 8 byte values
        long long i64[2];
        unsigned long long u64[2];
        double f64[2];
        
        // 4x 4 byte values
        int i32[4];
        unsigned int u32[4];
        float f32[4];

        // 8x 2 byte values
        short i16[8];
        unsigned short u16[8];

        // 16x 1 byte values
        char i8[16];
        unsigned char u8[16];

        // 16x 1 byte values
        char c[16];
    } data;
} eventContext;


// - - - Callback
// Return true to stop the event from being passed to other listeners and if you handled the event
typedef bool (*eventCallback)(unsigned short CODE, void* SENDER, void* LISTENER, eventContext CONTEXT);


// - - - | Event System functions | - - -


// - - - Game engine functions - - -

void eventSystemInitialize(unsigned long long* MEMORY_REQUIREMENT, void* STATE);

void eventSystemShutdown(void* STATE);


// - - - Game developer functions - - -

// Register to listen for events. Events with duplicate listener/callback pairs will be ignored and returned as false
FORGE_API bool eventRegister(unsigned short CODE, void* LISTENER, eventCallback CALLBACK);

// Unregister to listen for events. Events with duplicate listener/callback pairs will be ignored and returned as false
FORGE_API bool eventUnregister(unsigned short CODE, void* LISTENER, eventCallback CALLBACK);

FORGE_API bool eventTrigger(unsigned short CODE, void* SENDER, eventContext CONTEXT);
